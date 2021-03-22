#ifndef _SELECT_H_
#define _SELECT_H_

typedef enum select_flags select_flags_t;

enum select_flags {
   SELECT_FLAGS_READ = 0x1,
   SELECT_FLAGS_WRITE = 0x2,
   SELECT_FLAGS_EXCEPT = 0x4,
   SELECT_FLAGS_ALL = 0x7
};

/* 
 * fd - file descriptor to add
 * flags - file conditions to select for
 * handler - function to call with conditions are met
 *               fd - fd
 *               flags - flags met on select (not flags passed to func)
 *               ptr - user defined data
 *           returns 0 to remove
 * ptr - user defined data
 *
 * returns 1 if successfully added.
 */
int select_file_add(int fd, int flags, 
                    int (*handler)(int fd, int flags, void *ptr), 
                    void *ptr);

void *select_file_get_data(int fd);

int select_file_remove(int fd);

int select_file_remove_all(void);

int select_file_do_iter(int ms);


#endif
