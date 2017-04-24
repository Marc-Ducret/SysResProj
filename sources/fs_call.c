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
    if (dirent->attributes.rd_only && (flags && O_WRONLY)) {
        // No right to write.
        errno = 4; 
        return -1;
    }
    if (dirent->attributes.system && usermod && (flags & O_WRONLY)) {
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
    file_table[fd].flags = flags;
    file_table[fd].size = dirent->size;
    file_table[fd].mode = dirent->mode;
    
    if ((flags & O_TRUNC) && (flags & O_WRONLY)) {
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
        else if ((flags & O_CREAT)) {
            // Creates the file TODO
            int res = create_entries(dir, file_name, FILE, flags & O_CMODE);
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
    // If the new position is outside the file : if write is True, extends the
    // file. Otherwise, if write is false, does nothing.
    ft_entry_t *f = &file_table[fd];
    if (f->curr_cluster >= END_OF_CHAIN) {
        // If current true position is outside the file. 
        f->curr_cluster = f->start_cluster;
        f->curr_offset = 0;
        f->old_offset = 0;
    }
    if (f->global_offset == f->old_offset ||
       (f->global_offset >= f->size && !write)) {
        return 0;
    }
    //fprintf(stderr, "Starting positions : cluster %d at offset %d, old_offset %d\n",
    //        f->curr_cluster, f->curr_offset, f->old_offset);
    u32 cluster, offset, target_offset;
    // We have to move towards the right location, and extend if needed.
    target_offset = f->global_offset;
    if (f->global_offset > f->old_offset) {
        cluster = f->curr_cluster;
        offset = f->old_offset - f->curr_offset;
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
                if (cluster == 0)
                    return -1; // No space left.
            }
            else {
                // Corrupted file (size doesn't correspond.)
                errno = ECORRF;
                return -1;
            }
        }
        offset += fs.cluster_size;
    }
    f->old_offset = target_offset;
    f->curr_cluster = cluster;
    f->curr_offset = target_offset - offset;
    set_size(fd, umax(f->size, target_offset));
    
    //fprintf(stderr, "Ending positions : cluster %d at offset %d, old_offset %d\n",
    //        f->curr_cluster, f->curr_offset, f->old_offset);
    return 0;
}

