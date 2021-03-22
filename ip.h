#ifndef _IP_H_
#define _IP_H_

#include <stdint.h>
#include <netinet/in.h>

#define MAX_PACKET_LEN 65535

typedef struct eth_pkt eth_pkt_t;
typedef struct ip_pkt ip_pkt_t;
typedef struct ip_icmp_pkt ip_icmp_pkt_t;
typedef struct ip_icmp_unrch_pkt ip_icmp_unrch_pkt_t;
typedef struct udp_pkt udp_pkt_t;
typedef union ip_generic_pkt ip_generic_pkt_t;

struct eth_pkt {
   struct ether_header eth_hdr;
   uint8_t             data[0];
} __attribute__ ((__packed__));

struct ip_pkt {
   struct ether_header eth_hdr;
   struct iphdr        ip_hdr;
   uint8_t             data[0];
} __attribute__ ((__packed__));

struct ip_icmp_pkt {
   struct ether_header eth_hdr;
   struct iphdr        ip_hdr;
   struct icmphdr      icmp_hdr;
   uint8_t             data[0];
} __attribute__ ((__packed__));

struct ip_icmp_unrch_pkt {
   struct ether_header eth_hdr;
   struct iphdr        ip_hdr;
   struct icmphdr      icmp_hdr;
   struct iphdr        ip_unrch_hdr;
   struct udphdr       udp_unrch_hdr;
   uint8_t             data[0];
} __attribute__ ((__packed__));


struct udp_pkt {
   struct ether_header eth_hdr;
   struct iphdr        ip_hdr;
   struct udphdr       udp_hdr;
   uint8_t             data[0];
} __attribute__ ((__packed__));

union ip_generic_pkt {
   uint8_t raw_data[MAX_PACKET_LEN];
   eth_pkt_t eth_pkt;
   ip_pkt_t ip_pkt;
   ip_icmp_pkt_t ip_icmp_pkt;
   ip_icmp_unrch_pkt_t ip_icmp_unrch_pkt;
   udp_pkt_t udp_pkt;
};

/* These return the number of bytes after the struct. */
#define ETH_PKT_CHK_LEN(len) (len - sizeof(eth_pkt_t))
#define IP_PKT_CHK_LEN(len) (len - sizeof(ip_pkt_t))
#define IP_ICMP_PKT_CHK_LEN(len) (len - sizeof(ip_icmp_pkt_t))
#define IP_ICMP_UNRCH_PKT_CHK_LEN(len) (len - sizeof(ip_icmp_unrch_pkt_t))
#define UDP_PKT_CHK_LEN(len) (len - sizeof(udp_pkt_t))

#endif


