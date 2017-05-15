#include "stream.h"
stream_t static_stream; // TODO malloc or stream table

stream_t *create_stream(char *path, u8 *buffer) {
    // Creates a stream writing at specified location.
    oflags_t flags = O_WRONLY | O_CREAT | O_TRUNC;
    fd_t file = fopen(path, flags);
    if (file < 0) {
        // Error while opening file.
        return NULL;
    }
    file_table[file].process = -1; // To be accessible by every process while kernel mode.
    stream_t *stream = &static_stream;
    kprintf("Warning : only one stream may exist at a time.\n");
    
    stream->buffer = buffer;
    stream->file = file;
    stream->size = STREAM_BUFFER_SIZE;
    stream->index = 0;
    
    return stream;
}

int stream_putchar(char c, stream_t *stream) {
    if (stream->index == stream->size) {
        int res = flush(stream);
        if (res == -1) {
            // Error while flush
            return -1;
        }
        stream->index = 0;
    }
    stream->buffer[stream->index] = c;
    stream->index ++;

    return 0;
}

int flush(stream_t *stream) {
    // Writes content of stream buffer to the disk.
    if (stream == NULL) {
        kprintf("Null stream\n");
        return -1;
    }
    u32 remaining = stream->index;
    int written = 0;
    u8 *index = stream->buffer;
    while (remaining > 0) {
        written = write(stream->file, index, remaining);
        if (written == -1) {
            // Error while writing
            return -1;
        }
        remaining -= written;
        index += written;
    }
    return written;
}

int close_stream(stream_t *stream) {
    // Closes the stream after flushing it a last time.
    if (flush(stream) == -1) {
        // Error while flush
        return -1;
    }
    close(stream->file);
    // TODO In case of stream table, deallocate the corresponding entry.
    return 0;
}

