#include "fs_call.h"
dirent_t static_dirent; // Temporary
char static_path_start[MAX_PATH_NAME]; // Temporary
u32 static_path_offset = 0;
long_file_name_t static_longnames[MAX_FILE_NAME / 13 + 2];
directory_entry_t static_entry;
fd_t cwd; // Current working directory

// TODO
int usermod = 0; // Global variable with rights of current processs

void print_short_dirent(dirent_t *dirent) {
    kprintf("%s %s at offset %d, %d entries;\n", 
            (dirent->type == DIR) ? "Directory" : "File",
            dirent->name,
            dirent->ent_offset,
            dirent->ent_size);
}

fd_t openfile_ent(dirent_t *dirent, oflags_t flags) {
    // Checks it isn't a directory
    if (dirent->type != FILE) {
        errno = 4;
        return -1;
    }
    
    // Checks the rights
    if (dirent->attributes.rd_only && flags.write) {
        // No right to write.
        errno = 4; 
        return -1;
    }
    if (dirent->attributes.system && usermod && flags.write) {
        // No rights to modify.
        errno = 4;
        return -1;
    }
    // TODO Decide whether TRUNC and no WRITE must be an error ?
    
    fd_t fd = new_fd();
    if (fd < 0) // TODO Too much opened files, is errno already set ?
        return -1;
    
    strCopy(dirent->name, file_table[fd].name);
    
    file_table[fd].type = FILE;
    file_table[fd].global_offset = 0;
    file_table[fd].old_offset = 0;
    file_table[fd].curr_offset = 0;
    file_table[fd].start_cluster = dirent->cluster;
    file_table[fd].curr_cluster = dirent->cluster;
    file_table[fd].ent_cluster = dirent->ent_cluster;
    file_table[fd].ent_offset = dirent->ent_offset;
    file_table[fd].ent_size = dirent->ent_size;
    file_table[fd].mode = flags;
    file_table[fd].size = dirent->size;
    
    if (flags.trunc && flags.write) {
        // Truncates the file
        u32 cluster = file_table[fd].start_cluster;
        set_size(fd, 0);
        free_cluster_chain(get_next_cluster(cluster));
        reset_cluster(cluster);
        set_next_cluster(cluster, END_OF_CHAIN);
    }
    return fd;
}

fd_t openfile(char *path, oflags_t flags) {
    char *dir_n = dirname(path);
    char *name = basename(path);
    
    // TODO malloc and this will be normally clean
    char file_name[MAX_FILE_NAME];
    strCopy(name, file_name);
    
    fd_t dir = opendir(dir_n);
    if (dir < 0) {
        errno = 4; // No such directory
        return -1;
    }
    
    dirent_t *file = findfile(dir, file_name);
    
    if (file == NULL) {
        // No such file.
        if (errno == 4) {
            // It is a directory ! TODO
            errno = 4;
            return -1;
        }
        else if (flags.create) {
            // Creates the file TODO
            int res = create_entries(dir, file_name, FILE);
            if (res < 0)
                return -1;
            // TODO Checks results
            file = findfile(dir, file_name);
            if (file == NULL) {
                kprintf("Failed to find this file, even after creation.\n");
                errno = 4;
                return -1;
            }
        }
        else {
            // No such file, no O_CREATE, fails. TODO
            errno = 4;
            return -1;
        }
    }
    return openfile_ent(file, flags);
}

int close(fd_t fd) {
    if (file_table[fd].type != FILE) {
        // No such file
        kprintf("Are you sure you're allowed to close directories with close ?\n");
        errno = 4;
        return -1;
    }      
    free_fd(fd);
    return 0;
}

int really_seek(fd_t fd, int write) {
    // Updates position of cursor
    // If the new position is outside the file and write is True, extends the
    // file. Otherwise, if write is false, does nothing.
    ft_entry_t *f = &file_table[fd];
    if (f->global_offset == f->old_offset ||
       (f->global_offset >= f->size && !write)) {
        return 0;
    }
    kprintf("Starting positions : cluster %d at offset %d, old_offset %d\n",
            f->curr_cluster, f->curr_offset, f->old_offset);
    u32 cluster, offset, target_offset;
    // We have to move towards the right location, and extend if needed.
    target_offset = f->global_offset;
    if (f->global_offset > f->old_offset) {
        cluster = f->curr_cluster;
        offset = f->old_offset - f->curr_offset;
        kprintf("Are the offsets equal ? %d\n", f->curr_offset == f->old_offset % fs.cluster_size);
    }
    else {
        cluster = f->start_cluster;
        offset = 0;
    }
    u32 prev_cluster;
    while (target_offset - offset >= fs.cluster_size) {
        prev_cluster = cluster;
        cluster = get_next_cluster(cluster);
        if (cluster >= END_OF_CHAIN) {
            // End of file
            if (write) {
                // Extends file
                cluster = insert_new_cluster(prev_cluster, END_OF_CHAIN);
                assert(cluster);
            }
            else {
                // Corrupted file (size doesn't correspond.)
                errno = 4;
                return -1;
            }
        }
        offset += fs.cluster_size;
    }
    f->old_offset = target_offset;
    f->curr_cluster = cluster;
    f->curr_offset = target_offset - offset;
    set_size(fd, umax(f->size, target_offset));
    
    kprintf("Ending positions : cluster %d at offset %d, old_offset %d\n",
            f->curr_cluster, f->curr_offset, f->old_offset);
    return 0;
}

