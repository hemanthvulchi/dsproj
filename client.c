#define FUSE_USE_VERSION 26

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <fuse.h>
#include <sys/xattr.h>
#include <pthread.h>
#include "global.h"
#include "ping.c"
#include "socket_command.c"

char *namenode = NULL;

static int my_getattr(const char *path, struct stat *stbuf)
{
	printf("I am in getattr\n");
	printf("Path : %s\n", path);
	printf("st buf mode	: %d\n", stbuf->st_mode);
	printf("st buf ino 	: %d\n", stbuf->st_ino);
	printf("st buf dev 	: %d\n", stbuf->st_dev);
	printf("st buf rdev	: %d\n", stbuf->st_rdev);
	printf("st buf nlink	: %d\n", stbuf->st_nlink);
	printf("st buf uid	: %d\n", stbuf->st_uid);
	printf("st buf gid	: %d\n", stbuf->st_gid);
	printf("st buf size	: %d\n", stbuf->st_size);
	printf("st buf atime	: %d\n", stbuf->st_atime);
	printf("st buf mtime	: %d\n", stbuf->st_mtime);
	printf("st buf ctime	: %d\n", stbuf->st_ctime);
	printf("st buf blksize	: %d\n", stbuf->st_blksize);
	printf("st buf blocks	: %d\n", stbuf->st_blocks);
	int res;
	pthread_t recvcmd_thread;

	COMMAND_NAME = malloc (1+sizeof(char)*strlen(GETATTR));
	strcpy(COMMAND_NAME, GETATTR);
	int cmd_rc = pthread_create(&recvcmd_thread, NULL, receiveresponse_client, NULL);
        if (cmd_rc)
        {
                printf("Name node not to able to initiate receive comamnd thread\n");
		free(COMMAND_NAME);
                exit(1);
        }

	if (sendcommand(namenode, path, GETATTR) == -1)
	{
		pthread_kill(recvcmd_thread,0);
		free(COMMAND_NAME);
		return -1;
	}
	// now receive response and do something..
	pthread_join(recvcmd_thread, NULL);
	free(COMMAND_NAME);
	
	//res = lstat(path, stbuf);
	//printf("stbuf uid : %d\n", st_getattr->st_uid);
	//now copy stuff from st_getattr to stbuf
	memset(stbuf, 0, sizeof(struct stat));

	free(st_getattr);
	if (res == -1)
		return -errno;

	printf("End of getattr\n");
	return 0;
}

