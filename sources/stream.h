#ifndef STREAM_H
#define STREAM_H
#include "fs_call.h"
#include "int.h"

#define STREAM_BUFFER_SIZE 512

stream_t *create_stream(char *path, u8 *buffer);
int stream_putchar(char c, stream_t *stream);
int flush(stream_t *stream);
int close_stream(stream_t *stream);
#include "error.h"
#endif /* STREAM_H */