int read(fd_t fd, void *buffer, int length) {
    ft_entry_t *f = &file_table[fd];
    assert(length >= 0);
    // TODO u32 length ?
    // Checks the mode allows reading.
    if (!f->mode.read) {
        // No read mode
        kprintf("No read mode allowed !\n");
        errno = 4;
        return -1;
    }
    // Places correctly the reading head.
    if (really_seek(fd, 0) == -1) {
        kprintf("Failed to seek !\n");
        return -1;
    }
    // Checks if offset is past the end of file.
    if (f->global_offset >= f->size)
        return 0;
    kprintf("Is the global_offset equal to old_offset ? %d\n", f->global_offset == f->old_offset);
    // Otherwise reads the asked length.
    u32 done = 0;
    u32 remaining = umin(length, f->size - f->global_offset);
    u8 content[fs.cluster_size];
    u32 cluster = f->curr_cluster;
    u32 offset = f->curr_offset;
    u32 nb;
    kprintf("Trying to read %d bytes from offset %d at cluster %d in size %d\n",
            remaining, offset, cluster, f->size);
    while (remaining > 0) {
        kprintf("Cluster %d\n", cluster);
        assert(cluster < END_OF_CHAIN);
        read_cluster(cluster, content); // TODO assert fine
        nb = umin(remaining, fs.cluster_size - offset);
        kprintf("Have just read %d bytes from %d to %d\n", nb, offset, offset+nb);
        memcpy(buffer, content + offset, nb);
        remaining -= nb;
        done += nb;
        buffer += nb;
        offset = offset + nb;
        if (offset + nb == fs.cluster_size) {
            cluster = get_next_cluster(cluster);
            offset = 0;
        }
    }
    kprintf("Finished at offset %d in cluster %d\n", offset, cluster);
    // TODO Check that arriving in END_OF_CHAIN cluster isn't important.
    f->curr_cluster = cluster;
    f->curr_offset = offset;
    f->global_offset += done;
    f->old_offset += done;
    return done;
}

void set_size(fd_t fd, u32 size) {
    // Sets the size of file fd.
    u8 content[fs.cluster_size];
    u32 cluster = file_table[fd].ent_cluster;
    u32 offset = file_table[fd].ent_offset + 32 * (file_table[fd].ent_size - 1);
    read_cluster(cluster, content);
    directory_entry_t *dirent = (directory_entry_t *) &content[offset];
    dirent->file_size = size;
    write_cluster(cluster, content);
    file_table[fd].size = size;
}

// TODO Think about updating curr_offset and curr_cluster ! and read of 0 holes
int write(fd_t fd, void *buffer, int length) {
    assert(length >= 0);
    
    ft_entry_t *f = &file_table[fd];
    // Checks the mode allows writing.
    if (!f->mode.write) {
        // No writing mode
        errno = 4;
        return -1;
    }
    kprintf("Starting offset %d\n", f->old_offset);

    // Places correctly writing head.
    // In case of O_APPEND, places the head at enf of file.
    if (file_table[fd].mode.append) {
        kprintf("Append mode, old_offset is %d\n", file_table[fd].old_offset);
        file_table[fd].global_offset = file_table[fd].size;
    }
    if (really_seek(fd, 1) == -1)
        return -1;
    
    // Otherwise writes the asked length.
    u32 written = 0;
    u32 remaining = length;
    u8 content[fs.cluster_size];
    u32 cluster = f->curr_cluster;
    u32 prev_cluster = 0;
    u32 offset = f->curr_offset;
    u32 nb;
    kprintf("Trying to write %d bytes from offset %d at cluster %d in size %d\n",
            remaining, offset, cluster, f->size);
 
    while (remaining > 0) {
        kprintf("Cluster %d\n", cluster);
        if (cluster >= END_OF_CHAIN) {
            cluster = insert_new_cluster(prev_cluster, END_OF_CHAIN);
            assert(cluster);
        }
        nb = umin(remaining, fs.cluster_size - offset);

        if (nb < fs.cluster_size)
            read_cluster(cluster, content); // TODO assert fine
        
        memcpy(content + offset, buffer, nb);
        write_cluster(cluster, content);
        kprintf("Have just written %d bytes\n", nb);
        remaining -= nb;
        written += nb;
        buffer += nb;
        offset += nb;
        if (offset == fs.cluster_size) {
            offset = 0;
            prev_cluster = cluster;
            cluster = get_next_cluster(cluster);
        }
    }
    kprintf("Finished writing at offset %d in cluster %d\n", offset, cluster);
    // TODO Check that arriving in END_OF_CHAIN cluster isn't important.
    f->curr_cluster = cluster;
    f->curr_offset = offset;
    f->global_offset += written;
    f->old_offset += written;
    set_size(fd, umax(f->size, f->global_offset));
    kprintf("Final offset %d\n", f->curr_offset);
    return written;
}

