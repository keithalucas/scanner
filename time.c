#include <sys/time.h>
#include <time.h>

void tv_add_ms(struct timeval *dest, struct timeval *src, int ms) {
   int us = ms * 1000;

   dest->tv_sec = src->tv_sec;
   dest->tv_usec = src->tv_usec + us;

   if (dest->tv_usec >= 1000000) {
      dest->tv_usec -= 1000000;
      dest->tv_sec++;
   }
}

int tv_cmp(struct timeval *t, struct timeval *s) {
   if (t->tv_sec < s->tv_sec) return -1;
   if (t->tv_sec > s->tv_sec) return 1;
   if (t->tv_usec < s->tv_usec) return -1;
   if (t->tv_usec > s->tv_usec) return 1;
   return 0;
}

