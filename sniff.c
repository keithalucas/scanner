#include <pcap.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "ip.h"
#include "list.h"
#include "ethernet.h"
#include "udp_send.h"
#include "select.h"

static pcap_t *pcap;
static struct bpf_program fp;  

/* TODO: 
         REMOVE PRINTING
*/

#define printf(str, ...) 

static void hex_dump(uint8_t *data, int len) {
   int i;

   for(i = 0; i < len; i++) {
      printf("%02x ", data[i]);
   
      if ((i + 1) % 8 == 0 ) printf("\n");
   }
}

static void decode_icmp(ip_generic_pkt_t *pkt, int len, ip_addr_list_t *list) {
   struct iphdr *iphdr;
   struct udphdr *udphdr;
   struct in_addr addr;

   if (IP_ICMP_PKT_CHK_LEN(len) < 0) {
      printf("Unable to decode ICMP.\n");
      return;
   }

   switch(pkt->ip_icmp_pkt.icmp_hdr.type) {
      case ICMP_ECHOREPLY:
         printf("ICMP Echo Reply\n");  
         hex_dump(pkt->ip_icmp_pkt.data, IP_ICMP_PKT_CHK_LEN(len));
         break;
      case ICMP_DEST_UNREACH:
         printf("ICMP Destination Unreachable\n"); 

         if (IP_ICMP_UNRCH_PKT_CHK_LEN(len) < 0) {
            printf("Unable to decode ICM Unreachable message.\n");
            return;
         }
         
         switch(pkt->ip_icmp_pkt.icmp_hdr.code) {
            case ICMP_NET_UNREACH:
               printf("Network Unreacheable\n");
               break;
            case ICMP_HOST_UNREACH:
               printf("Host Unreachable\n");
               break;
            case ICMP_PROT_UNREACH:
               printf("Protocol Unreachable\n");
               break;
            case ICMP_PORT_UNREACH:
               printf("Port Unreachable\n");

               iphdr = &(pkt->ip_icmp_unrch_pkt.ip_unrch_hdr);
               addr.s_addr = iphdr->daddr;
               printf(" Destination IP: %s\n", inet_ntoa(addr));
               addr.s_addr = iphdr->saddr;
               printf("      Source IP: %s\n", inet_ntoa(addr));

               udphdr = &(pkt->ip_icmp_unrch_pkt.udp_unrch_hdr);

               printf(" Destination Port: %d\n", ntohs(udphdr->dest));
               printf("      Source Port: %d\n", ntohs(udphdr->source));

               udp_port_close(list, iphdr->daddr, ntohs(udphdr->dest));


               break;
            default:
               break;
         }
            

         break;
      case ICMP_ECHO:
         printf("ICMP Echo\n"); 
         hex_dump(pkt->ip_icmp_pkt.data, IP_ICMP_PKT_CHK_LEN(len));
         break;
 
      default:
         printf("ICMP %x\n", pkt->ip_icmp_pkt.icmp_hdr.type); 
         hex_dump(pkt->ip_icmp_pkt.data, IP_ICMP_PKT_CHK_LEN(len));
         break;
   }
}
static void decode_udp(ip_generic_pkt_t *pkt, int len, ip_addr_list_t *list) {
   struct iphdr *iphdr;
   struct udphdr *udphdr;
   char *data;

   if (UDP_PKT_CHK_LEN(len) < 0) {
      printf("Unable to decode UDP.\n");
      return;
   }

   iphdr = &(pkt->udp_pkt.ip_hdr);
   udphdr = &(pkt->udp_pkt.udp_hdr);

   printf(" Destination Port: %d\n", ntohs(udphdr->dest));
   printf("      Source Port: %d\n", ntohs(udphdr->source));

   ip_addr_is_up(list, iphdr->daddr, pkt->eth_pkt.eth_hdr.ether_dhost);
   udp_port_open(list, iphdr->saddr, ntohs(udphdr->source));
   
   data = (char *)calloc(1, UDP_PKT_CHK_LEN(len) + 1);

   memcpy(data, pkt->udp_pkt.data, UDP_PKT_CHK_LEN(len));
   data[UDP_PKT_CHK_LEN(len)] = '\0';
   printf("%s\n", data);

   free(data);
}


static void process_packet(u_char *data, const struct pcap_pkthdr *header,
                           const u_char *packet) {
   struct iphdr *iphdr;
   struct in_addr addr;
   ip_generic_pkt_t *pkt;
   ip_addr_list_t *list = (ip_addr_list_t *)data;

   pkt = (ip_generic_pkt_t *)packet;

   if (ETH_PKT_CHK_LEN(header->caplen) < 0) {
      printf("Unable to decode ethernet.\n");
      return;
   }

   printf("Destination MAC: %s\n", 
          ethernet_mactoa(pkt->eth_pkt.eth_hdr.ether_dhost));
   printf("     Source MAC: %s\n", 
          ethernet_mactoa(pkt->eth_pkt.eth_hdr.ether_shost));

   if (ntohs(pkt->eth_pkt.eth_hdr.ether_type) == ETHERTYPE_IP) {
      iphdr = &(pkt->ip_pkt.ip_hdr);

      if (IP_PKT_CHK_LEN(header->caplen) < 0) {
         printf("Unable to decode IP packet.\n");
         return;
      }

      addr.s_addr = iphdr->daddr;
      printf(" Destination IP: %s\n", inet_ntoa(addr));
      addr.s_addr = iphdr->saddr;
      printf("      Source IP: %s\n", inet_ntoa(addr));

      switch(iphdr->protocol) {
         case IPPROTO_ICMP:
            printf("           ICMP\n");
            decode_icmp(pkt, header->caplen, list);
            break;
         case IPPROTO_TCP:
            printf("            TCP\n");
            break;
         case IPPROTO_UDP:
            printf("            UDP\n");
            decode_udp(pkt, header->caplen, list);
            break;
         default:
            break;
      }
   }

   printf("\n");
}

static int packet_handler(int fd, int flags, void *list) {
    pcap_dispatch(pcap, 1, process_packet, (void *)list);
    
    return 1;
}


int sniff_init(const char *device, in_addr_t net, ip_addr_list_t *list) {
   char errbuf[PCAP_ERRBUF_SIZE];
   char filter[] = "icmp or udp";

   pcap = pcap_open_live(device, MAX_PACKET_LEN, 0, 25, errbuf);
 
   if (pcap == NULL) {
      return 0;
   }

   if (pcap_compile(pcap, &fp, filter, 0, net) == -1) {
      return 0;
   }

   if (pcap_setfilter(pcap, &fp) == -1) {
      return 0;
   }

   select_file_add(pcap_get_selectable_fd(pcap), SELECT_FLAGS_READ, 
                   packet_handler, list);
   return 1;
}


void sniff_run_iter(ip_addr_list_t *list) {
   int fd = pcap_get_selectable_fd(pcap);
   fd_set rd;
   int r;
   struct timeval tv = {0, 1000};

   FD_ZERO(&rd);
   FD_SET(fd, &rd);

   r = select(fd + 1, &rd, NULL, NULL, &tv); 

   if (r == -1 && errno == EINTR) return;
   
   if (r < 0) {
      perror("select()");
      exit(EXIT_FAILURE);
   }

   if (r > 0) {
      pcap_dispatch(pcap, 1, process_packet, (void *)list);
   }
} 

void sniff_cleanup(void) {
   pcap_freecode(&fp);
   pcap_close(pcap);
}
