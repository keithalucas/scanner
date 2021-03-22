#ifndef _TIME_H_
#define _TIME_H_

void tv_add_ms(struct timeval *dest, struct timeval *src, int ms);
int tv_cmp(struct timeval *t, struct timeval *s);

#endif
