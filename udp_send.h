#ifndef _UDP_SEND_H_
#define _UDP_SEND_H_

#include "list.h"
#include "interfaces.h"

void udp_send_msg(udp_port_t *port, interface_info_t *info);

int udp_check_timeouts(ip_addr_list_t *list, int timeout,
                        interface_info_t *info);

int udp_is_done(ip_addr_list_t *list);

#endif
