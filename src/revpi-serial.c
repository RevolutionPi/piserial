#include <stdint.h>
#include <stdio.h>

#include "atecc508a.h"

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

int main(int argc, char *argv[])
{
	const unsigned int dev_addr = 0x60;
	int ret;
	uint8_t serial[8];
	ret = atecc508a_serial("/dev/i2c-1", dev_addr, serial);
	printf("Serial: 0x%02X%02X%02X%02X%02X%02X%02X%02X\n",
	       serial[0], serial[1], serial[2], serial[3], serial[4], serial[5],
	       serial[6], serial[7]);
	return ret;
}
