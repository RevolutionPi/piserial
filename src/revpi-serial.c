// SPDX-License-Identifier: GPL-2.0

/*
 * Copyright: 2021 KUNBUS GmbH
 */

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "atecc508a.h"
#include "debug.h"
#include "tpm2.h"

#define PISERIAL_VERSION "2.1.0"

const char lock_path[] = "/var/run/piserial.lock";

/* The dummy driver is only for testing purpose. */
static int dummy_serial(uint8_t * const serial)
{
	const uint8_t dummy[] = {
		0xAB, 0xBA, 0xAF, 0xFE, 0xDE, 0xAD, 0xBE, 0xEF
	};
	memcpy(serial, dummy, sizeof(dummy));
	return 0;
}

static void serial2pw(char * const pw, const uint8_t * const serial)
{
	uint8_t pos, pos2;
	uint16_t data;
	uint8_t salt[] = { 0x81, 0xae, 0xf2, 0x9d, 0x47, 0xd3 };
	const char code[] = "hancvo714xqwelm289rtz0356sdfgybp";

	for (int i = 0; i < 6; i++)
	{
		salt[i] ^= serial[i + 2];
	}

	/* reduce the length to 4 bytes */
	salt[2] ^= salt[5];
	salt[0] ^= salt[4];

	for (int i = 0; i < 6; i++)
	{
		pos = (i * 5) / 8;
		data = salt[pos] + (salt[pos + 1] << 8);
		pos2 = (i * 5) % 8;
		data >>= pos2;
		data &= 0x001f;
		pw[i] = code[data];
	}
	pw[6] = '\0';
}

static int piserial_lock(void)
{
	int ret;
	int lock_fd;

	lock_fd = open(lock_path, O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (lock_fd < 0) {
		err_print("Can't open piserial lock file `%s': %s\n", lock_path,
		          strerror(errno));
		return -1;
	}

	for (int i = 0; i < 10; i++) {
		ret = flock(lock_fd, LOCK_EX | LOCK_NB);
		if (ret < 0 && errno != EWOULDBLOCK)
			break;
		usleep(10000);
	}
	if (ret < 0) {
		err_print("Can't get lock `%s': %s\n", lock_path,
		          strerror(errno));
		close(lock_fd);
		return -1;
	}

	return lock_fd;
}

static int piserial_unlock(const int lock_fd)
{
	int ret;

	ret = flock(lock_fd, LOCK_UN);
	if (ret < 0) {
		err_print("Can't remove piserial lock `%s': %s\n", lock_path,
		          strerror(errno));
		return ret;
	}

	close(lock_fd);
	return 0;
}

static void usage(FILE *out, const char * const pname)
{
	fprintf(out, "Usage: %s [OPTION]...\n", pname);
	fputs("  -c DEVICE  use the ATECC508A crypto device (default)\n", out);
	fputs("  -t DEVICE  use the tpm2 device\n", out);
	fputs("  -p         print the generated password\n", out);
	fputs("  -s         print the serial number\n", out);
	fputs("  -v         print version and exit\n", out);
	fputc('\n', out);
	fputs("If no print option is specified both, the serial number\n"
	      "and the password are printed.\n", out);
	fprintf(out, "\nPiSerial %s\n", PISERIAL_VERSION);
}

int main(int argc, char *argv[])
{
	const char * const pname = argv[0];
	const unsigned int dev_addr = 0x60;
	const char * dev_path = "/dev/i2c-1";
	int ret;
	int opt;
	bool opt_dummy = false;
	bool opt_atecc508a = false;
	bool opt_pw = false;
	bool opt_serial = false;
	bool opt_tpm2 = false;

	while ((opt = getopt(argc, argv, "c:dpst:v")) != -1) {
		switch (opt)
		{
		case 'c':
			opt_atecc508a = true;
			dev_path = optarg;
			break;
		case 'd':
			opt_dummy = true;
			break;
		case 'p':
			opt_pw = true;
			break;
		case 's':
			opt_serial = true;
			break;
		case 't':
			opt_tpm2 = true;
			dev_path = optarg;
			break;
		case 'v':
			printf("PiSerial %s\n", PISERIAL_VERSION);
			exit(EXIT_SUCCESS);
		default:
			usage(stderr, pname);
			exit(EXIT_FAILURE);
		}
	}

	/* If no device option was supplied fall back to use the cryptochip */
	if (!opt_dummy && !opt_atecc508a && !opt_tpm2) {
		opt_atecc508a = true;
	} else if ((opt_dummy && opt_atecc508a) ||
	           (opt_dummy && opt_tpm2) ||
	           (opt_atecc508a && opt_tpm2)) {
		err_print("Please specifiy only one device option.\n");
		usage(stderr, pname);
		exit(EXIT_FAILURE);
	}

	/* If no output option was supplied print both */
	if (!opt_pw && !opt_serial) {
		opt_pw = true;
		opt_serial = true;
	}

	int lock_fd;
	lock_fd = piserial_lock();
	if (lock_fd < 0)
		exit(EXIT_FAILURE);
	uint8_t serial[8];
	if (opt_dummy)
		ret = dummy_serial(serial);
	else if (opt_atecc508a)
		ret = atecc508a_serial(dev_path, dev_addr, serial);
	else if (opt_tpm2)
		ret = tpm2_serial(dev_path, serial);
	piserial_unlock(lock_fd);
	if (ret < 0) {
		err_print("FATAL: Can't get serial number from device\n");
		return ret;
	}

	char pw[7];
	serial2pw(pw, serial);

	if (opt_serial) {
		printf("%02X%02X%02X%02X%02X%02X%02X%02X",
		       serial[0], serial[1], serial[2], serial[3],
		       serial[4], serial[5], serial[6], serial[7]);
		if (opt_pw)
			putchar(' ');
	}
	if (opt_pw) {
		fputs(pw, stdout);
	}
	putchar('\n');
	fflush(stdout);

	return 0;
}
