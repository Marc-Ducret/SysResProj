#ifndef PRINTING_H
#define PRINTING_H

#include "int.h"
#include "lib.h"
#include "stream.h"

void kprintf(const char* data, ...);
void fprintf(sid_t sid, const char* data, ...);
#endif

