#define __IN_SOURCE_FILE
#include "debugV2.h"
typedef struct{
    void *ptr;
    size_t size;
}memory_t;

typedef struct{
    FILE *ptr;
    char name[FILENAME_MAX];
}file_t;

size_t total_memory,total_files, mem_buffer_size;
memory_t *mem_buffer;
file_t *file_buffer;
FILE *debugV2_log;

void debug_memory_end(void){
    free(mem_buffer);
    free(file_buffer);
    if(debugV2_log)fclose(debugV2_log);
}
void debug_memory_init(const size_t size){
    mem_buffer_size = size;
    if(size<1u){
        fputs("debug_memory_init: oprand is too small\n",stderr);
        exit(EXIT_FAILURE);
    }
    mem_buffer = calloc(size,sizeof(memory_t));
    if(mem_buffer == NULL){
        debug_memory_end();
        fputs("debug_memory_init: calloc failure\n",stderr);
        exit(EXIT_FAILURE);
    }
    file_buffer = calloc(FOPEN_MAX,sizeof(file_t));
    if(file_buffer == NULL){
        debug_memory_end();
        fputs("debug_memory_init: calloc failure\n",stderr);
        exit(EXIT_FAILURE);
    }
    debugV2_log = fopen("debugV2.log.c","w");
    if(debugV2_log == NULL){
        debug_memory_end();
        fputs("debug_memory_init: fopen failure\n",stderr);
        exit(EXIT_FAILURE);
    }
    fprintf(debugV2_log,"FOPEN_MAX = %u\nFILENAME_MAX = %u\n\n",FOPEN_MAX,FILENAME_MAX);
}

static void debug_log_memory(void){
    fputs("\nheap = {\n",debugV2_log);
    for(size_t i=0u;i<mem_buffer_size;++i)
        if(mem_buffer[i].ptr != NULL)
            fprintf(debugV2_log,"\t%zu: %p holds %zu bytes\n",i,mem_buffer[i].ptr,mem_buffer[i].size);
    fprintf(debugV2_log,"}\ntotal heap usage = %zu\n\n",total_memory);
}
static void debug_log_file(void){
    fputs("\nfiles = {\n",debugV2_log);
    for(size_t i=0u;i<FOPEN_MAX;++i)
        if(file_buffer[i].ptr != NULL)
            fprintf(debugV2_log,"\t%zu: %s\n",i,file_buffer[i].name);
    fprintf(debugV2_log,"}\nNumber of currently open files = %zu\n\n",total_files);
}

void *debug_malloc(const size_t size, const char * const restrict file, const size_t line){
    void* ptr = malloc(size);
    if(ptr == NULL){
        fprintf(debugV2_log,"//END Malloc failed at line, at line %zu in the file \"%s\"\n",line,file);
        debug_log_memory();
        return NULL;
    }
    for(size_t i=0u;i<mem_buffer_size;++i)
        if(mem_buffer[i].ptr == NULL){
            mem_buffer[i].ptr = ptr;
            mem_buffer[i].size = size;
            break;
        }
    total_memory += size;
    fprintf(debugV2_log,"Allocated: %zu bytes to %p, at line %zu in the file \"%s\"\n",size,ptr,line,file);
    debug_log_memory();
    return ptr;
}
void *debug_calloc(const size_t count, const size_t element,const char * const restrict file, const size_t line){
    void* ptr = calloc(count,element);
    size_t size = count*element;
    if(ptr == NULL){
        fprintf(debugV2_log,"//END Calloc failed at line, at line %zu in the file \"%s\"\n",line,file);
        debug_log_memory();
        return NULL;
    }
    for(size_t i=0u;i<mem_buffer_size;++i)
        if(mem_buffer[i].ptr == NULL){
            mem_buffer[i].ptr = ptr;
            mem_buffer[i].size = size; //overflow risk
            break;
        }
    total_memory += size;
    fprintf(debugV2_log,"Allocated: %zu bytes to %p, at line %zu in the file \"%s\"\n",size,ptr,line,file);
    debug_log_memory();
    return ptr;
}
void debug_free(void * restrict ptr, const char * const restrict file, const size_t line){
    size_t size;
    for(size_t i=0u;i<mem_buffer_size;++i)
        if(mem_buffer[i].ptr == ptr){
            mem_buffer[i].ptr = NULL;
            size = mem_buffer[i].size;
            mem_buffer[i].size = 0u;
            break;
        }
    total_memory -= size;
    fprintf(debugV2_log,"Freed: %zu bytes at %p, at line %zu in the file \"%s\"\n",size,ptr,line,file);
    free(ptr);
    debug_log_memory();
}
void *debug_realloc(void * restrict ptr, const size_t size, const char * const restrict file, const size_t line){
    size_t index;
    for(size_t i=0u;i<mem_buffer_size;++i){
        if(mem_buffer[i].ptr == ptr){
            index = i;
            break;
        }
    }
    fprintf(debugV2_log,"%p",ptr);
    ptr = realloc(ptr,size);
    if(ptr == NULL){
        fprintf(debugV2_log,"//END Realloc failed at line, at line %zu in the file \"%s\"\n",line,file);
        debug_log_memory();
        return NULL;
    }
    if(size<=mem_buffer[index].size)
        total_memory -= mem_buffer[index].size-size;
    else
        total_memory += size-mem_buffer[index].size;

    mem_buffer[index].ptr = ptr;
    mem_buffer[index].size = size;
    fprintf(debugV2_log," is Reallocated to %p now holds %zu, at line %zu in the file \"%s\"\n",ptr,size,line,file);
    debug_log_memory();
    return ptr;
}

