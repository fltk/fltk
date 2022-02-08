/*
  FLTK feature test: do we have PTHREAD_MUTEX_RECURSIVE ?
*/
#include <pthread.h>
int main() {
  return PTHREAD_MUTEX_RECURSIVE;
}
