#pragma once
#include <stddef.h>
void* ps_malloc(size_t size);
void* ps_realloc(void* ptr, size_t size);
bool psramFound();
