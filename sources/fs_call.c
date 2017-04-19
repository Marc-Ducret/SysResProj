#include "fs_call.h"
dirent_t static_dirent; // Temporary
char static_path_start[MAX_PATH_NAME]; // Temporary
u32 static_path_offset = 0;
long_file_name_t static_longnames[MAX_FILE_NAME / 13 + 2];
directory_entry_t static_entry;
fd_t cwd; // Current working directory

void print_short_dirent(dirent_t *dirent) {
    kprintf("%s %s at offset %d, %d entries;\n", 
            (dirent->type == DIR) ? "Directory" : "File",
            dirent->name,
            dirent->ent_offset,
            dirent->size);
}

fd_t openfile(char *path, oflags_t flags);
int close(fd_t fd);
int read(fd_t fd, void *buffer, int offset, int length);
int write(fd_t fd, void *buffer, int offset, int length);
int seek(fd_t fd, seek_cmd_t seek_command, int offset);

void fill_entries(u32 cluster, u32 offset, u32 size, void *entries) {
    // Copy the filename entries and final entry at offset from cluster.
    // The number of entries is size.
    while (offset >= fs.cluster_size) {
        cluster = get_next_cluster(cluster);
        size -= fs.cluster_size;
    }
    u8 content[fs.cluster_size];
    read_cluster(cluster, content);
    memcpy(content + offset, entries, size * 32);
    write_cluster(cluster, content);
}

void fill_dir_entry(u32 cluster, directory_entry_t *dirent, char *name) {
    dirent->attributes.dir = 1;
    dirent->attributes.rd_only = 0;
    dirent->attributes.system = 0;
    dirent->attributes.hidden = 0;
    dirent->attributes.archive = 1; // Dirty ?
    dirent->cluster_high = cluster >> 16;
    dirent->cluster_low = cluster & 0xFFFF;
    u32 size = strlen(name);
    assert(size <= 11);
    strCopy(name, dirent->file_name);
    
    for (u32 i = size; i < 11; i++)
        dirent->file_name[i] = ' '; // Pads with spaces
    dirent->file_size = 0;
}

char static_fresh_name[12]; // TODO
char *get_fresh_name() {
    static int count = 0;
    char *buffer = static_fresh_name;
    *buffer = 'f';
    write_int(buffer+1, count);
    count++;
    return buffer;
}

void *build_entries(u32 parent_cluster, char *name) {
    // Builds the several long name entries to put in the father directory for 
    // the new directory.
    // Also builds the final entry
    
    char buffer[MAX_FILE_NAME];
    memset(buffer, 0, MAX_FILE_NAME);
    strCopy(name, buffer);
    int len = strlen(buffer);
    u32 nb = len / 13 + ((len % 13) ? 1 : 0);
    kprintf("Nb of lfn : %d\n", nb);
    char *fresh_name = get_fresh_name();
    char sum = 0;
    for (int j = 0; j < 11; j++) {
        sum = (((sum & 1) << 7) | ((sum & 0xfe) >> 1)) + fresh_name[j];
    }
    
    for (u32 i = 1; i < nb+1; i++) {
        long_file_name_t *p = &static_longnames[nb - i];
        p->attribute = 0x0F;
        p->long_entry_type = 0;
        p->zero = 0;
        p->order = i;
        p->checksum = sum;
        set_long_name(p, name);
        name += 13;
        char buf[14];
        get_long_name(p, buf);
        kprintf("String %s\n", buf);
    }
    
    // Changes order field for last entry
    static_longnames[0].order |= 0x40;
    
    // Asks for a new cluster
    u32 fresh_cluster = new_cluster();
    
    // Fills in the base links (parent and current directory)
    u8 content[fs.cluster_size];
    memset(content, 0, fs.cluster_size);
    fill_dir_entry(fresh_cluster, (directory_entry_t *) content, CUR_DIR_NAME);
    fill_dir_entry(parent_cluster, (directory_entry_t *) (&content[32]), PARENT_DIR_NAME);
    kprintf("Parent cluster %d\n", parent_cluster);
    kprintf("Fresh cluster %d\n", fresh_cluster);
    write_cluster(fresh_cluster, content);

    // Creates final entry
    directory_entry_t *dirent = (directory_entry_t *) &(static_longnames[nb]);
    // TODO Add time things..
    fill_dir_entry(fresh_cluster, dirent, fresh_name);

    return static_longnames;
}

