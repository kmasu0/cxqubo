#ifndef CXQUBO_MISC_COMPILER_H
#define CXQUBO_MISC_COMPILER_H

#include "cxqubo/config/cxqubo-config.h"

namespace cxqubo {
#if __has_attribute(noinline)
#define CXQUBO_ATTRIBUTE_NOINLINE __attribute__((noinline))
#else
#define CXQUBO_ATTRIBUTE_NOINLINE
#endif

#if __has_attribute(used)
#define CXQUBO_ATTRIBUTE_USED __attribute__((used))
#else
#define CXQUBO_ATTRIBUTE_USED
#endif

#if !defined(NDEBUG) || defined(CXQUBO_ENABLE_DUMP)
#define CXQUBO_DUMP_METHOD CXQUBO_ATTRIBUTE_NOINLINE CXQUBO_ATTRIBUTE_USED
#else
#define CXQUBO_DUMP_METHOD
#endif
} // namespace cxqubo

#endif