ssize_t read(fd_t fd, void *buffer, size_t length) {
    ft_entry_t *f = &file_table[fd];

    // Checks the mode allows reading.
    if (f->type == F_UNUSED || !(f->flags & O_RDONLY)) {
        // Invalid file descriptor, or no read allowed
        errno = EBADF;
        return -1;
    }
    if (f->type == DIR) {
        errno = EISDIR;
        return -1;
    }
    // Places correctly the reading head.
    if (really_seek(fd, 0) == -1) {
        fprintf(stderr, "Seek failed before reading.\n");
        return -1;
    }
    // Checks if offset is past the end of file.
    if (f->global_offset >= f->size)
        return 0;
    fprintf(stderr, "Is the global_offset equal to old_offset ? %d\n",
            f->global_offset == f->old_offset);
    // Otherwise reads the asked length.
    size_t done = 0;
    size_t remaining = umin(length, f->size - f->global_offset);
    u8 content[fs.cluster_size];
    u32 cluster = f->curr_cluster;
    u32 prev_cluster = END_OF_CHAIN;
    u32 offset = f->curr_offset;
    size_t nb;
    fprintf(stderr, "Trying to read %d bytes from offset %d at cluster %d in size %d\n",
            remaining, offset, cluster, f->size);
    while (remaining > 0) {
        fprintf(stderr, "Cluster %d\n", cluster);
        if (cluster >= END_OF_CHAIN) {
            errno = ECORRF;
            return -1;
        }
        read_cluster(cluster, content);
        nb = umin(remaining, fs.cluster_size - offset);
        fprintf(stderr, "Have just read %d bytes from %d to %d\n", nb, offset, offset+nb);
        memcpy(buffer, content + offset, nb);
        remaining -= nb;
        done += nb;
        buffer += nb;
        offset = offset + nb;
        if (offset == fs.cluster_size) {
            prev_cluster = cluster;
            cluster = get_next_cluster(cluster);
            offset = 0;
        }
    }
    fprintf(stderr, "Finished at offset %d in cluster %d\n", offset, cluster);
    f->curr_cluster = cluster;
    f->curr_offset = offset;
    f->global_offset += done;
    f->old_offset += done;
    if (f->curr_cluster >= END_OF_CHAIN) {
        // That means that we read until end of file.
        // We set the position back in the last cluster to avoid to come back at
        // start of it.
        f->curr_cluster = prev_cluster;
        f->curr_offset = 0;
        f->old_offset -= fs.cluster_size;
    }
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
ssize_t write(fd_t fd, void *buffer, size_t length) {
    // Tries to write lenght bytes from buffer to file fd.
    // Returns the number of bytes written, or -1 in case of failure.
    ft_entry_t *f = &file_table[fd];
    // Checks the mode allows writing.
    if (f->type == F_UNUSED || !(f->flags & O_WRONLY)) {
        // Invalid file descriptor, or no write allowed
        errno = EBADF;
        return -1;
    }
    if (f->type == DIR) {
        errno = EISDIR;
        return -1;
    }
    //fprintf(stderr, "Starting offset %d\n", f->old_offset);

    // Places correctly writing head.
    // In case of O_APPEND, places the head at enf of file.
    if (file_table[fd].flags & O_APPEND) {
        //fprintf(stderr, "Append mode, old_offset is %d\n", file_table[fd].old_offset);
        file_table[fd].global_offset = file_table[fd].size;
    }
    if (really_seek(fd, 1) == -1)
        return -1;
    
    // Otherwise writes the asked length.
    size_t written = 0;
    size_t remaining = length;
    u8 content[fs.cluster_size];
    u32 cluster = f->curr_cluster;
    u32 prev_cluster = 0;
    u32 offset = f->curr_offset;
    size_t nb;
    //fprintf(stderr, "Trying to write %d bytes from offset %d at cluster %d in size %d\n",
    //        remaining, offset, cluster, f->size);
 
    while (remaining > 0) {
        //fprintf(stderr, "Cluster %d\n", cluster);
        if (cluster >= END_OF_CHAIN) {
            cluster = insert_new_cluster(prev_cluster, END_OF_CHAIN);
            if (cluster == 0) {
                // No more space available
                cluster = END_OF_CHAIN;
                if (written == 0)
                    return -1; 
                else
                    break; // Already wrote some data, so succeed.
            }
        }
        nb = umin(remaining, fs.cluster_size - offset);

        if (nb < fs.cluster_size)
            read_cluster(cluster, content);
        
        memcpy(content + offset, buffer, nb);
        write_cluster(cluster, content);
        //fprintf(stderr, "Have just written %d bytes\n", nb);
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
    //fprintf(stderr, "Finished writing at offset %d in cluster %d\n", offset, cluster);
    // TODO Check that arriving in END_OF_CHAIN cluster isn't important.
    f->curr_cluster = cluster;
    f->curr_offset = offset;
    f->global_offset += written;
    f->old_offset += written;
    set_size(fd, umax(f->size, f->global_offset));
    //fprintf(stderr, "Final offset %d\n", f->curr_offset);
    if (f->curr_cluster >= END_OF_CHAIN) {
        // That means that we wrote until end of file.
        // We set the position back in the last cluster to avoid to come back at
        // start of it.
        f->curr_cluster = prev_cluster;
        f->curr_offset = 0;
        f->old_offset -= fs.cluster_size;
    }
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
    oflags_t flags = O_RDONLY;
    
    fd_t a = openfile(old_path, flags);
    if (a < 0) {
        // Errno may be set
        return -1;
    }
    
    flags = O_WRONLY | O_CREAT | O_TRUNC; // Erases the target ? TODO
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
        offset -= fs.cluster_size;
    }
    u8 content[fs.cluster_size];
    read_cluster(cluster, content);
    memcpy(content + offset, entries, size * 32);
    write_cluster(cluster, content);
}

void fill_dir_entry(u32 cluster, directory_entry_t *dirent, char *name, ftype_t type, u8 mode) {
    dirent->attributes.dir = type == DIR ? 1 : 0;
    dirent->attributes.rd_only = mode & RDONLY;
    dirent->attributes.system = mode & SYSTEM;
    dirent->attributes.hidden = 0;
    dirent->attributes.archive = 1; // Dirty ?
    dirent->cluster_high = cluster >> 16;
    dirent->cluster_low = cluster & 0xFFFF;
    u32 size = strlen(name);
    if (size > 11)
        name[11] = 0; // TODO check
    strCopy(name, dirent->file_name);
    
    for (u32 i = size; i < 11; i++)
        dirent->file_name[i] = ' '; // Pads with spaces
    dirent->file_size = 0;
}

char static_fresh_name[12]; // TODO
char *get_fresh_name() {
    static int count = 0;
    if (count >= 10000000) // May not happen
        count = 0;
    char *buffer = static_fresh_name;
    *buffer = 'f';
    write_int(buffer+1, count);
    count++;
    return buffer;
}

void *build_entries(fd_t parent_dir, char *name, ftype_t type, u8 mode) {
    // Builds the several long name entries to put in the father directory for 
    // the new file or directory.
    // Also builds the final entry.
    
    char buffer[MAX_FILE_NAME];
    memset(buffer, 0, MAX_FILE_NAME);
    strCopy(name, buffer);
    int len = strlen(buffer);
    u32 nb = len / 13 + ((len % 13) ? 1 : 0);
    fprintf(stderr, "Nb of lfn : %d\n", nb);
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
        fprintf(stderr, "String %s\n", buf);
    }
    
    // Changes order field for last entry
    static_longnames[0].order |= 0x40;
    
    // Asks for a new cluster
    u32 fresh_cluster = new_cluster();
    if (fresh_cluster == 0)
        return NULL;
    
    u8 content[fs.cluster_size];
    memset(content, 0, fs.cluster_size);
    u32 parent_cluster = file_table[parent_dir].start_cluster;
    u8 parent_mode = file_table[parent_dir].mode;
    if (type == DIR) {
        // Fills in the base links (parent and current directory) in case of dir.
        fill_dir_entry(fresh_cluster, (directory_entry_t *) content, CUR_DIR_NAME, DIR, mode);
        fill_dir_entry(parent_cluster, (directory_entry_t *) (&content[32]), PARENT_DIR_NAME, DIR, parent_mode);
        fprintf(stderr, "Parent cluster %d\n", parent_cluster);
        fprintf(stderr, "Fresh cluster %d\n", fresh_cluster);
    }
    write_cluster(fresh_cluster, content);

    // Creates final entry
    directory_entry_t *dirent = (directory_entry_t *) &(static_longnames[nb]);
    // TODO Add time things..
    fill_dir_entry(fresh_cluster, dirent, fresh_name, type, mode);

    return static_longnames;
}

