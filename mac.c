#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <ctype.h>

#define MAX_LINE 256

static GHashTable *mapping = NULL;


static int mac_parse_line(char *line) {
   uint32_t prefix;
   char *vendor = NULL;
   
   /* strip leading white space */
   while (isspace(*line)) line++;
  
   if ((*line == '#') || (*line == '\n') || (*line == '\0')) return 1;

   prefix = (uint32_t)strtol(line, &vendor, 16); 

   if (prefix != 0) {
      
      /* strip leading white space */
      while (isspace(*vendor)) vendor++;
 
      /* strip new line character */
      line = vendor;
      while (*line != '\n') line++;
      *line = '\0';    
      
      /* strip trailing white space */
      while (isspace(*(--line))) *line = '\0';

      if (*vendor == '\0') return 0;

      g_hash_table_insert(mapping, GUINT_TO_POINTER(prefix), 
                          g_strdup(vendor));
      return 1;
   }
   
   return 0;
      
}

void mac_load_config(void) {
   FILE *f;
   char line[MAX_LINE];
   int i = 0;

   f = fopen("mac.txt", "r");

   mapping = g_hash_table_new_full(g_direct_hash, g_direct_equal,
                                   NULL, g_free);
   
   if (f != NULL) { 
      while (NULL != (fgets(line, MAX_LINE, f))) {
         i++;
         if (!mac_parse_line(line)) {
            printf("mac.txt:%d Syntax error.  Ignoring.\n", i);
         }
      }
      fclose(f);
   }
}

const char *mac_lookup(uint8_t *mac) {
   uint32_t prefix;
   char *vendor;
   
   if (mapping == NULL) {
      mac_load_config();
   }

   prefix = (mac[0] << 16) | (mac[1] << 8) | (mac[2]);

   if ((vendor = g_hash_table_lookup(mapping, GUINT_TO_POINTER(prefix)))) {
      return vendor;
   }

   //return "Unknown";
   return NULL;

}

