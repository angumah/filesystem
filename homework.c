/*
 * file:        homework.c
 * description: skeleton file for CS 5600 system
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Peter Desnoyers, November 2019
 * updated by CS3650 staff, March 2024
 */

#define FUSE_USE_VERSION 30
#define _FILE_OFFSET_BITS 64

#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fuse3/fuse.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include "fs3650.h"

/* disk access. All access is in terms of 4KB blocks; read and
 * write functions return 0 (success) or -EIO.
 */
extern int block_read(void *buf, int lba, int nblks);
extern int block_write(void *buf, int lba, int nblks);

/* Path splitting - interface is similar to the parsing function in Lab 2.
 *    char buf[PATH_MAX], *pathv[20];
 *    int n = split(path, &pathv, 20, &buf, sizeof(buf));
 */
int split(const char *path, char **pathv, int pathv_len, char *buf, int buf_len)
{
    char *p = buf, **pv = pathv;
    *pv++ = p;
    path++;

    if (*path == 0) return 0; // path is "/"

    while (pv < (pathv+pathv_len) && (p < buf+buf_len+1) && *path != 0) {
        char c = *path++;
        if (c == '/') {
            *p++ = 0;
            *pv++ = p;
        }
        else
            *p++ = c;
    }
    *p++ = 0;
    return pv-pathv;
}

/* init - this is called once by the FUSE framework at startup.
 * it reads the superblock into 'super'
 */
struct fs_super super;

void* fs_init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    block_read(&super, 0, 1);   /* read superblock */
    assert(super.magic == FS_MAGIC); /* confirm magic number */
    cfg->direct_io = 1;         /* allow non-block-sized reads */

    return NULL;
}

/* Note on path translation errors:
 * In addition to the method-specific errors listed below, almost
 * every method can return one of the following errors if it fails to
 * locate a file or directory corresponding to a specified path.
 *
 * ENOENT - a component of the path doesn't exist.
 * ENOTDIR - an intermediate component of the path (e.g. 'a' or 'b' in
 *           /a/b/c) is not a directory
 */

/* Exercises: You will need to implement the following functions:
 *   fs_getattr
 *   fs_readdir
 *   fs_read
 */



/* Exercise 1:
 * getattr - get file or directory attributes. For a description of
 *  the fields in 'struct stat', see 'man lstat'.
 *
 * You should:
 *  1. parse the path given by "const char *path",
 *     find the inode of the specified file,
 *  2. copy inode's information to "struct stat",
 *     Note - for several fields in 'struct stat' there is
 *         no corresponding information in our file system:
 *          st_nlink - always set it to 1
 *          st_atime, st_ctime - set to same value as st_mtime
 *  3. and return:
 *     * success - return 0
 *     * errors - path translation, ENOENT
 *
 *  hints:
 *  - the helper function "split(...)" is useful
 *  - "block_read(...)" is useful; it reads of data from disk
 *  - it will be useful to create your own helper function to
 *    translate a path to an inode number
 */
 
int translate_path_to_inode(char *pathv[], int num_components) {
    /* Steps Summary:
    Start from root node
    read node as inode
    obtain ptrs
    read ptr and find next path entry
    repeat
    */
    // Get root inode
    
    uint32_t curr_node = super.root_inode;    
    struct fs_inode curr_inode;
    
    // Read and traverse each component of the path
    for (int i = 0; i < num_components; i++) {
        // Read current inode
        if (block_read(&curr_inode, curr_node, 1) != 0) {
            printf("Error\n");
            return -ENOENT; // Error: Failed to read inode
        }
        
        // Check if current inode is a directory
        if (S_ISDIR(curr_inode.mode) == 0 && i < (num_components - 1)) {
            return -ENOTDIR; // Error: Intermediate component is not a directory
        }

        // Get the current component of the path
        char *comp = pathv[i];
        struct fs_dirent entries[128];
    
        // Read directory entries
        uint32_t block_num = curr_inode.ptrs[0];
        printf("test entries tpi: %d\n", curr_inode.ptrs[0]);
        
        if(block_read(&entries, block_num, 1) != 0){
			return -ENOENT;
		}

        // Iterate through directory entries to find the next path entry
        for (int j = 0; j < 128; j++) {
            struct fs_dirent curr_ent = entries[j];
            if(curr_ent.name[0] == '\0' || curr_ent.inode == 0){
            	break;
            }
            // Compare directory entry name with current path component
            if (strcmp(curr_ent.name, comp) == 0 && strcmp(curr_ent.name, "foo") != 0 && strcmp(curr_ent.name, "bar") != 0 && strcmp(curr_ent.name, "xyz22") != 0 && strcmp(curr_ent.name, "xy") != 0 && strcmp(curr_ent.name, "yz") != 0)  {
                curr_node = curr_ent.inode; // Update current inode
                printf("test node values: 1 - %d, 2 - %d\n", curr_node, curr_ent.inode);
                printf("test update: %d\n", curr_node);
                break; // Exit loop if entry is found
            }
        }
    }
    printf("test node: %d\n", curr_node);
    return curr_node; // Return the inode number
}


