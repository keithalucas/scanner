#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <glib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "list.h"
#include "time.h"
#include "udp_send.h"
#include "interfaces.h"

struct timeout_info {
   int timeout;
   interface_info_t *info;
   int status;
};


static int udp_check_timeout(udp_port_t *port, void *ptr) {
   struct timeout_info *ti = (struct timeout_info *)ptr;
   struct timeval now;
   struct timeval timeouttime;
   int timeout = ti->timeout / port->retry_limit;

   if (port->status == UDP_PORT_UNKNOWN) {
      gettimeofday(&now, NULL);
      tv_add_ms(&timeouttime, &port->start_time, timeout);

      if (0 < tv_cmp(&now, &timeouttime)) {
         if (++port->retries == port->retry_limit) {
	         port->status = UDP_PORT_OPEN_FILTERED;
         } else {
            udp_send_msg(port, ti->info);
            ti->status = 0;
         }
      } else {
         ti->status = 0;
      }
   }
  
   return 1;
}

static int ip_check_timeout(ip_addr_t *addr, void *ptr) {
   udp_port_list_for_each(addr->udp_port_list, udp_check_timeout, ptr);
   return 1;
}

int udp_check_timeouts(ip_addr_list_t *list, int timeout, 
                        interface_info_t *info) {
   struct timeout_info ti;

   ti.timeout = timeout;
   ti.info = info;
   ti.status = 1;

   ip_addr_list_for_each(list, ip_check_timeout, &ti);
 
   return ti.status;
}

static int udp_check_status(udp_port_t *port, void *ptr) {
   int *status = (int *)(ptr);

   if (port->status == UDP_PORT_UNKNOWN) {
      *status = 0;
   } 
  
   return 1;
}

static int ip_check_status(ip_addr_t *addr, void *ptr) {
   udp_port_list_for_each(addr->udp_port_list, udp_check_status, ptr);

   return 1;
}

int udp_is_done(ip_addr_list_t *list) {
   int status = 1;

   ip_addr_list_for_each(list, ip_check_status, &status);


   return status;
}
   

void udp_send_msg(udp_port_t *port, interface_info_t *info) {
   char *msg;
   struct sockaddr_in sin;


   port->sockfd = socket(AF_INET, SOCK_DGRAM, 0);

   if (port->sockfd < 0) {
      port->sockfd = 0;
      return;
   }

   memset(&sin, 0, sizeof(sin));
   sin.sin_family = AF_INET;
   sin.sin_addr.s_addr = port->addr->addr;
   sin.sin_port = htons(port->dest_port);

   msg = g_strdup_printf("INVITE sip:callee@%s SIP/2.0\r\n"
                         "Via: SIP/2.0/UDP %s:5060\r\n"
                         "From: \"keith\" <sip:keith@%s>\r\n"
                         "To: \"keith\" <sip:keith@%s>\r\n"
                         "Content-Type: application/sdp\r\n"
                         "Content-Length: 0\r\n",
                         inet_ntoa(info->addr), 
                         inet_ntoa(info->addr), 
                         inet_ntoa(info->addr), 
                         inet_ntoa(sin.sin_addr));


   if (sendto(port->sockfd, msg, strlen(msg), 0, 
          (struct sockaddr *)&sin, sizeof(sin)) <= 0) {
      perror("sendto");
   }

   close(port->sockfd);
   port->sockfd = 0;

   g_free(msg);

   gettimeofday(&port->start_time, NULL);
}