int create_entries(fd_t dir, char *name, ftype_t type, u8 mode) {
    if (strlen(name) >= MAX_FILE_NAME) {
        errno = ENAMETOOLONG;
        return -1;
    }
    char file[MAX_FILE_NAME];
    strCopy(name, file);
    
    // Build corresponding entries
    long_file_name_t *entries = build_entries(dir, file, type, mode);
    if (entries == NULL)
        return -1;
    // Computes the size of full entry
    u32 size = 0;
    while (entries[size].attribute == 0x0F)
        size ++;
    size ++;
    
    // Allocates place for these entries.
    // It uses a dirent_t struct to get back cluster and offset from new_entry()
    dirent_t dirent;
    dirent.ent_size = size;
    u32 cluster = file_table[dir].start_cluster;
    if (new_entry(cluster, &dirent) == -1) // May be no memory left.
        return -1;
    u32 offset = dirent.ent_offset;
    cluster = dirent.ent_cluster;
    
    fprintf(stderr, "Allocated %d entries at cluster %d and offset %d\n",
            size, cluster, offset);
    
    // Fills in these entries on the disk
    fill_entries(cluster, offset, size, entries);
    return 0;
}

int mkdir(char *path, u8 mode) {
    char *dir_name = dirname(path);
    char *file_name = basename(path);
    
    if (strlen(file_name) >= MAX_FILE_NAME) {
        errno = ENAMETOOLONG;
        return -1;
    }
    char file[MAX_FILE_NAME];
    strCopy(file_name, file);
    fprintf(stderr, "Creating directory %s at %s\n", file, dir_name);
    fd_t fd = opendir(dir_name);
    if (fd < 0)
        return -1;
    // Asserts directory doesn't exist
    if (finddir(fd, file) != NULL) {
        closedir(fd);
        return -1;
    }
    if (create_entries(fd, file, DIR, mode) == -1) {
        closedir(fd);
        return -1;
    }
    closedir(fd);
    return 0;
}

