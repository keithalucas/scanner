#include <stdlib.h>
#include <string.h>

#include "udp_scan.h"

static void ip_addr_node_free(ip_addr_node_t *node) {
   ip_addr_free(node->ip);
   node->ip = NULL;
 
   free(node);
}

udp_port_t *udp_port_new(void) {
   udp_port_t *port;

   port = (udp_port_t *) calloc(1, sizeof(udp_port_t));

   return port;
}

void udp_port_free(udp_port_t *port) {
   free(port);
   port = NULL;
}

udp_port_list_t *udp_port_list_new(void) {
   udp_port_list_t *list;

   list = (udp_port_list_t *) calloc(1, sizeof(udp_port_list_t));

   list->udp_dest_port_lookup = g_hash_table_new(g_direct_hash, g_direct_equal);
   list->udp_src_port_lookup = g_hash_table_new(g_direct_hash, g_direct_equal);

   return list;
}

udp_port_t *udp_port_list_lookup_dest_port(udp_port_list_t *list,
                                           uint16_t dest_port) {
   udp_port_t *port;

   port = (udp_port_t *) g_hash_table_lookup(list->udp_dest_port_lookup, 
			    GUINT_TO_POINTER((uint32_t) dest_port));
   
   return port;
}

udp_port_t *udp_port_list_lookup_src_port(udp_port_list_t *list,
                                          uint16_t src_port) {
   udp_port_t *port;

   port = (udp_port_t *) g_hash_table_lookup(list->udp_src_port_lookup, 
			         GUINT_TO_POINTER((uint32_t) src_port));
   
   return port;
}

void udp_port_node_free(udp_port_node_t *node) {
   udp_port_free(node->port);
   node->port = NULL;
   free(node);
   node = NULL;
}

void udp_port_list_add_port(udp_port_list_t *list, udp_port_t *port) {
   udp_port_node_t *node;

   node = (udp_port_node_t *) calloc(1, sizeof (udp_port_node_t));

   node->port = port;
   node->next = NULL;
   node->prev = NULL;

   /* Add to linked list. */
   if (list->head == NULL) {
      list->head = list->tail = node;
   } else { 
      node->prev = list->tail;
      list->tail->next = node;
      list->tail = node;
   }

   /* Add to hash table. */
   if (port->dest_port != 0) {
      g_hash_table_insert(list->udp_dest_port_lookup, 
                          GUINT_TO_POINTER((uint32_t)port->dest_port), 
			  (gpointer) port);
   }
   
}

void udp_port_list_free(udp_port_list_t *list) {

   udp_port_node_t *node;

   if (list != NULL) {
      g_hash_table_destroy(list->udp_dest_port_lookup);
      g_hash_table_destroy(list->udp_src_port_lookup);
       
      list->udp_dest_port_lookup = NULL;
      list->udp_src_port_lookup = NULL;

      for (node = list->head;
           node != NULL;
           node = node->next) {
         udp_port_node_free(node);
      }
      list->head = NULL;
      list->tail = NULL;
   }      

   free(list);
}

ip_addr_t *ip_addr_new(void) {
   ip_addr_t *ip;

   ip = (ip_addr_t *) calloc(1, sizeof (ip_addr_t));

   ip->udp_port_list = udp_port_list_new();

   return ip;
}

void ip_addr_free(ip_addr_t *ip) {
   if (ip != NULL) {
      udp_port_list_free(ip->udp_port_list);
      ip->udp_port_list = NULL;
  
      free(ip);
   }
}  
   
ip_addr_list_t *ip_addr_list_new(void) {
   ip_addr_list_t *list;

   list = (ip_addr_list_t *) calloc(1, sizeof (ip_addr_list_t));

   list->ip_addr_lookup = g_hash_table_new(g_direct_hash, g_direct_equal);
                             
   return list;
}

void ip_addr_list_add_ip(ip_addr_list_t *list, ip_addr_t *ip) {
   ip_addr_node_t *node;

   node = (ip_addr_node_t *) calloc(1, sizeof (ip_addr_node_t));

   node->ip = ip;
   node->next = NULL;
   node->prev = NULL;

   /* Add to linked list. */
   if (list->head == NULL) {
      list->head = list->tail = node;
   } else { 
      node->prev = list->tail;
      list->tail->next = node;
      list->tail = node;
   }

   /* Add to hash table. */
   g_hash_table_insert(list->ip_addr_lookup, GUINT_TO_POINTER(ip->addr), 
                       (gpointer) ip);

}

ip_addr_t *ip_addr_list_lookup_ip(ip_addr_list_t *list, in_addr_t ip) {
   ip_addr_t *addr;

   addr = (ip_addr_t *) g_hash_table_lookup(list->ip_addr_lookup,
                                                 GUINT_TO_POINTER(ip));

   return addr;
}

void ip_addr_list_free(ip_addr_list_t *list) {
   ip_addr_node_t *node;

   if (list != NULL) {
       g_hash_table_destroy(list->ip_addr_lookup);
       list->ip_addr_lookup = NULL;

       for (node = list->head;
            node != NULL;
            node = node->next) {
          ip_addr_node_free(node);
      }
      list->head = NULL;
      list->tail = NULL;
   }      

   free(list);
}

void ip_addr_list_add_ip_port(ip_addr_list_t *list, in_addr_t ip, 
                              uint16_t dest_port) {
   ip_addr_t *addr;
   udp_port_t *port;

   addr = ip_addr_list_lookup_ip(list, ip);

   if (addr == NULL) {
      addr = ip_addr_new();
      addr->addr = ip;
 
      ip_addr_list_add_ip(list, addr);
   }

   port = udp_port_list_lookup_dest_port(addr->udp_port_list, dest_port);

   if (port == NULL) {
      port = udp_port_new();
      port->dest_port = dest_port;
      port->addr = addr;
   
      udp_port_list_add_port(addr->udp_port_list, port);
   }
}

void ip_addr_list_for_each(ip_addr_list_t *list,
                           int (*func)(ip_addr_t *, void *),
                           void *ptr) {
   ip_addr_node_t *node;

   for (node = list->head; node != NULL; node = node->next) {
      if (!(*func)(node->ip, ptr)) {
         break;
      }
   }
}

void udp_port_list_for_each(udp_port_list_t *list,
                            int (*func)(udp_port_t *, void *),
                            void *ptr) {
   udp_port_node_t *node;

   for (node = list->head; node != NULL; node = node->next) {
      if (!(*func)(node->port, ptr)) {
         break;
      }
   }
}

void udp_port_close(ip_addr_list_t *list, in_addr_t ip, uint16_t dest_port) {
   ip_addr_t *addr;
   udp_port_t *port;

   addr = ip_addr_list_lookup_ip(list, ip);

   if (addr == NULL) {
      return;
   }

   addr->status = IP_ADDR_UP;

   port = udp_port_list_lookup_dest_port(addr->udp_port_list, dest_port);

   if (port == NULL) {
      return;
   }

   port->status = UDP_PORT_CLOSED;
}

void ip_addr_is_up(ip_addr_list_t *list, in_addr_t ip, uint8_t *mac) {
   ip_addr_t *addr;

   addr = ip_addr_list_lookup_ip(list, ip);

   if (addr == NULL) {
      return;
   }

   addr->status = IP_ADDR_UP;
   memcpy(addr->mac_addr, mac, sizeof(addr->mac_addr));

}



