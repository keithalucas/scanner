#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "ports.h"

#define MAX_LINE 256


typedef struct port_list port_list_t;

struct port_list {
   port_item_t *head;
   port_item_t *tail;
};

static port_list_t list;

static void port_parse_line(char *line) {
   uint16_t port;
   char *type;
   port_item_t *item;

   /* strip leading spaces */
   while (isspace(*line)) line++;
   
   if ((*line == '\0') || (*line == '\n') || (*line == '#')) return;
   
   port = (uint16_t)strtol(line, &type, 10);

   if (port > 0) {
      while(isspace(*type)) type++;
   
      item = (port_item_t *)calloc(1, sizeof(port_item_t));
      item->port = port;
 
      if (0 == strcmp(type, "udp")) {
         item->type = PORT_TYPE_UDP;
      } else if (0 == strcmp(type, "tcp")) {
         item->type = PORT_TYPE_TCP;
      } 

      if (item->type != PORT_TYPE_UNKNOWN) {
         if (list.head == NULL) {
            list.head = list.tail = item;
         } else {
            item->prev = list.tail;
            list.tail->next = item;
            list.tail = item;
         }
      } else {
         free(item);
      }
   }
}
      
static void port_create_list (void) {
   FILE *f;
   char line[MAX_LINE];

   f = fopen("ports.txt", "r");

   if (f != NULL) {
      while((fgets(line, sizeof(line), f))) {
         line[strlen(line)-1] = '\0';
         port_parse_line(line);
      }
      fclose(f);  
   }
}

port_item_t *port_get_list(void) {
   if (list.head == NULL) {
      port_create_list();
   } 

   return list.head;
}
   
