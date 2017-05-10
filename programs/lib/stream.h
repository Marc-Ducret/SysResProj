#ifndef STREAM_H
#define STREAM_H
#include "int.h"

#define STREAM_BUFFER_SIZE 512
#define NUM_STREAMS 50
#define NUM_CHANNELS_PROC 10

typedef enum {
    S_UNUSED, S_FILE, S_CHANNEL
} sstatus_t;

typedef int sid_t; // Stream id type

#define STDIN (int) 0
#define STDOUT (sid_t) 1
#define STDERR (sid_t) 1 // Easier to manage

typedef struct {
    sstatus_t status;
    u32 size;   // Max size
    u32 index;  // Next free index
    u8 *buffer; // Address of the buffer
    union {
        fd_t file;  // File descriptor linked to stream
        int chanid; // Channel id (equal to streamid)
    };
} stream_t;

sid_t create_stream(char *path);
sid_t create_channel_stream(int chanid);
int stream_putchar(char c, sid_t id);
int flush(sid_t id);
int close_stream(sid_t id);
#include "malloc.h"
#include "error.h"
#include "syscall.h"

#endif /* STREAM_H */