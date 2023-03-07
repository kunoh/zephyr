#include_next <bits/c++config.h>

#if defined(__ZEPHYR__) && defined(_GLIBCXX_HAVE_LINUX_FUTEX)
#undef _GLIBCXX_HAVE_LINUX_FUTEX
#endif