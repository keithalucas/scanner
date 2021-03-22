#ifndef _TCP_SEND_H_
#define _TCP_SEND_H_
void tcp_send_msg(tcp_port_t *port);
int tcp_is_done(ip_addr_list_t *list);
void tcp_check_timeouts(ip_addr_list_t *list, int timeout);
#endif
