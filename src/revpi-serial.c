#include <stdint.h>
#include <stdio.h>

#include "atecc508a.h"

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
