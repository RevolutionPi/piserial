#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>

typedef unsigned char       TBOOL;       ///< Boolean value (bTRUE/bFALSE)
#define bTRUE      ((TBOOL)1)
#define bFALSE    ((TBOOL)0)

#include <sched.h>
#include <fcntl.h>
#include <sys/mman.h>	// Needed for mlockall()
#include <unistd.h>		// needed for sysconf(int name);
#include <malloc.h>
#include <sys/time.h>	// needed for getrusage
#include <sys/resource.h>	// needed for getrusage
#include <pthread.h>
#include <limits.h>
#include <linux/i2c-dev.h>

#include <sys/ioctl.h>

int get_sn_tpm(const char *dev_node);
int get_sn_i2c(const char *dev_node);
char *readSerNum(void);
char *getHostname(void);
char *getPassword(void);

extern uint8_t i8uSernum[8];
void* md5(unsigned char*, unsigned int, unsigned char*, void *);

char *getHostname(void)
{
	uint8_t cnt;
	static char sHostname[11];
	unsigned char sSalt[64];
	char *sCode32 = "ABCDEFGHJKLMNPQRSTUVWXYZ12345678";
	unsigned char digest[16];
	void *handle = NULL;

	sSalt[24] = 0x46;
	sSalt[6] = 0xd5;
	sSalt[26] = 0x09;
	sSalt[15] = 0x11;
	sSalt[16] = 0x83;
	sSalt[17] = 0xaa;
	sSalt[18] = 0xf2;
	sSalt[5] = 0xd1;

	sSalt[25] = 0x40;
	sSalt[27] = i8uSernum[5];
	sSalt[14] = 0x36;
	sSalt[19] = i8uSernum[0];
	sSalt[3] = i8uSernum[7];
	sSalt[0] = 0x27;
	sSalt[1] = 0xab;
	sSalt[11] = i8uSernum[6];
	sSalt[13] = i8uSernum[2];
	sSalt[28] = 0xc2;
	sSalt[9] = 0x6e;
	sSalt[10] = 0xcc;
	sSalt[2] = i8uSernum[3];
	sSalt[7] = 0x03;
	sSalt[8] = 0xf3;
	sSalt[22] = i8uSernum[4];
	sSalt[23] = 0xc8;
	sSalt[4] = i8uSernum[1];
	sSalt[20] = 0xdd;
	sSalt[12] = 0x03;
	sSalt[21] = 0x9a;
	sSalt[29] = 0x56;

	handle = md5(sSalt, 30, digest, handle);

	for (cnt = 0; cnt < 10; cnt++)
	{
		sHostname[cnt] = sCode32[digest[cnt] % 32];
	}
	sHostname[10] = 0;

	return sHostname;
}


char *getPassword(void)
{
	uint8_t cnt, pos, pos2;
	uint16_t data;
	static char sPassword[7];
	char *sCode32 = "hancvo714xqwelm289rtz0356sdfgybp";
	uint8_t i8uSalt[] = { 0x81, 0xae, 0xf2, 0x9d, 0x47, 0xd3 };

	for (cnt = 0; cnt < 6; cnt++)
	{
		i8uSalt[cnt] ^= i8uSernum[cnt + 2];
	}

	// reduce the length to 4 Bytes
	i8uSalt[2] ^= i8uSalt[5];
	i8uSalt[0] ^= i8uSalt[4];

	for (cnt = 0; cnt < 6; cnt++)
	{
		pos = (cnt * 5) / 8;
		data = i8uSalt[pos] + (i8uSalt[pos + 1] << 8);
		pos2 = (cnt * 5) % 8;
		data >>= pos2;
		data &= 0x001f;
		sPassword[cnt] = sCode32[data];
	}

	return sPassword;
}

int main(int argc, char *argv[])
{
	int r;
	TBOOL bShowSerNum = bTRUE;
	TBOOL bShowHostname = bTRUE;
	TBOOL bShowDefaultPassword = bTRUE;
	/* by default get the serial number from /dev/i2c-1 */
	enum cypt_dev_type {CRYPT_DEV_I2C, CRYPT_DEV_TPM};
	enum cypt_dev_type crypt_dev = CRYPT_DEV_I2C;
	const char *dev_path = "/dev/i2c-1";
	int opt;

	while ((opt = getopt(argc, argv, "c:t:shp")) != -1) {
		switch (opt) {
		case 'c':
			crypt_dev = CRYPT_DEV_I2C;
			dev_path = optarg;
			break;
		case 't':
			crypt_dev = CRYPT_DEV_TPM;
			dev_path = optarg;
			break;
		case 's':
			bShowSerNum = bTRUE;
			bShowHostname = bFALSE;
			bShowDefaultPassword = bFALSE;
			break;
		case 'h':
			bShowSerNum = bFALSE;
			bShowHostname = bTRUE;
			bShowDefaultPassword = bFALSE;
			break;
		case 'p':
			bShowSerNum = bFALSE;
			bShowHostname = bFALSE;
			bShowDefaultPassword = bTRUE;
			break;
		default:
			printf("usage: %s [-c dev|-t dev ][-s ][-h ][-p ]\n\n",
								argv[0]);
			printf("-c specify the i2c device of the crypto chip\n");
			printf("-t specify the device of the tpm\n");
			printf("-s show serial number\n");
			printf("-h show hostname\n");
			printf("-p show inital password\n");
			printf("no argument for show: show all\n");
			exit(EXIT_FAILURE);
		}
	}

	if (crypt_dev == CRYPT_DEV_TPM) {
		r = get_sn_tpm(dev_path);
	} else {
		for (int cnt = 0; cnt < 10; cnt ++) {
			r = get_sn_i2c(dev_path);
			if (r >= 0) break;
		}
	}
	if (r < 0) {
		printf("get serial number failed:%d\n", r);
		exit(1);
	}

	if (bShowSerNum)
	{
		printf("%s ", readSerNum());
	}
	if (bShowHostname)
	{
		printf("%s ", getHostname());
	}
	if (bShowDefaultPassword)
	{
		printf("%s", getPassword());
	}
	printf("\n");
	return 0;
}
