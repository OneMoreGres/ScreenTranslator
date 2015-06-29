#ifndef ST_ASSERT_H
#define ST_ASSERT_H

#include <assert.h>

#if !defined(ST_ASSERT)
#  if defined(ST_NO_ASSERT)
#    define ST_ASSERT(CONDITION)
#  else
#    define ST_ASSERT(CONDITION) assert(CONDITION)
#  endif
#endif

#endif // ST_ASSERT_H