void mkdir(char *path) {
    char *dir_name = dirname(path);
    char *file_name = basename(path);
    char file[MAX_FILE_NAME];
    assert(strlen(file_name) < MAX_FILE_NAME);
    
    strCopy(file_name, file);
    
    fd_t fd = opendir(dir_name);
    assert(fd >= 0); // No such directory
    
    // Asserts directory doesn't exist
    assert(finddir(fd, file) == NULL);
    
    // Build corresponding entries
    long_file_name_t *entries = build_entries(file_table[fd].start_cluster, file);

    // Computes the size of full entry
    u32 size = 0;
    char test[14];
    kprintf("LFN entries : ");
    while (entries[size].attribute == 0x0F) {
        get_long_name(&entries[size], test);
        kprintf("%s ", test);
        size ++;
    }
    size ++;
    
    kprintf("\n");
    
    directory_entry_t *dirent = (directory_entry_t *) (&entries[size-1]);
    dirent_t *dirent_p = &static_dirent; // TODO malloc!
    dirent_p->size = size;
    dirent_p->type = dirent->attributes.dir ? DIR : FILE;
    dirent_p->cluster = get_cluster(dirent);
    get_short_name(dirent, test);
    strCopy(test, dirent_p->name);
    print_short_dirent(dirent_p);
    
    // Allocates place for these entries
    u32 cluster = file_table[fd].start_cluster;
    new_entry(cluster, dirent_p);
    u32 offset = dirent_p->ent_offset;
    cluster = dirent_p->ent_cluster;
    
    kprintf("Allocated %d entries at cluster %d and offset %d\n", size, cluster, offset);
    // Fills in these entries on the disk
    fill_entries(cluster, offset, size, entries);
}

void rmdir(char *path) {
    char name[MAX_PATH_NAME];
    strCopy(path, name);
    fd_t dir = opendir(name);
    dirent_t *dirent;
    while ((dirent = readdir(dir))) {
        if (!(strEqual(dirent->name, CUR_DIR_NAME) || strEqual(dirent->name, PARENT_DIR_NAME))) {
            kprintf("Can't remove directory %s : directory isn't empty.\n", path);
            return;
        }
    }
    
    concat(name, PARENT_DIR_NAME);
    kprintf("Parent : %s\n", name);
    fd_t parent = opendir(name); // Opens the parent directory
    kprintf("Parent cluster %d \n", file_table[parent].start_cluster);

    // Finds the directory entry corresponding to fd, and removes it.
    dirent = cluster_finddir(parent, file_table[dir].start_cluster);
    assert(dirent != NULL);
    
    print_short_dirent(dirent);
    free_entry(dirent);
    kprintf("Hello_n");
    closedir(dir);
    closedir(parent);
    kprintf("Closed directories \n");
    // Removes the content of directory.
    u32 cluster = file_table[dir].start_cluster;
    u32 next_cluster;
    while (cluster < END_OF_CHAIN) {
        kprintf("Cluster %d\n", cluster);
        next_cluster = get_next_cluster(cluster);
        free_cluster(cluster, 0);
        cluster = next_cluster;
    }
}

void chdir(char *path) {
    fd_t fd = opendir(path);
    closedir(cwd);
    cwd = fd;
}

char *getcwd() {
    // Returns the path from root to current working directory cwd.
    char path[MAX_PATH_NAME];
    //char res[MAX_PATH_NAME]; // TODO malloc
    char *res = static_path_start;
    u32 res_index = MAX_PATH_NAME - 2;
    res[MAX_PATH_NAME - 1] = 0;
    
    // Disjunction between root and non root directories
    u32 cl = file_table[cwd].start_cluster;
    strCopy((cl == fs.root_cluster) ? "" : CUR_DIR_NAME, path);
    fd_t fd = opendir(path);

    fd_t parent;
    dirent_t *dirent;
    while (file_table[fd].start_cluster > fs.root_cluster) {
        concat(path, PARENT_DIR_NAME);
        parent = opendir(path); // Opens the parent directory
        
        // Finds the directory entry corresponding to fd.
        dirent = cluster_finddir(parent, file_table[fd].start_cluster);
        closedir(fd); // Closes fd
        
        fd = parent;
        u32 length = strlen(dirent->name);
        assert(res_index >= length + 1);
        strCopy(dirent->name, res + res_index - length);
        res[res_index] = DIR_SEP;
        res_index -= length + 1;
    }
    
    // fd corresponds to the root.
    res[res_index] = ROOT_NAME;
    
    //TODO Temporary
    static_path_offset = res_index;
    return res + res_index;
}

