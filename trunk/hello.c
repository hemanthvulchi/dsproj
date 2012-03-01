#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";

static int hello_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, hello_path) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(hello_str);
	} else
		res = -ENOENT;
	return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, hello_path + 1, NULL, 0);

	return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
	if (strcmp(path, hello_path) != 0)
		return -ENOENT;

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;
	if(strcmp(path, hello_path) != 0)
		return -ENOENT;

	len = strlen(hello_str);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, hello_str + offset, size);
	} else
		size = 0;

	return size;
}

/*
int fuse_argc = 1;
char *fuse_argv[2];
void add_fuse_arg(char* new_arg)
{
    char* new_alloc;
    new_alloc=malloc(strlen(new_arg)+1);
    strcpy(new_alloc, new_arg);
    fuse_argv[fuse_argc++]=new_alloc;
}

//Return 1 on success, -1 for termination.
int parse_arguments(int argc, char **argv)
{
        //Check server details here - return -1 for error..
        fuse_argc = 2;
        fuse_argv[0] = malloc(strlen(argv[0])*sizeof(char)+1);
        strcpy(fuse_argv[0],argv[0]);
        fuse_argv[1] = malloc(strlen(argv[2])*sizeof(char)+1);
        strcpy(fuse_argv[1],argv[2]);

	
        int i = 3;
        for (i = 3; i < argc; i++)
        {
                if (strcmp(argv[i],"-d") == 0)
                        add_fuse_arg("-d");
        }

        //To add any more argument in the future, use the below command as a sample
        //add_fuse_arg(argv[2]);
        return 1;
}
*/

static struct fuse_operations hello_oper = {
	.getattr	= hello_getattr,
	.readdir	= hello_readdir,
	.open		= hello_open,
	.read		= hello_read,
};

int main(int argc, char *argv[])
{
	//USAGE
	/*
        if(argc < 3)
        {
                printf("\nUSAGE : %s <namenode-name> <mnt-path>\n\n",argv[0]);
                exit(1);
        }
	*/
        //if (parse_arguments(argc, argv) == -1) exit(1);

	printf("I am here\n");
        //I dunno why they use umask.. will figure it out at the end..
        umask(0);
        return fuse_main(argc, argv, &hello_oper, NULL);
}
