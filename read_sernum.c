/**
 *
 * Copyright (c) 2015 Atmel Corporation. All rights reserved.
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel integrated circuit.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * 
 * Extensions by KUNBUS under the following License
 *
 * Copyright (C) 2017 : KUNBUS GmbH, Heerweg 15C, 73370 Denkendorf, Germany
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>

#include <sys/ioctl.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

uint8_t i8uSernum[8];

static void atCRC(uint8_t length, const uint8_t *data, uint8_t *crc)
{
    uint8_t counter;
    uint16_t crc_register = 0;
    uint16_t polynom = 0x8005;
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

int getSerNumI2c(void)
{
    int fd;
    int r, i;
    uint8_t buf[10];
    uint8_t cnt;
    
    fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0)
    {
        return -1;
    }
    
    int addr = 0x60;
    if (ioctl(fd, I2C_SLAVE, addr) < 0) 
    {
        close(fd);
        return -2;
    }
 
    for (i = 0; i < 2; i++)
    {
        /* Using I2C read */
        buf[0] = 0x03;	// command
        buf[1] = 7;		// length
        buf[2] = 0x02;	// read
        buf[3] = 0x00;	// zone
        if (i == 0)
        {
            // the sernum is stored under address 0 and 2
            buf[4] = 0;		// address
        }
        else
        {
            buf[4] = 2;		// address
        }
            
        buf[5] = 0x00;	// address

        atCRC(5, buf + 1, buf + 6);
        
        cnt = 0;
        while (cnt < 20 && write(fd, buf, buf[1] + 1) != buf[1] + 1) 
        {
            // make some retries until chip wakes up
            cnt++;
        }
        if (cnt >= 20)
        {
            // chip did not respond
            close(fd);
            return -3;
        }
    
        cnt = 0;
        while (cnt < 20)
        {
            r = read(fd, buf, 10);
            if (r > 0 && buf[0] == 7) 
                break;
            cnt++;
        }
        if (cnt >= 20)
        {
            // chip did not respond
            close(fd);
            return -4;
        }
        
        for (cnt = 0; cnt < 4; cnt++)
        {
            i8uSernum[i * 4 + cnt] = buf[1 + cnt];
        }
        usleep(1000);
    }

    close(fd);
    return 0;
}    			

int read_ek(char *buf, int len); 
int getSerNumTpm(void)
{
	static int init = 0;

	if (!init) {
		read_ek(i8uSernum, 8);
		init = 1;
	}
	return 0;
} 

int getSerNum(void)
{

	struct stat s;

	stat("/dev/tpm0", &s);

	if (S_ISCHR(s.st_mode)){
		return getSerNumTpm();
	
	} else {
		return getSerNumI2c();
	}
}               

char *readSerNum(void)
{
    uint8_t cnt;
    static char sSernum[17];
    char *sHex = "0123456789ABCDEF";
        
    for (cnt = 0; cnt < 8; cnt++)
    {
        sSernum[cnt * 2] = sHex[i8uSernum[cnt] >> 4];
        sSernum[cnt * 2 + 1] = sHex[i8uSernum[cnt] & 0x0f];
    }
    sSernum[16] = 0;
    
    return sSernum;
}


