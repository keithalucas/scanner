#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>


#include "list.h"

static int mac_is_empty(uint8_t *ptr) {
   int i;

   for(i = 0; i < 6; i++) {
      if (ptr[i] & 0377) return 1;
   }
   return 0;
}
char *ethernet_mactoa(uint8_t *ptr) {
   static char buf[256];

   sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x",
           (ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
           (ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377));

   return buf;
}

void ip_get_mac_address(ip_addr_t *addr, const char *device) {
   int fd;
   struct arpreq areq;
   struct sockaddr_in *sin;
   struct sockaddr *sa;

   memset(&areq, 0, sizeof(areq));
   sin = (struct sockaddr_in *) &areq.arp_pa;
   sin->sin_family = AF_INET;
   sin->sin_addr.s_addr = addr->addr;
   sin = (struct sockaddr_in *) &areq.arp_ha;
   sin->sin_family = ARPHRD_ETHER;

   strncpy(areq.arp_dev, device, 15);

   fd = socket(AF_INET, SOCK_DGRAM, 0);

   if (ioctl(fd, SIOCGARP, (caddr_t)&areq) < 0) {
      //perror("ioctl()");
   } else {
      sa = (struct sockaddr *) &areq.arp_ha;
      memcpy(addr->mac_addr, &sa->sa_data, sizeof(addr->mac_addr));
      if (mac_is_empty(addr->mac_addr)) puts(ethernet_mactoa(addr->mac_addr));
   }

   close(fd);

}


