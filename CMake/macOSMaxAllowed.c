#include <Availability.h>
#include <stdio.h>

int main(int argc, char **argv) {
  unsigned int ver = __MAC_OS_X_VERSION_MAX_ALLOWED;
  printf("%d.%d.%d", ver/10000, (ver/100)%100, ver%100);
  return 0;
}
