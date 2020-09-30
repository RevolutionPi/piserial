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

#include "project.h"

#include <sys/ioctl.h>

int getSerNum(void);
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
    
    getSerNum();
    
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
    
    getSerNum();
    
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
    sPassword[10] = 0;
    
    return sPassword;
}

int main(int argc, char *argv[])
{
    int cnt, r;
    TBOOL bShowSerNum = bTRUE;
    TBOOL bShowHostname = bTRUE;
    TBOOL bShowDefaultPassword = bTRUE;
    
    if (argc == 1)
    {
        // show all
    }
    else if (argc == 2)
    {
        if (strcmp(argv[1], "-s") == 0)
        {
            bShowSerNum = bTRUE;
            bShowHostname = bFALSE;
            bShowDefaultPassword = bFALSE;
        }
        else if (strcmp(argv[1], "-h") == 0)
        {
            bShowSerNum = bFALSE;
            bShowHostname = bTRUE;
            bShowDefaultPassword = bFALSE;
        }
        else if (strcmp(argv[1], "-p") == 0)
        {
            bShowSerNum = bFALSE;
            bShowHostname = bFALSE;
            bShowDefaultPassword = bTRUE;
        }
        else
        {
            argc = 3;
        }
    }
    
    if (argc > 2)
    {
        printf("usage: %s [-s|-h|-p]\n\n", argv[0]);
        printf("-s show serial number\n");
        printf("-h show hostname\n");
        printf("-p show inital password\n");
        printf("-t show inital password\n");
        printf("no argument: show all\n");
        exit(0);
        
    }

    cnt = 0;
    do
    {
        r = getSerNum();
        if (r < 0)
        {
            //printf("getSerNum() failed %d\n", r);
        }
        cnt++;
        if (cnt > 10)
        {
            printf("getSerNum() failed %d\n", r);
            exit(-1);
        }
    } while (r < 0);
    
    
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


