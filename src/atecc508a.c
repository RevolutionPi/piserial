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

const char * const dev_path = "/dev/i2c-1";

int atecc508a_serial()
{
	int fd;
	int ret;
	fd = open(dev_path, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "ERROR: Can't open file `%s': %s\n", dev_path,
		        strerror(errno));
		return -errno;
	}

	unsigned int addr = 0x60;
	ret = ioctl(fd, I2C_SLAVE, addr);
	if (ret < 0) {
		fprintf(stderr,
		        "ERROR: IOCTL 'I2C_SLAVE' (0x%04x) on file `%s' failed: %s\n",
		        I2C_SLAVE, dev_path, strerror(errno));
		ret = -errno;
		goto out;
	}

out:
	close(fd);
	return ret;
}