/*
 * Microchip Technology Inc. ATECC508A
 *
 * This file implements reading the serial number from ATECC508A.
 * The full data sheet can be found here:
 * https://content.arduino.cc/assets/mkr-microchip_atecc508a_cryptoauthentication_device_summary_datasheet-20005927a.pdf
 *
 * The serial number is stored in the EEPROM Configuration Zone (see 2.2).
 */

#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"

static const int MAX_RETRY = 5;

/* Word Address Values (see: 6.2.1 Word Address Values) */
#define WA_RESET 0x00
#define WA_SLEEP 0x01
#define WA_IDLE  0x02
#define WA_CMD   0x03

/* Command Opcodes (see: 9.1.3 Command Opcodes, ... */
#define CMD_READ 0x02

/* Status/Error Codes (see: 9.1.2 Status/Error Codes) */
#define STAT_SUCCESS 0x00
#define STAT_CHKMAC  0x01
#define STAT_PARSE   0x03
#define STAT_ECC     0x05
#define STAT_EXEC    0x0f
#define STAT_WAKE    0x11
#define STAT_WD_EXP  0xEE
#define STAT_CRC     0xFF

struct atecc508a
{
	int fd;
	const char *dev_path;
};

struct __attribute__((__packed__)) atecc508a_cmd
{
	uint8_t waddr;
	uint8_t len;
	uint8_t cmd;
	uint8_t zone;
	uint8_t addr[2];
	uint8_t crc[2];
};

struct __attribute__((__packed__)) atecc508a_recv_stat
{
	uint8_t len;
	uint8_t stat;
	uint8_t crc[2];
};

struct __attribute__((__packed__)) atecc508a_recv_data
{
	uint8_t len;
	uint8_t data[4];
	uint8_t crc[2];
};

static void calc_crc(const uint8_t length, const uint8_t * const data, uint8_t * const crc)
{
	const uint16_t polynom = 0x8005;
	uint8_t counter;
	uint16_t crc_register = 0;
	uint8_t shift_register;
	uint8_t data_bit, crc_bit;

	for (counter = 0; counter < length; counter++) {
		for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1) {
			data_bit = (data[counter] & shift_register) ? 1 : 0;
			crc_bit = crc_register >> 15;
			crc_register <<= 1;
			if (data_bit != crc_bit)
				crc_register ^= polynom;
		}
	}
	crc[0] = (uint8_t)(crc_register & 0x00FF);
	crc[1] = (uint8_t)(crc_register >> 8);
}

/*
 * This function can be used to send simple Word Address Values to the device.
 * This can be used for reset, sleep, idle. To send commands a more complex
 * function is needed.
 */
static void atecc508a_simple_wa(const struct atecc508a * const priv, const uint8_t wa)
{
	int ret = write(priv->fd, &wa, 1);
	if (ret < 0) {
		dbg_print("Can't write (%s()) to the I2C device: %s\n",
		          __func__, strerror(errno));
	}
}

/*
 * Reset the address counter. The next I2C read or write transaction will start
 * with the beginning of the I/O buffer.
 * Also wakeups up the device if it is in the sleep mode.
 */
static void atecc508a_reset(const struct atecc508a * const priv)
{
	atecc508a_simple_wa(priv, WA_RESET);
	/* The sleep time was evaluated by testing */
	usleep(1000);
}

/*
 * Send the ATECC508A into the low power sleep mode.
 */
static void atecc508a_sleep(const struct atecc508a * const priv)
{
	atecc508a_simple_wa(priv, WA_SLEEP);
}

/*
 * Check the crc of received data.
 */
static int atecc508a_recv_checkcrc(const uint8_t * const data)
{
	const uint8_t len = data[0] - 2;
	uint8_t crc[2];
	calc_crc(len, data, crc);
	dbg_print("recv.crc: 0x%02x%02x, crc: 0x%02x%02x\n",
	          data[len], data[len + 1], crc[0], crc[1]);
	return (data[len] == crc[0] && data[len + 1] == crc[1]) ? 0 : -1;
}

