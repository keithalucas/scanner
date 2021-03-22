#ifndef _INTERFACES_H_
#define _INTERFACES_H_

typedef struct interface_info interface_info_t;
typedef struct interface_info_list interface_info_list_t;

struct interface_info {
   char interface_name[IFNAMSIZ];
   struct in_addr addr;
   struct in_addr netmask;
   struct in_addr broadaddr;
   unsigned char mac_addr[IFHWADDRLEN];

   struct in_addr network;

   int cidr;
};

struct interface_info_list {
   int num;
  
   interface_info_t *list;
};

interface_info_t *interface_info_get(const char *name);
void interface_info_free(interface_info_t *info);
interface_info_list_t *interface_info_list_get(void);
void interface_info_list_free(interface_info_list_t *list);

#endif

