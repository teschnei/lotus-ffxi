#include <cassert>

#define DEBUG_BREAK() assert(false);
#define DEBUG_BREAK_IF(x)                                                                                                                                      \
    if (x)                                                                                                                                                     \
        assert(x);
