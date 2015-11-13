
#include <stdio.h>

#include "net/mia.h"

void mia_dump_ip(const int pos)
{
    printf("%i.%i.%i.%i",
           mia_buf[pos], mia_buf[pos + 1], mia_buf[pos + 2], mia_buf[pos + 3]);
}

void mia_dump_mac(const int pos)
{
    printf("0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x",
          mia_buf[pos], mia_buf[pos + 1], mia_buf[pos + 2],
          mia_buf[pos + 3], mia_buf[pos + 4], mia_buf[pos + 5]);
}

