
#include <AvailabilityMacros.h>
#if MAC_OS_X_VERSION_MAX_ALLOWED < SDK_VERSION_CHECK
#error MAC_OS_X_VERSION_MAX_ALLOWED < SDK_VERSION_CHECK
#endif
int main(int argc, char** argv) { return 0; }
