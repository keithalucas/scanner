#include <stdio.h>

#include "list.h"
#include "ethernet.h"
#include "mac.h"

static int result_udp_addr(udp_port_t *port, void *ptr) {

   switch (port->status) {
      case UDP_PORT_OPEN:
         printf("%d\tUDP\tOpen\n", port->dest_port);
         break;

      case UDP_PORT_OPEN_FILTERED:
         printf("%d\tUDP\tOpen (filtered)\n", port->dest_port);
         break;
   
      case UDP_PORT_CLOSED:
         printf("%d\tUDP\tClosed\n", port->dest_port);
         break;
      
      default:
         break;
      
   }

   return 1;
}

static int result_tcp_addr(tcp_port_t *port, void *ptr) {
   switch (port->status) {
      case TCP_PORT_OPEN:
         printf("%d\tTCP\tOpen\n", port->dest_port);
         break;

      case TCP_PORT_OPEN_FILTERED:
         printf("%d\tTCP\tOpen (filtered)\n", port->dest_port);
         break;
   
      case TCP_PORT_CLOSED:
         printf("%d\tTCP\tClosed\n", port->dest_port);
         break;
      
      default:
         break;
      
   }

   return 1;
}

static int result_ip_addr(ip_addr_t *addr, void *ptr) {
   struct in_addr in;

   in.s_addr = addr->addr;

   if (addr->status == IP_ADDR_UP) {
      if (addr->sends_icmp_unreach) {
         printf("IP %s (%s - %s) is up.\n", inet_ntoa(in),
             ethernet_mactoa(addr->mac_addr),
             mac_lookup(addr->mac_addr) ? mac_lookup(addr->mac_addr) : "Unknown"); 
      } else {
         printf("IP %s (%s - %s) is up (filtered). \n", inet_ntoa(in),
             ethernet_mactoa(addr->mac_addr),
             mac_lookup(addr->mac_addr) ? mac_lookup(addr->mac_addr) : "Unknown"); 
      }
      printf("Port\tType\tState\n");
      printf("----------------------------------------\n");
  
      udp_port_list_for_each(addr->udp_port_list, result_udp_addr, ptr);
      tcp_port_list_for_each(addr->tcp_port_list, result_tcp_addr, ptr);
      printf("\n");
   }


   return 1;
}

void results_display(ip_addr_list_t *list) {
   ip_addr_list_for_each(list, result_ip_addr, NULL);
}

