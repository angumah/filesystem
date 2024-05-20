/*
 * file:        lab4fuse.c
 * description: main() for Lab 4
 *   DO NOT MODIFY THIS FILE
 */
#define FUSE_USE_VERSION 30
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <fuse3/fuse.h>

#include "fs3650.h"

extern void block_init(char *file);

/* All homework functions are accessed through the operations
 * structure.
 */
extern struct fuse_operations fs_ops;

struct data {
    char *image_name;
    int   part;
    int   cmd_mode;
} _data;

/**************/

/*
 * See comments in /usr/include/fuse/fuse_opts.h for details of
 * FUSE argument processing.
 *
 *  usage: ./<exe> -image disk.img [-part #] directory
 *              disk.img  - name of the image file to mount
 *              directory - directory to mount it on
 */
static struct fuse_opt opts[] = {
    {"-image %s", offsetof(struct data, image_name), 0},
    FUSE_OPT_END
};

int main(int argc, char **argv)
{
    /* Argument processing and checking
     */
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    if (fuse_opt_parse(&args, &_data, opts, NULL) == -1)
	exit(1);

    block_init(_data.image_name);

    return fuse_main(args.argc, args.argv, &fs_ops, NULL);
}