int rmdir(char *path) {
    if (strlen(path) >= MAX_PATH_NAME) {
        errno = ENAMETOOLONG;
        return -1;
    }
    char name[MAX_PATH_NAME];
    strCopy(path, name);
    fd_t dir = opendir(name);
    if (dir < 0)
        return -1;
    if (file_table[dir].start_cluster == file_table[cwd].start_cluster) {
        closedir(dir);
        errno = EINVAL;
        return -1;
    }
    if ((file_table[dir].mode & SYSTEM) && usermod) {
        closedir(dir);
        errno = EACCES;
        return -1;
    }
    dirent_t *dirent;
    while ((dirent = readdir(dir))) { // Check readdir result ?
        if (!(strEqual(dirent->name, CUR_DIR_NAME) || strEqual(dirent->name, PARENT_DIR_NAME))) {
            fprintf(stderr, "Can't remove directory %s : directory isn't empty.\n", path);
            closedir(dir);
            errno = ENOTEMPTY;
            return -1;
        }
    }
    u32 cluster = file_table[dir].start_cluster;
    closedir(dir);
    
    concat(name, PARENT_DIR_NAME); // Safe ?
    fprintf(stderr, "Parent : %s\n", name);
    fd_t parent = opendir(name); // Opens the parent directory
    if (parent < 0)
        return -1;
    if (file_table[parent].mode && usermod) {
        closedir(parent);
        errno = EACCES;
        return -1;
    }
    fprintf(stderr, "Parent cluster %d \n", file_table[parent].start_cluster);

    // Finds the directory entry corresponding to fd, and removes it.
    dirent = cluster_finddir(parent, cluster);
    if (dirent == NULL) {
        closedir(dir);
        closedir(parent);
        errno = ECORRF;
        return -1;
    }
    
    free_entry(dirent);
    closedir(parent);
    fprintf(stderr, "Closed directories \n");
    // Removes the content of directory.
    free_cluster_chain(cluster);
    return 0;
}

int chdir(char *path) {
    fd_t fd = opendir(path);
    closedir(cwd);
    cwd = fd;
    return 0;
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
        dirent.ent_cluster = fs.root_cluster;
        dirent.ent_offset = 64;
        dirent.ent_prev_cluster = 0;
        dirent.mode = SYSTEM;
        dirent.size = 0;
        dirent.ent_size = 1;
        dirent.attributes.dir = 1;
        dirent.attributes.system = 1;

        fd = opendir_ent(&dirent); // Opens root directory.
        for (; *path == DIR_SEP; path++) {} // Removes all starting separators
    }
    else {
        // Starts from current directory
        fd = opendir_ent(finddir(cwd, CUR_DIR_NAME));
    }
    if (fd == -1)
        return -1;
    //char nextdir[MAX_FILE_NAME];
    char *nextdir;
    // Now, follows the path until end of it.
    while (*path) {
        path = nextdirname(path, &nextdir);
        
        // Search entry nextdir in the previous directory.
        dirent_p = finddir(fd, nextdir);
        if (dirent_p == NULL) {
            int err = errno;
            closedir(fd);
            errno = err;
            return -1;
        }
        
        // Close current directory and opens nextdir.
        closedir(fd);
        fd = opendir_ent(dirent_p);
        
        for (; *path == DIR_SEP; path++) {} // Removes all following separators
        }
    return fd;
}

