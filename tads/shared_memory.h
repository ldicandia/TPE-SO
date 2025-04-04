#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <stddef.h>

// Funciones para manejar memoria compartida
void *create_shared_memory(const char *name, size_t size);
void *attach_shared_memory(const char *name, size_t size, int flags, int prot);
void detach_shared_memory(void *ptr, size_t size);
void destroy_shared_memory(const char *name);

#endif