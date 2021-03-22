#ifndef _SNIFF_H_
#define _SNIFF_H_

int sniff_init(const char *device, in_addr_t net, ip_addr_list_t *list);
void sniff_run_iter(ip_addr_list_t *list);
void sniff_cleanup(void);

#endif