static int atecc508a_read(const struct atecc508a *const priv,
                          const uint8_t * const addr,
                          uint8_t * const out)
{
	int ret;
	int retry = 0;

	struct atecc508a_cmd cmd;
	cmd.waddr = WA_CMD;
	cmd.len = sizeof(struct atecc508a_cmd) - sizeof(cmd.waddr);
	cmd.cmd = CMD_READ;
	cmd.zone = 0;
	cmd.addr[0] = addr[0];
	cmd.addr[1] = addr[1];
	calc_crc(cmd.len - sizeof(cmd.crc), &cmd.len, cmd.crc);

	do {
		ret = write(priv->fd, &cmd, sizeof(struct atecc508a_cmd));
		if (ret < 0)
			warn_print("Can't write to `%s': %s\n", priv->dev_path,
			           strerror(errno));

		dbg_print("write: WA:0x%02x, LEN:0x%02x, CMD:0x%02x, "
		          "ZONE:0x%02x, ADDR:0x%02x%02x, CRC:0x%02x%02x = %d\n",
		          cmd.waddr, cmd.len, cmd.cmd, cmd.zone, cmd.addr[0],
		          cmd.addr[1], cmd.crc[0], cmd.crc[1], ret);

		retry++;
		if (retry >= MAX_RETRY)
			return -1;
	} while (ret != sizeof(struct atecc508a_cmd));

	/* The Max. Exec. Time for Read is 1ms. (see 9.1.3) */
	usleep(1000);

	retry = 0;
	struct atecc508a_recv_data rd;
	size_t num_read = 0;
	do {
		uint8_t *ptr = (uint8_t *)&rd + num_read;
		ret = read(priv->fd, ptr, sizeof(struct atecc508a_recv_data) - num_read);
		if (ret < 0) {
			warn_print("Can't read from `%s': %s\n", priv->dev_path,
			           strerror(errno));
			retry++;
			if (retry >= MAX_RETRY)
				return -1;
		} else if (ret != sizeof(struct atecc508a_recv_data) - num_read) {
			num_read += ret;
			continue;
		} else {
			if(rd.len > 7) {
				warn_print("Wrong data len\n");
				return -EBADMSG;
			}
			if (atecc508a_recv_checkcrc((uint8_t *)&rd) < 0) {
				warn_print("Wrong CRC recived\n");
				return -EBADMSG;
			}

			if (rd.len == 4) {
				const struct atecc508a_recv_stat *rs;
				rs = (struct atecc508a_recv_stat *)&rd;
				dbg_print("Got status: 0x%02x\n", rs->stat);
				switch (rs->stat)
				{
				case STAT_WAKE:
					ret = -EAGAIN;
					break;
				case STAT_WD_EXP:
					atecc508a_sleep(priv);
					atecc508a_reset(priv);
					ret = -EAGAIN;
					break;
				case STAT_CRC:
					atecc508a_reset(priv);
					ret = -EAGAIN;
					break;
				default:
					ret = -EINVAL;
					break;
				}
				return ret;
			} else if (rd.len != 7) {
				err_print("Unknown response from device.\n");
				return -EINVAL;
			}
		}
	} while (ret < 0);

	memcpy(out, rd.data, sizeof(rd.data));

	return 0;
}

int atecc508a_serial(const char * const dev_path, const unsigned int dev_addr,
                     uint8_t * const serial)
{
	struct atecc508a priv;
	priv.fd = open(dev_path, O_RDWR);
	priv.dev_path = dev_path;

	if (priv.fd < 0) {
		err_print("Can't open file `%s': %s\n", dev_path,
		          strerror(errno));
		return -errno;
	}

	int ret;
	ret = ioctl(priv.fd, I2C_SLAVE, dev_addr);
	if (ret < 0) {
		err_print("IOCTL 'I2C_SLAVE' (0x%04x) on file `%s' failed: %s\n",
		          I2C_SLAVE, dev_path, strerror(errno));
		ret = -errno;
		goto out;
	}

	uint8_t addr[2];
	addr[0] = 0;
	addr[1] = 0;
	atecc508a_reset(&priv);
	do {
		ret = atecc508a_read(&priv, addr, serial);
	} while (ret == -EAGAIN);
	if (ret < 0)
		goto sleep;
	addr[0] = 2;
	do {
		ret = atecc508a_read(&priv, addr, serial + 4);
	} while (ret == -EAGAIN);
sleep:
	atecc508a_sleep(&priv);
out:
	close(priv.fd);
	return ret;
}
