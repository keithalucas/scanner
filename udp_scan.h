#ifndef _UDP_SCAN_H_
#define _UDP_SCAN_H_

#include <glib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <sys/time.h>
#include <time.h>


typedef enum udp_port_status udp_port_status_t;
typedef struct udp_port udp_port_t;
typedef struct udp_port_node udp_port_node_t;
typedef struct udp_port_list udp_port_list_t;
typedef enum ip_addr_status ip_addr_status_t;
typedef struct ip_addr ip_addr_t;
typedef struct ip_addr_node ip_addr_node_t;
typedef struct ip_addr_list ip_addr_list_t;

enum udp_port_status {
   UDP_PORT_UNKNOWN,
   UDP_PORT_OPEN,
   UDP_PORT_CLOSED
};

enum ip_addr_status {
   IP_ADDR_UNKNOWN,
   IP_ADDR_UP,
   IP_ADDR_DOWN
};

struct udp_port {
   uint16_t dest_port;
   uint16_t src_port;

   struct timeval start_time;
   
   int sockfd;

   udp_port_status_t status;

   ip_addr_t *addr;
};

struct udp_port_node {
   udp_port_t *port;
  
   udp_port_node_t *next;
   udp_port_node_t *prev;
}; 

struct udp_port_list {
   udp_port_node_t *head;
   udp_port_node_t *tail;

   GHashTable *udp_dest_port_lookup;    
   GHashTable *udp_src_port_lookup;    
};
   
struct ip_addr {
   in_addr_t addr;

   ip_addr_status_t status;

   uint8_t mac_addr[IFHWADDRLEN];

   udp_port_list_t *udp_port_list;
};

struct ip_addr_node {
   ip_addr_t *ip;

   ip_addr_node_t *next;
   ip_addr_node_t *prev;
};

struct ip_addr_list {
   ip_addr_node_t *head;
   ip_addr_node_t *tail;

   GHashTable *ip_addr_lookup;
};

udp_port_t *udp_port_new(void);
void udp_port_free(udp_port_t *port);

udp_port_list_t *udp_port_list_new(void);
udp_port_t *udp_port_list_lookup_dest_port(udp_port_list_t *list, 
                                           uint16_t dest_port);
udp_port_t *udp_port_list_lookup_src_port(udp_port_list_t *list, 
                                           uint16_t src_port);
void udp_port_list_add_port(udp_port_list_t *list, udp_port_t *port);
void udp_port_list_free(udp_port_list_t *list);

ip_addr_t *ip_addr_new(void);
void ip_addr_free(ip_addr_t *ip);

ip_addr_list_t *ip_addr_list_new(void);
void ip_addr_list_add_ip(ip_addr_list_t *list, ip_addr_t *ip);
ip_addr_t *ip_addr_list_lookup_ip(ip_addr_list_t *list, in_addr_t ip);
void ip_addr_list_free(ip_addr_list_t *list);

void ip_addr_list_add_ip_port(ip_addr_list_t *list, in_addr_t ip, 
                              uint16_t dest_port);

void ip_addr_list_for_each(ip_addr_list_t *list,
                           int (*func)(ip_addr_t *, void *),
                           void *ptr);
void udp_port_list_for_each(udp_port_list_t *list,
                            int (*func)(udp_port_t *, void *),
                            void *ptr);
void udp_port_close(ip_addr_list_t *list, in_addr_t ip, uint16_t dest_port);
void ip_addr_is_up(ip_addr_list_t *list, in_addr_t ip, uint8_t *mac);
#endif   
