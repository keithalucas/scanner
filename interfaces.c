#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <arpa/inet.h>

#include "interfaces.h"

#define MAXNUMINTF 50

static void get_cidr(interface_info_t *interface) {
   uint32_t netmask = ntohl(interface->netmask.s_addr);
   int i;
   int cidr = 0;

   for (i = 0; i < 32; i++) {
      if (((netmask >> (31 - i)) & 0x1) == 1) {
         cidr++;
      } else {
         break;
      }
   }
   
   interface->cidr = cidr;
}
   

static void get_more_info(int sock, const char *name, 
                          interface_info_t *interface) {
   struct ifreq netmaskreq;
   struct ifreq broadreq;
   struct ifreq hwreq;
   struct sockaddr_in *sa_in;
   struct sockaddr *sa;
   
   strncpy(netmaskreq.ifr_name, name, IFNAMSIZ);

   if (ioctl(sock, SIOCGIFNETMASK , (char *) &netmaskreq) >= 0) {
      sa_in = (struct sockaddr_in *) &netmaskreq.ifr_netmask;

      memcpy(&interface->netmask, &sa_in->sin_addr, sizeof(struct in_addr));
   } 

   strncpy(broadreq.ifr_name, name, IFNAMSIZ);

   if (ioctl(sock, SIOCGIFBRDADDR , (char *) &broadreq) >= 0) {
      sa_in = (struct sockaddr_in *) &broadreq.ifr_broadaddr;

      memcpy(&interface->broadaddr, &sa_in->sin_addr, sizeof(struct in_addr));
   } 

   strncpy(hwreq.ifr_name, name, IFNAMSIZ);

   if (ioctl(sock, SIOCGIFHWADDR , (char *) &hwreq) >= 0) {
      sa = (struct sockaddr *) &hwreq.ifr_hwaddr;

      memcpy((char *)interface->mac_addr, &sa->sa_data, IFHWADDRLEN);
   }

   interface->network.s_addr = interface->addr.s_addr & 
                               interface->netmask.s_addr;

   get_cidr(interface); 
}

interface_info_t *interface_info_get(const char *name) {
   int sock;
   struct ifreq ipreq;
   struct sockaddr_in *sa_in;
   interface_info_t *interface;
   
   sock = socket(AF_INET, SOCK_DGRAM, 0);

   if (sock < 0) {
      perror("socket");
      return NULL;
   }

   strncpy(ipreq.ifr_name, name, IFNAMSIZ);

   if (ioctl(sock, SIOCGIFADDR , (char *) &ipreq) >= 0) {
      sa_in = (struct sockaddr_in *) &ipreq.ifr_addr;

      interface = (interface_info_t *) calloc(1, sizeof(interface_info_t));
      memcpy(&interface->addr, &sa_in->sin_addr, sizeof(struct in_addr));

      get_more_info(sock, name, interface);

      close(sock);

      return interface;
   } else { 
      perror("ioctl");
      return NULL;
   }
}

interface_info_list_t *interface_info_list_get(void) {
   int sock;
   int nintfs;
   int i;
   interface_info_t *interfaces;
   interface_info_list_t *list;
   struct ifconf ifconf;
   struct ifreq ifreq[MAXNUMINTF];
   struct sockaddr_in *sa;


   memset(&ifconf, 0, sizeof (ifconf));
   memset(&ifreq, 0, sizeof (ifreq));

   ifconf.ifc_buf = (char *) &ifreq;
   ifconf.ifc_len = sizeof(ifreq);

   sock = socket(AF_INET, SOCK_DGRAM, 0);

   if (sock < 0) {
      perror("socket");
      return NULL;
   }

   if (ioctl(sock, SIOCGIFCONF, (char *) &ifconf) < 0) {
      perror("ioctl");
      close(sock);
      return NULL;
   }
 
   nintfs = ifconf.ifc_len / sizeof(struct ifreq);
   interfaces = (interface_info_t *) calloc(nintfs, 
                                            sizeof (interface_info_t));

   list = (interface_info_list_t *) calloc(1, sizeof (interface_info_list_t));
   list->num = nintfs;
   list->list = interfaces;
   

   for (i = 0; i < nintfs; i++) {
      strncpy(interfaces[i].interface_name, ifreq[i].ifr_name, IFNAMSIZ);
     
      sa = (struct sockaddr_in *) &ifreq[i].ifr_addr;

      memcpy(&interfaces[i].addr, &sa->sin_addr, sizeof(struct in_addr));
  
      get_more_info(sock, ifreq[i].ifr_name, &interfaces[i]);
   }

   close(sock);

   return list;
}

void interface_info_free(interface_info_t *interfaces) {
   free(interfaces);
}
   
void interface_info_list_free(interface_info_list_t *list) {
   int i;

   for (i = 0; i < list->num; i++) {
      interface_info_free(&list->list[i]);
   }

   free(list);
}
#if 0

static char *ethernet_mactoa(unsigned char *ptr) {
   static char buff[256];

   sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X",
           (ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
           (ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377));

   return buff;
}


int main(int argc, char *argv[]) {
   int nintfs;
   int i;
   interface_info_t *interfaces = interfaces_get(&nintfs);

   for (i = 0; i < nintfs; i++) {
      printf("%s\n", interfaces[i].interface_name);
      printf("%s\n", inet_ntoa(interfaces[i].addr));
      printf("%s\n", inet_ntoa(interfaces[i].netmask));
      printf("%s\n", ethernet_mactoa(interfaces[i].mac_addr));
   }

   interfaces_free(interfaces);

   return 0;
}
#endif