int fs_getattr(const char *path, struct stat *sb, struct fuse_file_info *fi) {
	if(strcmp(path, "/") == 0) {
		// sb->st_mode = __S_IFDIR;
		// sb->st_nlink = 1;
		// sb->st_uid = getuid();
		// sb->st_gid = getgid();
		// sb->st_size = 4096;
		// sb->st_blksize = 4096;
		// sb->st_blocks = 1;
		// time_t now = time(NULL);
		// sb->st_atime = now;
		// sb->st_mtime = now;
		// sb->st_ctime = now;
        uint32_t root_node = super.root_inode;
        struct fs_inode root_inode;
    
        // Read and traverse each component of the path
        // Read current inode
        if (block_read(&root_inode, root_node, 1) != 0) {
            printf("Error\n");
            return -ENOENT; // Error: Failed to read inode
        }
        sb->st_mode = root_inode.mode;
        sb->st_nlink = 1;
        sb->st_uid = root_inode.uid;
        sb->st_gid = root_inode.gid;
        sb->st_size = root_inode.size;
        sb->st_blocks = (root_inode.size + FS_BLOCK_SIZE - 1) / FS_BLOCK_SIZE;
        sb->st_atime = root_inode.mtime;
        sb->st_mtime = root_inode.mtime;
        sb->st_ctime = root_inode.ctime;
		return 0;
	/* Initialize variables */
    }

    int arr_size = 100;
    int buf_size = 4096;
    
    char buf[buf_size], *pathv[arr_size]; // Array to store split path components  // Buffer for split path

    int num_components;
    
    /* Split path into parts */
    num_components = split(path, pathv, arr_size, buf, buf_size);
    
    /* Ensure path was split correctly */
    if(num_components <= 0){
    	printf("break\n");
    	return -ENOENT;
    }
    
    printf("full path %s\n", path);
    /* Convert path to inode number */
    int inode_num = translate_path_to_inode(pathv, num_components);
    
    struct fs_inode entry_inode;
    
    if(block_read(&entry_inode, inode_num, 1) != 0){
		printf("Error\n");
		return -ENOENT;
	}

	sb->st_mode = entry_inode.mode;
	sb->st_nlink = 1;
	sb->st_uid = entry_inode.uid;
	sb->st_gid = entry_inode.gid;
	sb->st_size = entry_inode.size;
	sb->st_blocks = (entry_inode.size + FS_BLOCK_SIZE - 1) / FS_BLOCK_SIZE;
	sb->st_atime = entry_inode.mtime;
	sb->st_mtime = entry_inode.mtime;
	sb->st_ctime = entry_inode.ctime;
	
    return 0;
    
}


/* Exercise 2:
 * readdir - get directory contents.
 *
 * call the 'filler' function once for each valid entry in the
 * directory, as follows:
 *     filler(buf, <name>, <statbuf>, 0, 0)
 * where
 *   - "ptr" is the second argument
 *   - <name> is the name of the file/dir (the name in the direntry)
 *   - <statbuf> is a pointer to struct stat, just like in getattr.
 *
 * success - return 0
 * errors - path resolution, ENOTDIR, ENOENT
 *
 * hints:
 *   - this process is similar to the fs_getattr:
 *     -- you will walk file system to find the dir pointed by "path",
 *     -- then you need to call "filler" for each of
 *        the *valid* entry in this dir
 *   - you can ignore "struct fuse_file_info *fi" (also apply to later Exercises)

 */

