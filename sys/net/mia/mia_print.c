
#include <stdio.h>

#include "net/mia.h"


void mia_print_ip_addr(uint8_t *ip)
{
    printf("%i.%i.%i.%i", ip[0], ip[1], ip[2], ip[3]);
}

void mia_print_mac_addr(uint8_t *mac)
{
    printf("%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
