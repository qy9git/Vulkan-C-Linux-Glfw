#ifndef __DEBUGV2__
#define __DEBUGV2__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern void debug_memory_init(const size_t);
extern void debug_memory_end(void);

extern void *debug_malloc(const size_t, const char * const restrict, const size_t);
extern void *debug_calloc(const size_t, const size_t, const char * const restrict, const size_t);
extern void debug_free(void * restrict, const char * const restrict, const size_t);
extern void *debug_realloc(void * restrict, const size_t, const char * const restrict, const size_t);


extern FILE *debug_fopen(const char * restrict, const char * restrict, const char * const restrict, const size_t);
extern int debug_fclose(FILE * restrict, const char * const restrict, const size_t);
extern FILE *debug_tmpfile(const char * const restrict, const size_t);
extern FILE *debug_freopen(const char * restrict, const char * restrict, FILE * restrict, const char * const restrict, const size_t);
#ifndef __IN_SOURCE_FILE

#define malloc(x) debug_malloc(x,__FILE__,__LINE__)
#define free(x) debug_free(x,__FILE__,__LINE__)
#define realloc(x,y) debug_free(x,y,__FILE__,__LINE__)
#define calloc(x,y) debug_calloc(x,y,__FILE__,__LINE__)
//aligned_alloc not supported

#define fopen(x,y) debug_fopen(x,y,__FILE__,__LINE__)
#define fclose(x) debug_fclose(x,__FILE__,__LINE__)
#define tmpfile() debug_tmpfile(__FILE__,__LINE__)
#define freopen(x,y,z) debug_freopen(x,y,z,__FILE__,__LINE__)

#endif
#endif
