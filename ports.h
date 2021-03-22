#ifndef _PORTS_H_
#define _PORTS_H_

typedef enum port_type port_type_t;
typedef struct port_item port_item_t;

enum port_type {
   PORT_TYPE_UNKNOWN,
   PORT_TYPE_UDP,
   PORT_TYPE_TCP,
};

struct port_item {
   uint16_t port;

   port_type_t type;

   port_item_t *prev;
   port_item_t *next;
};

port_item_t *port_get_list(void);

#endif
