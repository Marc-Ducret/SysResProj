#include "fs_call.h"
dirent_t static_dirent; // Temporary
fd_t cwd; // Current working directory
        
fd_t openfile(char *path, oflags_t flags);
int close(fd_t fd);
int read(fd_t fd, void *buffer, int offset, int length);
int write(fd_t fd, void *buffer, int offset, int length);
int seek(fd_t fd, seek_cmd_t seek_command, int offset);

void mkdir(char *path);
void rmdir(char *path);
void chdir(char *path);
char *getcwd() {
    // Returns the path from root to current working directory cwd.
    char path[MAX_PATH_NAME];
    
    fd_t fd = cwd;
    dirent_t *dirent = readdir(fd);
}

fd_t opendir(char *path) {
    // TODO It supposes that it is an absolute path from root.
    fd_t fd;
    dirent_t *dirent_p;
    
    // Initialisation : open the source directory.
    if (*path == DIR_SEP) {
        dirent_t dirent;
        dirent.cluster = 0;
        dirent.name[0] = '/';
        dirent.name[1] = 0;
        dirent.type = DIR;
        
        fd = opendir_ent(&dirent); // Opens root directory.
        for (; *path == DIR_SEP; path++) {} // Removes all starting separators
    }
    else {
        kprintf("Non supported.");
        return -1;
    }
    
    //char nextdir[MAX_FILE_NAME];
    char *nextdir;
    // Now, follows the path until end of it.
    while (*path) {
        path = nextdirname(path, &nextdir);
        
        // Search entry nextdir in the previous directory.
        dirent_p = finddir(fd, nextdir);
        
        assert(dirent_p != NULL); // Not NULL pointer
        
        // Close current directory and opens nextdir.
        closedir(fd);
        fd = opendir_ent(dirent_p);
        
        for (; *path == DIR_SEP; path++) {} // Removes all following separators
        }
    return fd;
}

fd_t opendir_ent(dirent_t *dirent) {
    assert(dirent->type == DIR); // We want to open a directory !
    
    fd_t fd = new_fd();
    strCopy(dirent->name, file_table[fd].name);
    file_table[fd].type = DIR;
    file_table[fd].start_cluster = dirent->cluster;
    file_table[fd].curr_cluster = file_table[fd].start_cluster;
    file_table[fd].curr_offset = 0;
    
    return fd;
}

dirent_t *readdir(fd_t fd) {
    // Reads the next entry of the directory.
    // If there is no such entry, returns NULL.
    assert(file_table[fd].type == DIR);
    
    u32 cluster = file_table[fd].curr_cluster;
    u8 buffer[fs.cluster_size];
    u32 next_cluster = 0;
    
    // Create a buffer to store the name.
    char name[MAX_FILE_NAME];
    char *name_index;
    name_index = name;

    do {
        read_cluster(cluster, buffer);

        for (u32 i = file_table[fd].curr_offset; i < fs.cluster_size; i += 32) {
            if (buffer[i] == 0) {
                // No more directories or files.
                file_table[fd].curr_cluster = cluster;
                file_table[fd].curr_offset = i;
                return NULL;
            }
            if (buffer[i] == 0xE5) {
                // Unused entry
                continue;
            }
            if (buffer[i + 11] == 0x0F) {
                // Long name entry
                long_file_name_t *lfn = (long_file_name_t *) (buffer+i);
                get_long_name(lfn, name_index);
                name_index += 13;
                continue;
            }            
            else {
                // This is a directory entry.
                directory_entry_t *dirent = (directory_entry_t *) (buffer+i);
                
                // Removes ending spaces
                for (; *name_index == ' '; name_index--) {} 
                *(name_index + 1) = 0;
                
                //dirent_t *dirent_p = kmalloc(sizeof(dirent_t)); // TODO !
                dirent_t *dirent_p = &static_dirent; // TODO !
                dirent_p->type = dirent->attributes.dir ? DIR : FILE;
                dirent_p->cluster = get_cluster(dirent);
                
                if (name_index == name) {
                    // Not a logn filename
                    get_short_name(dirent, name);
                }
                strCopy(name, dirent_p->name);
                
                file_table[fd].curr_cluster = cluster;
                file_table[fd].curr_offset = i + 32;
                
                return dirent_p;
            }
        }
        next_cluster = get_next_cluster(cluster);
    } while (next_cluster < 0x0FFFFFF8);
    
    // End of directory.
    file_table[fd].curr_cluster = cluster;
    file_table[fd].curr_offset = fs.cluster_size;
    
    return NULL;
}

void rewinddir(fd_t fd) {
    file_table[fd].curr_cluster = file_table[fd].start_cluster;
    file_table[fd].curr_offset = 0;
}

void closedir(fd_t fd) {
    assert(file_table[fd].type == DIR); // No such directory.
    free_fd(fd);
}

dirent_t *finddir(fd_t dir, char *name) {
    // Find the directory entry corresponding to directory name.
    dirent_t *dirent;
    while ((dirent = readdir(dir))) {
        if (strEqual(dirent->name, name)) {
            if (dirent->type == DIR)
                return dirent;
            //assert(0);
            return NULL;    // This is not a directory.
        }
    }
    //assert(0);
    return NULL;    // Entry doesn't exist.
}

dirent_t *cluster_finddir(fd_t dir, u32 cluster) {
    // Find the directory entry corresponding to directory name.
    dirent_t *dirent;
    while ((dirent = readdir(dir))) {
        if (dirent->cluster, cluster) {
            if (dirent->type == DIR)
                return dirent;
            //assert(0);
            return NULL;    // This is not a directory.
        }
    }
    //assert(0);
    return NULL;    // Entry doesn't exist.
}

void print_short_dirent(dirent_t *dirent) {
    kprintf("%s %s\n", (dirent->type == DIR) ? "Directory" : "File",
            dirent->name);
}

void test_dir() {
    kprintf("Testing Directory Calls :\n");
    kprintf("Content of root directory : \n");
    fd_t root = opendir("/");
    dirent_t *dirent = readdir(root);
    print_short_dirent(dirent);
    closedir(root);
    
    kprintf("\nContent of boot directory : \n");
    fd_t boot = opendir("/boot");
    while ((dirent = readdir(boot))) {
        print_short_dirent(dirent);
    }
    closedir(boot);
    
    kprintf("\nContent of grub directory : \n");
    fd_t grub = opendir("////boot//grub/");
    while ((dirent = readdir(grub))) {
        print_short_dirent(dirent);
    }
    closedir(grub); 
}