FILE *debug_fopen(const char * restrict name, const char * restrict mode, const char * const restrict file, const size_t line){
    FILE *ptr = fopen(name,mode);
    if(ptr == NULL){
        fprintf(debugV2_log,"//END fopen \"%s\" with mode \"%s\" failed at line %zu in the file \"%s\"\n",name,mode,line,file);
        debug_log_file();
        return NULL;
    }
    for(size_t i=0u;i<FOPEN_MAX;++i)
        if(file_buffer[i].ptr == NULL){
            file_buffer[i].ptr = ptr;
            strcpy(file_buffer[i].name,name);
            break;
        }
    ++total_files;
    fprintf(debugV2_log,"Opened \"%s\" with mode \"%s\", at line %zu in the file \"%s\"\n",name,mode,line,file);
    debug_log_file();
    return ptr;
}
int debug_fclose(FILE * restrict ptr, const char * const restrict file, const size_t line){
    size_t index;
    for(size_t i=0u;i<FOPEN_MAX;++i)
        if(file_buffer[i].ptr == ptr){
            index = i;
            break;
        }
    if(fclose(ptr)){
        fprintf(debugV2_log,"//END fclose \"%s\" failed at line %zu in the file \"%s\"\n",file_buffer[index].name,line,file);
        debug_log_file();
        return EOF;
    }
    --total_files;
    file_buffer[index].ptr = NULL;
    fprintf(debugV2_log,"Closed \"%s\" at line %zu in the file \"%s\"\n",file_buffer[index].name,line,file);
    debug_log_file();
    return 0;
}
FILE *debug_tmpfile(const char * const restrict file, const size_t line){
    FILE * ptr = tmpfile();
    if(ptr == NULL){
        fprintf(debugV2_log,"//END tmpfile failed at line %zu in the file \"%s\"\n",line,file);
        debug_log_file();
        return NULL;
    }
    for(size_t i=0u;i<FOPEN_MAX;++i)
        if(file_buffer[i].ptr == NULL){
            file_buffer[i].ptr = ptr;
            strcpy(file_buffer[i].name,"tmpfile");
            break;
        }
    ++total_files;
    fprintf(debugV2_log,"Opened a temporary file, at line %zu in the file \"%s\"\n",line,file);
    debug_log_file();
    return ptr;
}
FILE *debug_freopen(const char * restrict name, const char * restrict mode, FILE * restrict ptr, const char * const restrict file, const size_t line){
    size_t index;
    for(size_t i=0u;i<FOPEN_MAX;++i)
        if(file_buffer[i].ptr == ptr){
            index = i;
            break;
        }
    ptr = freopen(name,mode,ptr);
    if(ptr == NULL){
        fprintf(debugV2_log,"//END freopen \"%s\" to \"%s\" with \"%s\" failed at line %zu in the file \"%s\"\n",file_buffer[index].name,name,mode,line,file);
        debug_log_file();
        return NULL;
    }
    file_buffer[index].ptr = ptr;
    strcpy(file_buffer[index].name,name);
    fprintf(debugV2_log,"Closed \"%s\" at line %zu in the file \"%s\"\n",file_buffer[index].name,line,file);
    debug_log_file();
    return ptr;
}
