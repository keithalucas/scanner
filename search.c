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
#include <linux/if.h>

#include "interfaces.h"
#include "list.h"
#include "udp_send.h"
#include "tcp_send.h"
#include "sniff.h"
#include "ethernet.h"
#include "mac.h"
#include "results.h"
#include "select.h"
#include "ports.h"

static void usage(const char *progname) {
   printf("%s <interface>\n", progname);
}

static int iter_udp_addr(udp_port_t *port, void *ptr) {
   interface_info_t *info = (interface_info_t *)ptr;

   udp_send_msg(port, info);

   return 1;
}

static int iter_tcp_addr(tcp_port_t *port, void *ptr) {
   tcp_send_msg(port);

   return 1;
}


static int iter_ip_addr(ip_addr_t *addr, void *ptr) {
   interface_info_t *info = (interface_info_t *)ptr;

   ip_get_mac_address(addr, info->interface_name);

   udp_port_list_for_each(addr->udp_port_list, iter_udp_addr, ptr);
   tcp_port_list_for_each(addr->tcp_port_list, iter_tcp_addr, ptr);

   return 1;
}

static void generate_ips(interface_info_t *info,
                         in_addr_t net,
                         ip_addr_list_t *ip_list) {
   in_addr_t ip;
   port_item_t *list;
   port_item_t *item;

   ip = htonl(ntohl(net) + 1);

   list = port_get_list();

   do {
      if ((ip != info->broadaddr.s_addr) &&
          (ip != info->addr.s_addr)) {
         for(item = list; item != NULL; item = item->next) {
            if (item->type == PORT_TYPE_UDP) {
               ip_addr_list_add_ip_udp_port(ip_list, ip, item->port);
            } else if (item->type == PORT_TYPE_TCP) {
               ip_addr_list_add_ip_tcp_port(ip_list, ip, item->port);
            }
         }
      }
      
      ip = htonl(ntohl(ip) + 1);
   } while (net == (ip & info->netmask.s_addr));


}

int main(int argc, char *argv[]) {
   ip_addr_list_t *ip_list;
   interface_info_t *info;

   if (argc != 2) {
      usage(argv[0]);
      exit(EXIT_FAILURE);
   }

   ip_list = ip_addr_list_new();

   info = interface_info_get(argv[1]);

   if (info == NULL) {
      printf("Invalid interface name.\n");
      usage(argv[0]);
      exit(EXIT_FAILURE);
   }
 

   generate_ips(info, info->network.s_addr, ip_list);

   if (!sniff_init(argv[1], info->network.s_addr, ip_list)) {
      printf("Cannot initialize sniffer.\n");
      exit(EXIT_FAILURE);
   }
   
   printf("Searching %s/%d.\n\n", inet_ntoa(info->network), info->cidr);
      
   ip_addr_list_for_each(ip_list, iter_ip_addr, info);

   while (!(udp_is_done(ip_list) && tcp_is_done(ip_list))) {
      select_file_do_iter(5);

      udp_check_timeouts(ip_list, 10000, info);
      tcp_check_timeouts(ip_list, 10000);
   }

   results_display(ip_list);

   interface_info_free(info);

   ip_addr_list_free(ip_list);

   exit(EXIT_SUCCESS);
}