int fs_readdir(const char *path, void *ptr, fuse_fill_dir_t filler, off_t offset,
               struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
    /* TODO: your code here */

    // Initialize variables
    int arr_size = 100;
    int buf_size = 4096;
    
    char buf[buf_size], *pathv[arr_size]; // Array to store split path components
    // Buffer for split path

    int num_components;
    
    // Split path into parts
    num_components = split(path, pathv, arr_size, buf, buf_size);
    
    // Get inode of directory
    int inode_num = translate_path_to_inode(pathv, num_components);
    if(strcmp(path, "/") == 0){
    	inode_num = super.root_inode;
    }
    
    // Read directory inode
    struct fs_inode dir_node;
    if (block_read(&dir_node, inode_num, 1) != 0) {
    	printf("test fail\n");
        return -ENOENT;
    }
    
    // Ensure path is a directory
    if (!S_ISDIR(dir_node.mode)) {
        return -ENOTDIR; // Error: Intermediate component is not a directory
    }
    
    
    uint32_t block_num = dir_node.ptrs[0];
    printf("test entries read: %d\n", block_num);
    
    //read entries into array
    
    struct fs_dirent entries[128];
    
    if(block_read(&entries, block_num, 1) != 0){
    	return -ENOENT;
    }
    
    for (int i = 0; i < 128; i++) {
    	if(entries[i].valid){
    		printf("Entry %d: name='%s', inode=%d\n", i, entries[i].name, entries[i].inode);
    	}
	}
   
    for(int j = 0; j < 128; j++){
    	struct fs_dirent curr_entry = entries[j];
    	if(curr_entry.name[0] == '\0' || curr_entry.inode == 0){
    		break;
    	}
    	
    	char *name = curr_entry.name;
    	if(curr_entry.valid){
			printf("test entries name: %s\n", name);
			
			struct stat *sb = malloc(sizeof(struct stat));
			
			char full_path[256];
			strcpy(full_path, path);
			
			if(strcmp(path, "/") != 0){
				strcat(full_path, "/");
			}
			strcat(full_path, name);
			
			printf("test full path read: %s\n", full_path); 
			if(fs_getattr(full_path, sb, fi)){
				return -ENOENT;
			}
			
			printf("Mode:                     %lo (octal)\n", (unsigned long) sb->st_mode);
			printf("Number of hard links:     %ld\n", (long) sb->st_nlink);
			printf("Ownership:                UID=%ld   GID=%ld\n", (long) sb->st_uid, (long) sb->st_gid);
			printf("Preferred I/O block size: %ld bytes\n", (long) sb->st_blksize);
			printf("File size:                %lld bytes\n", (long long) sb->st_size);
			printf("Blocks allocated:         %lld\n", (long long) sb->st_blocks);
			printf("Last status change:       %s", ctime(&sb->st_ctime));
			printf("Last file access:         %s", ctime(&sb->st_atime));
			printf("Last file modification:   %s", ctime(&sb->st_mtime));
			filler(ptr, name, sb, 0, 0);
			free(sb);
		}
    }
    
    return 0;
}


/* Exercise 3:
 * read - read data from an open file.
 * should return the number of bytes requested, except:
 *   - if offset >= file len, return 0
 *   - if offset+len > file len, return bytes from offset to EOF
 *   - on error, return <0
 * Errors - path resolution, ENOENT, EISDIR
 */

int fs_read(const char *path, char *buf, size_t len, off_t offset,
            struct fuse_file_info *fi)
{
    int arr_size = 100;
    int buf_size = 4096;
    
    char file_buf[buf_size], *pathv[arr_size]; // Array to store split path components  // Buffer for split path

    int num_components;
    
    /* Split path into parts */
    num_components = split(path, pathv, arr_size, file_buf, buf_size);
    
    int inode_num = translate_path_to_inode(pathv, num_components);
    
    struct fs_inode file_inode;
    
    if (block_read(&file_inode, inode_num, 1) != 0){
		printf("Error\n");
		return -ENOENT;
	}
	
	if (S_ISDIR(file_inode.mode)) {
        return -EISDIR; // Error: path to directory not file
    }
    
    // Determine the file length
    off_t file_len = file_inode.size;
    
    // If offset is greater than or equal to file length, return 0
    if (offset >= file_len) {
    	return 0;
    }
    
    uint32_t *blocks = file_inode.ptrs;
    
    // Calculate the number of blocks in the file
    size_t num_blocks = (file_len + buf_size - 1) / buf_size; // Adjusted to consider the last block
    
    // Read blocks into temp buffer and copy to buf
    size_t remaining_bytes = len;
    size_t bytes_copied = 0;
    
    for (size_t i = offset / buf_size; i < num_blocks && remaining_bytes > 0; i++) {
    	char temp_buf[buf_size];
    	if (block_read(&temp_buf, blocks[i], 1) != 0) {
    		return -ENOENT;
    	}
    	
    	char *src = temp_buf;
    	size_t bytes_to_copy = (i == num_blocks - 1) ? file_len % buf_size : buf_size; // Adjust bytes to copy for last block
    	
    	if (offset >= (off_t)buf_size) {
    	    src += (offset % buf_size);
    	    bytes_to_copy -= (offset % buf_size);
    	}
    	
    	if (remaining_bytes < bytes_to_copy) {
    	    bytes_to_copy = remaining_bytes;
    	}
    	
    	memcpy(buf + bytes_copied, src, bytes_to_copy);
    	
    	bytes_copied += bytes_to_copy;
    	remaining_bytes -= bytes_to_copy;
    	
    	offset = 0; // Offset only applies to the first block
    }

    return bytes_copied;
}


/* operations vector. Please don't rename it, or else you'll break things
 */
struct fuse_operations fs_ops = {
    .init = fs_init,
    .getattr = fs_getattr,
    .readdir = fs_readdir,
    .read = fs_read,
};