fd_t opendir(char *path) {
    fd_t fd;
    dirent_t *dirent_p;
    
    // Initialisation : open the source directory.
    if (*path == DIR_SEP) {
        dirent_t dirent;
        dirent.cluster = fs.root_cluster;
        dirent.name[0] = '/';
        dirent.name[1] = 0;
        dirent.type = DIR;
        
        fd = opendir_ent(&dirent); // Opens root directory.
        for (; *path == DIR_SEP; path++) {} // Removes all starting separators
    }
    else {
        // Starts from current directory
        if (file_table[cwd].start_cluster == fs.root_cluster) {
            // Starts from root
            dirent_t dirent;
            dirent.cluster = fs.root_cluster;
            dirent.name[0] = '/';
            dirent.name[1] = 0;
            dirent.type = DIR;  
        
            fd = opendir_ent(&dirent); // Opens root directory.
        }
        else {
            fd = opendir_ent(finddir(cwd, CUR_DIR_NAME));
        }
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
        if (fd != cwd)
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
    file_table[fd].prev_cluster = 0;
    
    return fd;
}

dirent_t *readdir(fd_t fd) {
    // Reads the next entry of the directory.
    // If there is no such entry, returns NULL.
    assert(file_table[fd].type == DIR);
    
    u32 cluster = file_table[fd].curr_cluster;
    u32 prev_cluster = file_table[fd].prev_cluster;
    u8 buffer[fs.cluster_size];
    memset(buffer, 0, fs.cluster_size);
    u32 next_cluster = cluster;
    
    // Create a buffer to store the name.
    char name[MAX_FILE_NAME];
    char *name_index;
    name_index = name + MAX_FILE_NAME - 1;
    u32 size = 0;
    u32 start_offset = file_table[fd].curr_offset;

    do {
        prev_cluster = cluster;
        cluster = next_cluster;
        read_cluster(cluster, buffer);
        for (u32 i = start_offset; i < fs.cluster_size; i += 32) {
            if (buffer[i] == 0) {
                // No more directories or files.
                file_table[fd].curr_cluster = cluster;
                file_table[fd].curr_offset = i;
                file_table[fd].prev_cluster = prev_cluster;
                return NULL;
            }
            if (buffer[i] == 0xE5) {
                // Unused entry
                continue;
            }
            if (buffer[i + 11] == 0x0F) {
                // Long name entry
                long_file_name_t *lfn = (long_file_name_t *) (buffer+i);
                get_long_name(lfn, name_index - 13);
                name_index -= 13;
                size ++;
                continue;
            }            
            else {
                // This is a directory entry.
                directory_entry_t *dirent = (directory_entry_t *) (buffer+i);
                
                // Removes ending spaces
                char * name_end = name + MAX_FILE_NAME - 1;
                for (; *name_end == ' '; name_end --) {} 
                *(name_end + 1) = 0;
                
                size ++;
                //dirent_t *dirent_p = kmalloc(sizeof(dirent_t)); // TODO !
                dirent_t *dirent_p = &static_dirent; // TODO !
                dirent_p->type = dirent->attributes.dir ? DIR : FILE;
                dirent_p->cluster = get_cluster(dirent);
                dirent_p->ent_offset = i - 32 * (size - 1);
                dirent_p->ent_cluster = cluster;
                dirent_p->size = size;
                dirent_p->ent_prev_cluster = prev_cluster;
                
                if (name + MAX_FILE_NAME - 1 == name_index) {
                    // Not a long filename
                    get_short_name(dirent, name);
                    name_index = name;
                }
                strCopy(name_index, dirent_p->name);
                
                file_table[fd].prev_cluster = prev_cluster;
                file_table[fd].curr_cluster = cluster;
                file_table[fd].curr_offset = i + 32;
                
                return dirent_p;
            }
        }
        next_cluster = get_next_cluster(cluster);
        start_offset = 0;
        
    } while (next_cluster < 0x0FFFFFF8 && next_cluster != UNUSED_CLUSTER);
    
    // End of directory.
    file_table[fd].curr_cluster = cluster;
    file_table[fd].curr_offset = fs.cluster_size;
    file_table[fd].prev_cluster = prev_cluster;
    
    return NULL;
}

void rewinddir(fd_t fd) {
    file_table[fd].curr_cluster = file_table[fd].start_cluster;
    file_table[fd].curr_offset = 0;
    file_table[fd].prev_cluster = 0;
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
            // Rewinds to avoid side effects.
            rewinddir(dir);
            if (dirent->type == DIR)
                return dirent;
            //assert(0);
            return NULL;    // This is not a directory.
        }
    }
    // Rewinds to avoid side effects.
    rewinddir(dir);
    //assert(0);
    return NULL;    // Entry doesn't exist.
}