int remove(char *path) {
    // Removes the specified file.
    char *dir_n = dirname(path);
    char *name = basename(path);
    
    // TODO malloc and this will be normally clean
    char file_name[MAX_FILE_NAME];
    strCopy(name, file_name);
    
    fd_t dir = opendir(dir_n);
    
    if (dir < 0) {
        // No such file.
        errno = 4;
        return -1;
    }
    dirent_t *dirent = findent(dir, file_name, FILE);
    if (dirent == NULL) {
        if (errno == 4) // Not a file !
            errno = 8;
        else
            errno = 27; // No such file
        return -1;
    }
    if (dirent->attributes.system && usermod) {
        // No such permission
        errno = 4;
        return -1;
    }
    // Deletes the entry
    free_entry(dirent);
    closedir(dir);
    // Deletes the content of file
    free_cluster_chain(dirent->cluster);
    return 0;
}

int seek(fd_t fd, seek_cmd_t seek_command, int offset) {
    if (seek_command == SEEK_CUR)
        offset += file_table[fd].global_offset;
    if (seek_command == SEEK_END)
        offset += file_table[fd].size;
    
    if (offset < 0) {
        // Wrong offset
        errno = 4;
        return -1;
    }
    
    file_table[fd].global_offset = offset;
    return offset;
}

int copyfile(char *old_path, char *new_path) {
    // Copies the first file toward second place.
    oflags_t flags;
    flags.read = 1;
    flags.write = 0;
    
    fd_t a = openfile(old_path, flags);
    if (a < 0) {
        // Errno may be set
        return -1;
    }
    
    flags.write = 1;
    flags.read = 0;
    flags.create = 1;
    flags.trunc = 1; // Erases the target ? TODO
    // TODO Check that both files have same parameters (system, ..)
    fd_t b = openfile(new_path, flags);
    if (b < 0) {
        // Errno may be set
        return -1;
    }
    
    // We just have to copy the content of a in b.
    u8 buffer[fs.cluster_size];
    
    int nb = read(a, buffer, fs.cluster_size);
    while (nb > 0) {
        write(b, buffer, nb);
        nb = read(a, buffer, fs.cluster_size);
    }
    
    return 0;
}

int rename(char *old_path, char *new_path) {
    int res = copyfile(old_path, new_path);
    if (res == -1) {
        // errno may be set
        return -1;
    }
    res = remove(old_path);
    if (res == -1) {
        // Big problem : copied but not removed other !
        // Should check at start the rights and so on to be sure !
        return -1;
    }
    return 0;
}

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

