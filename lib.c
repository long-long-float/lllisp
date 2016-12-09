#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

typedef struct _ilist {
  int32_t         car;
  struct _ilist  *cdr; // if cdr is NULL, this ilist is nil
} ilist;

void printn(int n) {
  printf("%d\n", n);
}

void printl(ilist *xs) {
  printf("(");
  for (ilist *cur = xs; cur->cdr; cur = cur->cdr) {
    printf("%d", cur->car);
    if (cur->cdr->cdr) {
      printf(" ");
    }
  }
  printf(")\n");
}

ilist* cons(int32_t car, ilist *cdr) {
  // TODO: fix memory leak
  ilist *list = malloc(sizeof(ilist));
  list->car = car;
  list->cdr = cdr;
  return list;
}

ilist NIL = {0, NULL};
ilist* nil() {
  return &NIL;
}

char* itoa(int n) {
  // TODO: fix memory leak
  char* buf = malloc(sizeof(char) * 100);
  snprintf(buf, 100, "%d", n);
  return buf;
}


