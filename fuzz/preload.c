#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>

#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>


void stacktrace() {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  backtrace_symbols_fd(array, size, STDERR_FILENO);
}


static void* (*real_malloc)(size_t)=NULL;

static void mtrace_init(void)
{
    real_malloc = dlsym(RTLD_NEXT, "malloc");
    if (NULL == real_malloc) {
        fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
}

void *malloc(size_t size)
{
    if(real_malloc==NULL) {
        mtrace_init();
    }

    if (size > 1000000) {
        fprintf(stderr, "malloc(%zu)\n", size);
        stacktrace();
        asm("int3");
    }

    void *p = NULL;
    p = real_malloc(size);
    return p;
}
