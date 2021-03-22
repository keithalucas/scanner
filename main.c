#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <linux/sockios.h>


static uint32_t valid_netmasks[] = {
   0xffffffff, 0xfffffffe, 0xfffffffc, 0xfffffff8, 
   0xfffffff0, 0xffffffe0, 0xffffffc0, 0xffffff80, 
   0xffffff00, 0xfffffe00, 0xfffffc00, 0xfffff800, 
   0xfffff000, 0xffffe000, 0xffffc000, 0xffff8000, 
   0xffff0000, 0xfffe0000, 0xfffc0000, 0xfff80000, 
   0xfff00000, 0xffe00000, 0xffc00000, 0xff800000, 
   0xff000000, 0xfe000000, 0xfc000000, 0xf8000000, 
   0xf0000000, 0xe0000000, 0xc0000000, 0x80000000
};

static int valid_netmask_recursive(uint32_t mask, int begin, int end) {
   int middle;
   
   if (begin == end) return 0;

   middle = (begin + end + 1) / 2;

   if (valid_netmasks[middle] == mask) return 1;
   if (valid_netmasks[middle] > mask) 
      return valid_netmask_recursive(mask, middle + 1, end);
   else 
      return valid_netmask_recursive(mask, begin, middle - 1);
}

static int is_valid_netmask(in_addr_t mask) {
   return valid_netmask_recursive(ntohl(mask), 0, sizeof(valid_netmasks) - 1);
}

static void usage (const char *program) {
   printf("Usage: %s <ip> <netmask>\n", program);
   exit(EXIT_FAILURE);
}

/* params in network byte order */
static int scan_ip_port(in_addr_t ip, uint16_t port) {
   int fd;
   struct sockaddr_in sin;

   memset(&sin, 0, sizeof(sin));
   sin.sin_family = AF_INET;
   sin.sin_addr.s_addr = ip;
   sin.sin_port = port; 
  

   fd = socket(AF_INET, SOCK_STREAM, 0);

   if (fd < 0) {
      perror("Cannot create socket.");
      exit(EXIT_FAILURE);
   }

   //  if (setsockopt(sd, SOL_SOCKET, SO_LINGER,  (const char *) &l, sizeof(struct li
   //  nger)))

   if (connect(fd, (struct sockaddr *)&sin, sizeof(sin)) == 0) {
       close(fd);
       return 1;
   } else {
       perror("Cannot connect()");
       close(fd);
       
       if (errno == ENETUNREACH) return -1;
       return 0;
   }
}

static char *ethernet_mactoa(struct sockaddr *addr) { 
   static char buff[256]; 
   unsigned char *ptr = (unsigned char *) addr->sa_data;

   sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X", 
           (ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377), 
           (ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377)); 

   return buff; 
} 


static void get_mac_address(in_addr_t ip) {
   int fd;
   struct arpreq areq;
   struct sockaddr_in *sin;

   memset(&areq, 0, sizeof(areq));
   sin = (struct sockaddr_in *) &areq.arp_pa;
   sin->sin_family = AF_INET;
   sin->sin_addr.s_addr = ip;
   sin = (struct sockaddr_in *) &areq.arp_ha;
   sin->sin_family = ARPHRD_ETHER;

   strncpy(areq.arp_dev, "eth0", 15);


   fd = socket(AF_INET, SOCK_DGRAM, 0);
   
   if (ioctl(fd, SIOCGARP, (caddr_t)&areq) < 0) {
      perror("ioctl()");
   }

   close(fd);
   printf("%s\n", ethernet_mactoa(&areq.arp_ha));
}


int main (int argc, char *argv[]) {
   struct in_addr addr;
   struct in_addr netmask;
   struct in_addr network;
   struct in_addr temp;

   if (argc != 3) {
      usage(argv[0]);
   }
 
   if (INADDR_NONE == inet_aton(argv[1], &addr)) {
      printf("Invalid ip.\n");
      usage(argv[0]);
   }

   if (INADDR_NONE == inet_aton(argv[2], &netmask)) {
      printf("Invalid netmask.\n");
      usage(argv[0]);
   } 

   if (!is_valid_netmask(netmask.s_addr)) {
      printf("Invalid netmask.\n");
      usage(argv[0]);
   } 

   network.s_addr = addr.s_addr & netmask.s_addr;
   temp.s_addr = network.s_addr;

   do {
      printf("%s\n", inet_ntoa(temp));

      /* Look for udp 5060, 4569 */
      //scan_ip_port(temp.s_addr, htons(5060));
      //scan_ip_port(temp.s_addr, htons(4569));
      get_mac_address(temp.s_addr);

      temp.s_addr = htonl(ntohl(temp.s_addr) + 1);
   } while (network.s_addr == (temp.s_addr & netmask.s_addr));
      
    

   exit(EXIT_SUCCESS);
   return 0;
}
   
