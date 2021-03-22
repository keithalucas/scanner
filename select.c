#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>

#include "select.h"

typedef struct select_file select_file_t;
typedef struct select_list select_list_t;

struct select_file {
   int fd;
  
   int flags;

   int (*handler)(int fd, int flags, void *ptr);   

   void *ptr;

   int is_closed;

   select_file_t *next;
   select_file_t *prev;
};

struct select_list {
   select_file_t *head;
   select_file_t *tail;

   int max_fd;

   GHashTable *fd_mapping;
};

static select_list_t *select_list = NULL;

static void select_list_init(void) {
   if (select_list == NULL) {
      select_list = (select_list_t *) calloc (1, sizeof(select_list_t));

      select_list->fd_mapping = g_hash_table_new(g_direct_hash, g_direct_equal);
   }
}

static int select_file_populate_fds(fd_set *read_fds, fd_set *write_fds,
                                    fd_set *except_fds) {
   select_file_t *file;
   int max_fd = 0;

   FD_ZERO(read_fds);
   FD_ZERO(write_fds);
   FD_ZERO(except_fds);

   if (select_list == NULL) return 0;

   for (file = select_list->head; file != NULL; file = file->next) {
      if (file->is_closed) continue;

      if (file->flags & SELECT_FLAGS_READ) {
         FD_SET(file->fd, read_fds);
      } else if (file->flags & SELECT_FLAGS_WRITE) { 
         FD_SET(file->fd, write_fds);
      } else if (file->flags & SELECT_FLAGS_EXCEPT) { 
         FD_SET(file->fd, except_fds);
      }
       
      if (file->flags) {
         max_fd = MAX(max_fd, file->fd);
      } 
   }

   return max_fd + 1;
}

static void select_file_dispatch(fd_set *read_fds, fd_set *write_fds,
                                 fd_set *except_fds) {
   select_file_t *file;
   int flags = 0;

   for (file = select_list->head; file != NULL; file = file->next) {
      flags = 0;
      if (file->is_closed) continue;

      if (FD_ISSET(file->fd, read_fds)) {
         flags |= SELECT_FLAGS_READ;
      } 

      if (FD_ISSET(file->fd, write_fds)) {
         flags |= SELECT_FLAGS_WRITE;
      }
  
      if (FD_ISSET(file->fd, except_fds)) {
         flags |= SELECT_FLAGS_EXCEPT;
      }

      if (flags > 0) {
         /* TODO Remove if returns 0. */
         (*file->handler)(file->fd, flags, file->ptr);
      }
   }
}

static void select_file_set_timeval(struct timeval *tv, int ms) {
   if (ms > 0) {
      tv->tv_sec = (ms % 1000);
      if (ms > 0) ms /= 1000;
      tv->tv_usec = (ms * 1000);
   } else {
      tv->tv_sec = 0;
      tv->tv_usec = 0;
   }
}


int select_file_add(int fd, int flags, 
                    int (*handler)(int fd, int flags, void *ptr),
                    void *ptr) {
   select_file_t *file;

   select_list_init();

   if (NULL != g_hash_table_lookup(select_list->fd_mapping, 
                                   GINT_TO_POINTER(fd))) {
      return 0;
   }

   file = (select_file_t *) calloc (1, sizeof(select_file_t));

   file->fd = fd;
   file->flags = flags;
   file->handler = handler;
   file->ptr = ptr;
   file->next = NULL;
   file->prev = NULL;

   if (select_list->head == NULL) {
      select_list->head = select_list->tail = file;
   } else {
      file->prev = select_list->tail;
      select_list->tail->next = file;
      select_list->tail = file;
   }
   
   g_hash_table_insert(select_list->fd_mapping, GINT_TO_POINTER(fd),
                       (gpointer) file);

   return 1;
}    

void *select_file_get_data(int fd) {
   select_file_t *file;

   select_list_init();
 
   file = (select_file_t *) g_hash_table_lookup(select_list->fd_mapping,
                                                GINT_TO_POINTER(fd));

   return file ? file->ptr : NULL;
}

int select_file_remove(int fd) {
   select_file_t *file;

   for (file = select_list->head; file != NULL; file = file->next) {
      if (file->fd == fd) {
         /* uh oh this could be called when this list is being traversed
            just flag it so we don't mess up memory. */

         file->is_closed = 1;
 
         g_hash_table_remove(select_list->fd_mapping, GINT_TO_POINTER(fd));
         return 1;
      }
   }

   return 0;
}

int select_file_remove_all(void) {
   if (select_list == NULL) return 0;

   return 1;
}

int select_file_do_iter(int ms) {
   int nfds;
   fd_set read_fds;
   fd_set write_fds;
   fd_set except_fds;
   struct timeval tv;
   int rv;

   nfds = select_file_populate_fds(&read_fds, &write_fds, &except_fds); 
   select_file_set_timeval(&tv, ms);

   rv = select(nfds, &read_fds, &write_fds, &except_fds, &tv);

   if (rv < 0) {
      perror("select()");
      return -1;
   } else if (rv == 0) {
      return 0;
   } else { 
      select_file_dispatch(&read_fds, &write_fds, &except_fds);
      return rv;
   }
}

