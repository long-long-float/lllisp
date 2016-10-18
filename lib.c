#include <stdio.h>
#include <stdlib.h>

void printn(int n) {
  printf("%d\n", n);
}

char* itoa(int n) {
  // TODO: fix memory leak
    char* buf = malloc(sizeof(char) * 100);
    sprintf(buf, "%d", n);
    return buf;
}
