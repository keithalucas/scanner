#ifndef _ETHERNET_H_
#define _ETHERNET_H_

char *ethernet_mactoa(u_int8_t *ptr);
void ip_get_mac_address(ip_addr_t *addr, const char *device);


#endif