void fill_dir_entry(u32 cluster, directory_entry_t *dirent, char *name, ftype_t type) {
    dirent->attributes.dir = type == DIR ? 1 : 0;
    dirent->attributes.rd_only = 0; // TODO Find a way to set it if wanted
    dirent->attributes.system = 0; // TODO Find a way to set it if wanted
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

void *build_entries(u32 parent_cluster, char *name, ftype_t type) {
    // Builds the several long name entries to put in the father directory for 
    // the new file or directory.
    // Also builds the final entry.
    
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
    
    u8 content[fs.cluster_size];
    memset(content, 0, fs.cluster_size);
    if (type == DIR) {
        // Fills in the base links (parent and current directory) in case of dir.
        fill_dir_entry(fresh_cluster, (directory_entry_t *) content, CUR_DIR_NAME, DIR);
        fill_dir_entry(parent_cluster, (directory_entry_t *) (&content[32]), PARENT_DIR_NAME, DIR);
        kprintf("Parent cluster %d\n", parent_cluster);
        kprintf("Fresh cluster %d\n", fresh_cluster);
    }
    write_cluster(fresh_cluster, content);

    // Creates final entry
    directory_entry_t *dirent = (directory_entry_t *) &(static_longnames[nb]);
    // TODO Add time things..
    fill_dir_entry(fresh_cluster, dirent, fresh_name, type);

    return static_longnames;
}

int create_entries(fd_t dir, char *name, ftype_t type) {
    char file[MAX_FILE_NAME];
    assert(strlen(name) <= MAX_FILE_NAME);
    strCopy(name, file);
    
    // Build corresponding entries
    long_file_name_t *entries = build_entries(file_table[dir].start_cluster, file, type);

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
    dirent_p->ent_size = size;
    dirent_p->type = dirent->attributes.dir ? DIR : FILE;
    dirent_p->cluster = get_cluster(dirent);
    //get_short_name(dirent, test);
    strCopy(test, dirent_p->name);
    print_short_dirent(dirent_p);
    
    // Allocates place for these entries
    u32 cluster = file_table[dir].start_cluster;
    new_entry(cluster, dirent_p);
    u32 offset = dirent_p->ent_offset;
    cluster = dirent_p->ent_cluster;
    
    kprintf("Allocated %d entries at cluster %d and offset %d\n", size, cluster, offset);
    // Fills in these entries on the disk
    fill_entries(cluster, offset, size, entries);
    return 0;
}

void mkdir(char *path) {
    char *dir_name = dirname(path);
    char *file_name = basename(path);
    
    assert(strlen(file_name) < MAX_FILE_NAME);
    char file[MAX_FILE_NAME];
    strCopy(file_name, file);

    fd_t fd = opendir(dir_name);
    assert(fd >= 0); // No such directory
    
    // Asserts directory doesn't exist
    assert(finddir(fd, file) == NULL);
    
    create_entries(fd, file, DIR);
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
    free_cluster_chain(dirent->cluster);
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
    file_table[fd].ent_cluster = dirent->ent_cluster;
    file_table[fd].ent_offset = dirent->ent_offset;
    file_table[fd].ent_size = dirent->ent_size;
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
                dirent_p->ent_size = size;
                dirent_p->ent_prev_cluster = prev_cluster;
                dirent_p->attributes = dirent->attributes;
                dirent_p->size = dirent->file_size;
                
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

dirent_t *findent(fd_t dir, char *name, ftype_t type) {
    // Find the entry with specified name and type in directory dir.
    // Returns NULL if no such entry.
    dirent_t *dirent;
    while ((dirent = readdir(dir))) {
        if (strEqual(dirent->name, name)) {
            // Rewinds to avoid side effects.
            rewinddir(dir);
            if (dirent->type == type)
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

dirent_t *finddir(fd_t dir, char *name) {
    return findent(dir, name, DIR);
}

dirent_t *findfile(fd_t dir, char *name) {
    return findent(dir, name, FILE);
}

dirent_t *cluster_findent(fd_t dir, u32 cluster, ftype_t type) {
    // Find the directory entry corresponding to directory name.
    dirent_t *dirent;
    while ((dirent = readdir(dir))) {
        if (dirent->cluster == cluster) {
            // Rewinds to avoid side effects.
            rewinddir(dir);
            if (dirent->type == type)
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
    return cluster_findent(dir, cluster, DIR);
}

dirent_t *cluster_findfile(fd_t dir, u32 cluster) {
    return cluster_findent(dir, cluster, FILE);
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
    oflags_t flags;
    flags.append = 1;
    flags.create = 1;
    flags.read = 1;
    flags.write = 1;
    //flags.trunc = 1;
    fd_t file = openfile("testfile", flags);
    char buffer[500];
    strCopy("Hello world ! I'm not happy", buffer);
    kprintf("Wrote %d bytes\n", write(file, buffer, 511));
    //file_table[file].size = 200;
    memset(buffer, 0, 500);
    seek(file, SEEK_SET, 0);
    kprintf("Read : %d\n", read(file, buffer, 201));
    kprintf("Content : %s\n", buffer);
    copyfile("testfile", "testfile_copy");
    remove("testfile");
    rewinddir(fonts);
    while ((dirent = readdir(fonts))) {
        print_short_dirent(dirent);
    }
    kprintf("Read : %d\n", read(openfile("testfile_copy", flags), buffer, 201));
    kprintf("COntent :%s\n", buffer);
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
    fill_dir_entry(cluster, buffer, CUR_DIR_NAME, DIR);
    fill_dir_entry(cluster, &buffer[1], PARENT_DIR_NAME, DIR);
    
    dirent_t dirent;
    dirent.cluster = cluster;
    dirent.size = 2;
   
    new_entry(cluster, &dirent);
    kprintf("Cluster %d offset %d size %d\n", dirent.ent_cluster, dirent.ent_offset, dirent.ent_size );
    fill_entries(dirent.ent_cluster, dirent.ent_offset, dirent.ent_size, buffer);
    
}