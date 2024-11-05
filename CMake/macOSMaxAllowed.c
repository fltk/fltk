
#include <AvailabilityMacros.h>
#if __MAC_OS_X_VERSION_MAX_ALLOWED < SDK_VERSION_CHECK
#error __MAC_OS_X_VERSION_MAX_ALLOWED < SDK_VERSION_CHECK
#endif
int main(int argc, char** argv) { return 0; }
