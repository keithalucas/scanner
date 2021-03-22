#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "select.h"
#include "list.h"
#include "time.h"

static int tcp_handler(int sockid, int flags, void *ptr) {
   tcp_port_t *port = (tcp_port_t *) ptr;
   int optval;
   socklen_t optlen = sizeof (int);

  if (0 == getsockopt(port->sockfd, SOL_SOCKET, SO_ERROR,
                       (char *) &optval, &optlen)) {
      switch (optval) {
         case 0:
            select_file_remove(port->sockfd);
            port->status = TCP_PORT_OPEN;
            port->addr->status = IP_ADDR_UP;
            close(port->sockfd);
            break;

         default:
            select_file_remove(port->sockfd);
            port->status = TCP_PORT_CLOSED;
            close(port->sockfd);
            break;
      }
   } else {
      perror("getsockopt");
   }

   return 1;
} 

void tcp_send_msg(tcp_port_t *port) {
   int flags;
   int rv;

   memset(&port->sin, 0, sizeof(port->sin));
   port->sin.sin_family = AF_INET;
   port->sin.sin_addr.s_addr = port->addr->addr;
   port->sin.sin_port = htons(port->dest_port);

   port->sockfd = socket(AF_INET, SOCK_STREAM, 0);

   if (port->sockfd < 0) {
      perror("socket()");
      return;
   }

   flags = fcntl(port->sockfd, F_GETFL, 0);

   if (flags < 0) {
      perror("fcntl()");
      return;
   }
 
   flags = fcntl(port->sockfd, F_SETFL, flags | O_NONBLOCK);

   if (flags < 0) {
      perror("fcntl()");
      return;
   }

   rv = connect(port->sockfd, (struct sockaddr *)&port->sin, sizeof(port->sin));

   if (rv == 0) {
      port->status = TCP_PORT_OPEN;
      port->addr->status = IP_ADDR_UP;
      close(port->sockfd);
   } else {
      switch (errno) {
         case EINPROGRESS:
         case EAGAIN:
         case EALREADY:
         case EISCONN:
         case EINTR:
            select_file_add(port->sockfd, SELECT_FLAGS_WRITE, 
                            tcp_handler, port);
            /* try again */
            break;

         default:
            /* remove fd if it's there */
            port->status = TCP_PORT_CLOSED;
            close(port->sockfd);
            break;
      }
   }
 
   gettimeofday(&port->start_time, NULL);
}

static int tcp_check_status(tcp_port_t *port, void *ptr) {
   int *status = (int *)(ptr);

   if (port->status == TCP_PORT_UNKNOWN) {
      *status = 0;
   }

   return 1;
}

static int ip_check_status(ip_addr_t *addr, void *ptr) {
   tcp_port_list_for_each(addr->tcp_port_list, tcp_check_status, ptr);

   return 1;
}

int tcp_is_done(ip_addr_list_t *list) {
   int status = 1;

   ip_addr_list_for_each(list, ip_check_status, &status);


   return status;
}
                                  
static int tcp_check_timeout(tcp_port_t *port, void *ptr) {
   int timeout = GPOINTER_TO_INT(ptr);
   struct timeval now;
   struct timeval timeouttime;

   if (port->status == TCP_PORT_UNKNOWN) {
      gettimeofday(&now, NULL);
      tv_add_ms(&timeouttime, &port->start_time, timeout);

      if (0 < tv_cmp(&now, &timeouttime)) {
         port->status = TCP_PORT_CLOSED;
      }
   }

   return 1;
}

static int ip_check_timeout(ip_addr_t *addr, void *ptr) {
   tcp_port_list_for_each(addr->tcp_port_list, tcp_check_timeout, ptr);
   return 1;
}

void tcp_check_timeouts(ip_addr_list_t *list, int timeout) {
   ip_addr_list_for_each(list, ip_check_timeout, GINT_TO_POINTER(timeout));
}