dirent_t *cluster_finddir(fd_t dir, u32 cluster) {
    // Find the directory entry corresponding to directory name.
    dirent_t *dirent;
    while ((dirent = readdir(dir))) {
        if (dirent->cluster == cluster) {
            // Rewinds to avoid side effects.
            rewinddir(dir);
            if (dirent->type == DIR)
                return dirent;
            //assert(0);
            return NULL;    // This is not a directory.
        }
    }
    // Rewinds to avoid side effects.
    rewinddir(dir);
    //assert(0);
    return NULL;    // Entry doesn't exist.
}

void print_chain(u32 cluster) {
    kprintf("Cluster chain : ");
    while (cluster < END_OF_CHAIN) {
        kprintf("%d -> ", cluster);
        cluster = get_next_cluster(cluster);
    }
    kprintf("END\n");
}

void test_dir() {
    cwd = opendir(ROOT_NAME_STR);
    kprintf("Testing Directory Calls :\n");
    kprintf("Content of root directory : \n");
    fd_t root = opendir("/");
    dirent_t *dirent;
    while ((dirent = readdir(root))) {
        print_short_dirent(dirent);
    }
    closedir(root);
    
    kprintf("\nContent of boot directory : \n");
    fd_t boot = opendir("/boot");
    kprintf("Cluster %d\n", file_table[boot].start_cluster);
    while ((dirent = readdir(boot))) {
        print_short_dirent(dirent);
    }
    closedir(boot);
    
    kprintf("\nContent of grub directory : \n");
    fd_t grub = opendir("boot//grub/");
    kprintf("Cluster %d\n", file_table[grub].start_cluster);
    while ((dirent = readdir(grub))) {
        print_short_dirent(dirent);
    }
    closedir(grub);
    
    kprintf("\nTest of getcwd at root directory : %s\n", getcwd()); 
    
    kprintf("Test de chdir dans fonts/ :\n");
    chdir("/boot/grub/fonts/");
    fd_t fonts = opendir("");
    while ((dirent = readdir(fonts))) {
        print_short_dirent(dirent);
    }
    kprintf("getcwd() : %s \n", getcwd());
    //for (int i = 0; i < 5; i++)
    //    rmdir("test1");

    //kprintf("Cluster %d\n", file_table[fd].start_cluster);
    print_chain(4506);
}

void init_filename_gen() {
    // Saves in a file the next 8.3 filename.
    // TODO Need files 
}

void init_root() {
    // Adds . and .. entries to the root directory.
    
    fd_t fd = opendir("/");
    void* res = finddir(fd, CUR_DIR_NAME);
    closedir(fd);
    if (res != NULL)  {
        // These entries already exists
        return;
    }
    
    u32 cluster = fs.root_cluster;
    directory_entry_t buffer[2];
    fill_dir_entry(cluster, buffer, CUR_DIR_NAME);
    fill_dir_entry(cluster, &buffer[1], PARENT_DIR_NAME);
    
    dirent_t dirent;
    dirent.cluster = cluster;
    dirent.size = 2;
   
    new_entry(cluster, &dirent);
    kprintf("Cluster %d offset %d size %d\n", dirent.ent_cluster, dirent.ent_offset, dirent.size );
    fill_entries(dirent.ent_cluster, dirent.ent_offset, dirent.size, buffer);
    
}