static int xmp_access(const char *path, int mask)
{
	printf("I am in access\n");
	printf("\n\n\nFirst funtion to use\n\n\n");
	int res = 0;
	printf("Path : %s\n",path);

	if (strcmp(path,"/") != 0)
	{
		COMMAND_NAME = malloc (1+sizeof(char)*strlen(GETATTR));
        	strcpy(COMMAND_NAME, GETATTR);

		if (sendcommand(namenode, path, ACCESS) == -1)
        	{
                	free(COMMAND_NAME);
                	return -1;
        	}
        	free(COMMAND_NAME);
		res = -1;
	}

	//res = access(path, mask);
	if (res == -1)
		return -errno;
	
	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	printf("I am in readlink\n");
	int res;

	res = readlink(path, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}


static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	printf("I am in readdir\n");

	// I have to send the path from here to the namenode..
	// Get back the list of files..
	COMMAND_NAME = malloc (1+sizeof(char)*strlen(GETATTR));
        strcpy(COMMAND_NAME, READDIR);
	pthread_t recvcmd_thread;
        int cmd_rc = pthread_create(&recvcmd_thread, NULL, receiveresponse_client, NULL);
        if (cmd_rc)
        {
                printf("Name node not to able to initiate receive comamnd thread\n");
                free(COMMAND_NAME);
                exit(1);
        }
	if (sendcommand(namenode, path, READDIR) == -1)
        {
                pthread_kill(recvcmd_thread,0);
                free(COMMAND_NAME);
                return -1;
        }
	pthread_join(recvcmd_thread, NULL);
        free(COMMAND_NAME);

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(path);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	printf("I am in xmp_mknod\n");
	int res;

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
		res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(path, mode);
	else
		res = mknod(path, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	printf("I am in xmp_mkdir\n");
	int res;

	res = mkdir(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_unlink(const char *path)
{
	printf("Unlink\n");
	int res;

	res = unlink(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
	printf("rmdir\n");
	int res;

	res = rmdir(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
	printf("symlink\n");
	int res;

	res = symlink(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
	printf("rename\n");
	int res;

	res = rename(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_link(const char *from, const char *to)
{
	printf("link\n");
	int res;

	res = link(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	printf("chmod\n");
	int res;

	res = chmod(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
	printf("chown\n");
	int res;

	res = lchown(path, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	printf("truncate\n");
	int res;

	res = truncate(path, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	printf("utimens\n");
	int res;
	struct timeval tv[2];

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(path, tv);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	printf("I am in open\n");
	int res;

	res = open(path, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	printf("I am in read\n");
	int fd;
	int res;

	(void) fi;
	fd = open(path, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	printf("I am in write\n");
	int fd;
	int res;

	(void) fi;
	fd = open(path, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	printf("I am in statfs\n");
	int res;

	res = statvfs(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	printf("I am in release\n");
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) fi;
	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}


#ifdef HAVE_SETXATTR
static int xmp_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
	printf("I am in setxattr\n");
	int res = lsetxattr(path, name, value, size, flags);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
	printf("I am in getxattr\n");
	int res = lgetxattr(path, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
{
	printf("I am in list xattr\n");
	int res = llistxattr(path, list, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_removexattr(const char *path, const char *name)
{
	printf("I am in remove xattr\n");
	int res = lremovexattr(path, name);
	if (res == -1)
		return -errno;
	return 0;
}
#endif /* HAVE_SETXATTR */

static struct fuse_operations xmp_oper = {
	.getattr	= my_getattr,
	.access		= xmp_access,
	.readlink	= xmp_readlink,
	.readdir	= xmp_readdir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.symlink	= xmp_symlink,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.link		= xmp_link,
	.chmod		= xmp_chmod,
	.chown		= xmp_chown,
	.truncate	= xmp_truncate,
	.utimens	= xmp_utimens,
	.open		= xmp_open,
	.read		= xmp_read,
	.write		= xmp_write,
	.statfs		= xmp_statfs,
	.release	= xmp_release,
	.fsync		= xmp_fsync,
#ifdef HAVE_SETXATTR
	.setxattr	= xmp_setxattr,
	.getxattr	= xmp_getxattr,
	.listxattr	= xmp_listxattr,
	.removexattr	= xmp_removexattr,
#endif
};

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
	// Check server details here - return -1 for error..
	namenode = malloc (strlen(argv[1])*sizeof(char)+1);
	// Ping Server and check if host exists.
	if(sendping(argv[1]) == -1)
		return -1;
	printf("Ping success\n");
	strcpy(namenode,argv[1]);

	fuse_argc = 2;
	fuse_argv[0] = malloc(strlen(argv[0])*sizeof(char)+1);
	strcpy(fuse_argv[0],argv[0]);
	fuse_argv[1] = malloc(strlen(argv[2])*sizeof(char)+1);
	strcpy(fuse_argv[1],argv[2]);

	/*
	int i = 3;
	for (i = 3; i < argc; i++)
	{
		if (strcmp(argv[i],"-d") == 0)*/
        		add_fuse_arg("-d");
	//}

	//To add any more argument in the future, use the below command as a sample
	//add_fuse_arg(argv[2]);
	return 1;
}

int main(int argc, char *argv[])
{
	//USAGE
	if(argc < 3)
	{
		printf("\nUSAGE : %s <namenode-name> <mnt-path>\n\n",argv[0]);
		exit(1);
	}

	if (parse_arguments(argc, argv) == -1)
	{
		printf("ERROR in arguments passed\n");
		exit(1);
	}
	pthread_t recvping_thread;
	int rc = pthread_create(&recvping_thread, NULL, receiveping, NULL);
	if (rc)
	{
		printf("receive ping thread create error\n");
		exit(1);
	}
	//I shouldn't do join.. Let the thread work till the client process is alive.. Makes perfect sense..
	//pthread_join(recvping_thread,NULL);

	//I dunno why they use umask.. will figure it out at the end..
	//umask(0);
	return fuse_main(fuse_argc, fuse_argv, &xmp_oper, NULL);
}