fd_t opendir_ent(dirent_t *dirent) {
    if (dirent->type != DIR) {
        // We want to open a directory !
        errno = ENOTDIR;
        return -1;
    }
    fd_t fd = new_fd();
    if (fd == -1)
        return -1;
    
    strCopy(dirent->name, file_table[fd].name);
    file_table[fd].type = DIR;
    file_table[fd].start_cluster = dirent->cluster;
    file_table[fd].curr_cluster = file_table[fd].start_cluster;
    file_table[fd].curr_offset = 0;
    file_table[fd].prev_cluster = 0;
    file_table[fd].ent_cluster = dirent->ent_cluster;
    file_table[fd].ent_offset = dirent->ent_offset;
    file_table[fd].ent_size = dirent->ent_size;
    file_table[fd].mode = dirent->mode;
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
                dirent_p->mode = dirent->attributes.system ? SYSTEM : 0;
                dirent_p->mode |= dirent->attributes.rd_only ? RDONLY : 0;
                
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

int closedir(fd_t fd) {
    if (fd < 0 || fd >= MAX_NB_FILE || file_table[fd].type != DIR) {
        // No such directory, or bad fd.
        errno = EBADF;
        return -1;
    }
    free_fd(fd);
    return 0;
}

dirent_t *findent(fd_t dir, char *name, ftype_t type) {
    // Find the entry with specified name and type in directory dir.
    // Returns NULL if no such entry.
    dirent_t *dirent;
    while ((dirent = readdir(dir))) { // Check results ?
        if (strEqual(dirent->name, name)) {
            // Rewinds to avoid side effects.
            rewinddir(dir);
            if (dirent->type == type)
                return dirent;
            errno = (type == DIR) ? ENOTDIR : EISDIR;
            return NULL;    // This is not the wanted type.
        }
    }
    // Rewinds to avoid side effects.
    rewinddir(dir);
    errno = ENOENT; // Entry doesn't exist.
    return NULL;   
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
    oflags_t flags = O_APPEND | O_CREAT | O_RDWR;
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
    
    rewinddir(root);
    while ((dirent = readdir(root))) {
        print_short_dirent(dirent);
    }
    closedir(root);
    //init_stderr(NULL);
    fprintf(stderr, "I'm finally debugging normally !\n");
    flush(stderr);
    fd_t err = openfile("/error/stderr", flags);
    kprintf("Read %d from error \n", read(err, buffer, 400));
    kprintf("Errno : %s\n", strerror(errno));
    fprintf(NULL, "Content : (* %s *)\n", buffer);
    kprintf("Error %d : %s\n", 42, strerror(42));
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
    u8 mode = RDONLY;
    fill_dir_entry(cluster, buffer, CUR_DIR_NAME, DIR, mode);
    fill_dir_entry(cluster, &buffer[1], PARENT_DIR_NAME, DIR, mode);
    
    dirent_t dirent;
    dirent.cluster = cluster;
    dirent.ent_size = 2;
   
    new_entry(cluster, &dirent);
    kprintf("Cluster %d offset %d size %d\n", dirent.ent_cluster, dirent.ent_offset, dirent.ent_size );
    fill_entries(dirent.ent_cluster, dirent.ent_offset, dirent.ent_size, buffer);
    
}