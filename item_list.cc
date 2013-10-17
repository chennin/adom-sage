#include "jaakkos.h"
#include <ctype.h>

char item_list[ITEMS][ITEMSTRSZ];

static void trim_end(char *s) {
  while(isspace(s[strlen(s)-1]))
    s[strlen(s)-1] = '\0';
}

void load_item_list() {
  memset(item_list, 0, ITEMS*ITEMSTRSZ);

  FILE *fp = fopen("/var/lib/adom/item_list_111.txt", "r");
  if(!fp) fp = fopen("item_list_111.txt", "r");  
  if(!fp) return;

  for(int i=0; i<ITEMS; i++) {
    fgets(item_list[i], ITEMSTRSZ, fp);
    trim_end(item_list[i]);
  }

  fclose(fp);
}
