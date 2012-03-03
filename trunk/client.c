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

char tmp_path[100] = "/tmp/shyam-fuse";

char *namenode = NULL;

static int my_getattr(const char *path, struct stat *stbuf)
{
	printf("\n\nI am in getattr\n\n");
	printf("Path : %s\n", path);
	int res;
	pthread_t recvcmd_thread;
		
	char my_path[100];
	memset(my_path,'\0',100);
	strcpy(my_path,tmp_path);
	strcat(my_path,path);

	printf("My Path : %s\n", my_path);
	COMMAND_NAME = malloc (1+sizeof(char)*strlen(GETATTR));
	strcpy(COMMAND_NAME, GETATTR);
	int cmd_rc = pthread_create(&recvcmd_thread, NULL, receiveresponse_client, NULL);
        if (cmd_rc)
        {
                printf("Name node not to able to initiate receive comamnd thread\n");
		free(COMMAND_NAME);
                exit(1);
        }
	printf("\nGetattr sent thread");
	if (sendcommand(namenode, my_path, GETATTR) == -1)
	{
		printf("\nGetattr before thread kill");
		pthread_kill(recvcmd_thread,0);
		free(COMMAND_NAME);
		return -1;
	}
	// now receive response and do something..
	printf("\nGetattr before join");
	pthread_join(recvcmd_thread, NULL);
	free(COMMAND_NAME);
	
	//res = lstat(path, stbuf);
	//now copy stuff from st_getattr to stbuf
	memset(stbuf, 0, sizeof(struct stat));

	char *resv = strtok(getattr_buf,",");
	res = atoi(resv);
	if (res == -1)
	{
		char *err = strtok(NULL,",");
		errno = atoi(err);
		return -errno;
	}

	char *err = strtok(NULL,",");
	errno = atoi(err); // This should be 0..
	printf("Errno : %d\n", errno);

	char *tmp;
	tmp = strtok(NULL,",");
	stbuf->st_mode = atoi(tmp);

	tmp = strtok(NULL,",");
	stbuf->st_ino = atoi(tmp);

	tmp = strtok(NULL,",");
	stbuf->st_dev = atoi(tmp);

	tmp = strtok(NULL,",");
	stbuf->st_rdev = atoi(tmp);

	tmp = strtok(NULL,",");
	stbuf->st_nlink = atoi(tmp);

	tmp = strtok(NULL,",");
	stbuf->st_uid = atoi(tmp);

	tmp = strtok(NULL,",");
	stbuf->st_gid = atoi(tmp);

	tmp = strtok(NULL,",");
	stbuf->st_size = atoi(tmp);

	tmp = strtok(NULL,",");
	stbuf->st_atime = atoi(tmp);

	tmp = strtok(NULL,",");
	stbuf->st_mtime = atoi(tmp);

	tmp = strtok(NULL,",");
	stbuf->st_ctime = atoi(tmp);

	tmp = strtok(NULL,",");
	stbuf->st_blksize = atoi(tmp);

	tmp = strtok(NULL,",");
	stbuf->st_blocks = atoi(tmp);
	
	/*
	printf("st buf mode     : %d\n", stbuf->st_mode);
        printf("st buf ino      : %d\n", stbuf->st_ino);
        printf("st buf dev      : %d\n", stbuf->st_dev);
        printf("st buf rdev     : %d\n", stbuf->st_rdev);
        printf("st buf nlink    : %d\n", stbuf->st_nlink);
        printf("st buf uid      : %d\n", stbuf->st_uid);
        printf("st buf gid      : %d\n", stbuf->st_gid);
        printf("st buf size     : %d\n", stbuf->st_size);
        printf("st buf atime    : %d\n", stbuf->st_atime);
        printf("st buf mtime    : %d\n", stbuf->st_mtime);
        printf("st buf ctime    : %d\n", stbuf->st_ctime);
        printf("st buf blksize  : %d\n", stbuf->st_blksize);
        printf("st buf blocks   : %d\n", stbuf->st_blocks);
	*/
	printf("End of getattr %s\n",path);
	return 0;
}

static int xmp_access(const char *path, int mask)
{
	printf("\n\nI am in access\n\n");
	printf("\n\n\nFirst funtion to use\n\n\n");
	int res = 0;

	printf("Path : %s\n",path);
	char my_path[100];
        //strcpy(my_path,"/tmp/shyam-fuse");
        strcpy(my_path,tmp_path);
        strcat(my_path,path);

	if (strcmp(path,"/") != 0)
	{
		pthread_t recvcmd_thread;
        	int cmd_rc = pthread_create(&recvcmd_thread, NULL, receiveresponse_client, NULL);
        	if (cmd_rc)
        	{
                	printf("Name node not to able to initiate receive comamnd thread\n");
                	free(COMMAND_NAME);
                	exit(1);
        	}

		COMMAND_NAME = malloc (1+sizeof(char)*strlen(GETATTR));
       		strcpy(COMMAND_NAME, GETATTR);
		char buffer[30];
                memset(buffer,'\0',30);
                snprintf(buffer, 10,"%d",mask);
		strcat(my_path,",");
		strcat(my_path,buffer);
		if (sendcommand(namenode, my_path, ACCESS) == -1)
       		{
			printf("Send command failed\n");
                	pthread_kill(recvcmd_thread,0);
               		free(COMMAND_NAME);
               		return -1;
       		}
		printf("Waiting for pthread join\n");
		pthread_join(recvcmd_thread, NULL);
       		free(COMMAND_NAME);
		res = access_return;
	}
	//res = access(path, mask);
	if (res == -1)
		return -errno;
	
	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	printf("\n\nI am in readlink\n\n");
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
	printf("\n\nI am in readdir %s\n",path);
	char my_path[100];
	memset(my_path,'\0',100);
        //strcpy(my_path,"/tmp/shyam-fuse");
        strcpy(my_path,tmp_path);
        strcat(my_path,path);

	// I have to send the path from here to the namenode..
	// Get back the list of files..
	COMMAND_NAME = malloc (1+sizeof(char)*strlen(GETATTR));
        strcpy(COMMAND_NAME, READDIR);
	pthread_t recvcmd_thread;
        int cmd_rc = 0;
	pthread_create(&recvcmd_thread, NULL, receiveresponse_client, NULL);
        if (cmd_rc)
        {
                printf("Name node not to able to initiate receive comamnd thread\n");
                free(COMMAND_NAME);
                exit(1);
        }
	if (sendcommand(namenode, my_path, READDIR) == -1)
        {
		printf("Send command failed\n");
                pthread_kill(recvcmd_thread,0);
                free(COMMAND_NAME);
                return -1;
        }
	printf("Waiting for pthread join\n");
	pthread_join(recvcmd_thread, NULL);
        free(COMMAND_NAME);
	printf("End of command\n");

	(void) offset;
	(void) fi;

	char *val = strtok(readdir_buf,",");
	while (val != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		char name[100];
		strcpy(name,val);
		val = strtok(NULL,",");
		st.st_ino = atoi(val);
		val = strtok(NULL,",");
		st.st_mode = atoi(val) << 12;
		if (filler(buf,name, &st, 0))
			break;
		val = strtok(NULL,",");
	}
	printf("End of readdir %s\n",path);
	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	printf("\n\nI am in xmp_mknod\n\n");
	printf("Mode : %d\n", mode);
	printf("Dev : %d\n", rdev);
	int res;

	char my_path[100];
        //strcpy(my_path,"/tmp/shyam-fuse");
        strcpy(my_path,tmp_path);
        strcat(my_path,path);
	printf("Path : %s\n", path);

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(my_path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
		res = close(res);
		}
	else if (S_ISFIFO(mode))
		res = mkfifo(my_path, mode);
	else
		res = mknod(my_path, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	printf("I am in xmp_mkdir :path:%s mode:%d \n",path,mode);
	int res;
		COMMAND_NAME = malloc (1+sizeof(char)*strlen(MKDIR));
        strcpy(COMMAND_NAME, MKDIR);
	pthread_t recvcmd_thread;
        int cmd_rc = 0;
	pthread_create(&recvcmd_thread, NULL, receiveresponse_client, NULL);
	printf("Make dir: Started receieving\n");
        if (cmd_rc)
        {
                printf("Name node not to able to initiate receive comamnd thread\n");
                free(COMMAND_NAME);
                exit(1);
        }
	if (sendcommand(namenode, path, READDIR) == -1)
        {
		printf("Send command failed\n");
                pthread_kill(recvcmd_thread,0);
                free(COMMAND_NAME);
                return -1;
        }
	printf("Make dir: Command Sent\n");        
	printf("Waiting for pthread join\n");
	pthread_join(recvcmd_thread, NULL);
        free(COMMAND_NAME);
	printf("End of command\n");

/*
	res = mkdir(path, mode);
	if (res == -1)
		return -errno;
*/
	return 0;
}

static int xmp_unlink(const char *path)
{
	printf("\n\nUnlink\n\n");
	int res;

	char my_path[100];
        strcpy(my_path,tmp_path);
        strcat(my_path,path);

	res = unlink(my_path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
	printf("\n\nrmdir\n\n");
	int res;

	res = rmdir(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
	printf("\n\nsymlink\n\n");
	int res;

	res = symlink(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
	printf("\n\nrename\n\n");
	int res;

	res = rename(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_link(const char *from, const char *to)
{
	printf("\n\n link\n\n");
	int res;

	res = link(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	printf("\n\nchmod\n\n");
	int res;

	res = chmod(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
	printf("\n\nchown\n\n");
	int res;

	res = lchown(path, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	printf("\n\ntruncate\n\n");
	int res;

	res = truncate(path, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	printf("\n\nutimens\n\n");
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
	printf("\n\nI am in open\n\n");
	char my_path[100];
        strcpy(my_path,tmp_path);
        strcat(my_path,path);

	int res;
	printf("Fi flags %d\n", fi->flags);

	// I have to send the path from here to the namenode..
        // Get back the list of files..
        COMMAND_NAME = malloc (1+sizeof(char)*strlen(OPEN));
        strcpy(COMMAND_NAME, OPEN);
        pthread_t recvcmd_thread;
        int cmd_rc = 0;
        pthread_create(&recvcmd_thread, NULL, receiveresponse_client, NULL);
        if (cmd_rc)
        {
                printf("Name node not to able to initiate receive comamnd thread\n");
                free(COMMAND_NAME);
                exit(1);
        }
        char buffer[30];
        memset(buffer,'\0',30);
        snprintf(buffer, 10,"%d",fi->flags);
        strcat(my_path,",");
        strcat(my_path,buffer);

        if (sendcommand(namenode, my_path, OPEN) == -1)
        {
                printf("Send command failed\n");
                pthread_kill(recvcmd_thread,0);
                free(COMMAND_NAME);
                return -1;
        }
        printf("Waiting for pthread join\n");
        pthread_join(recvcmd_thread, NULL);
        free(COMMAND_NAME);
        printf("End of command\n");

	res = open_return;
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	printf("\n\nI am in read\n\n");
	printf("Path : %s\n",path);
	printf("Buf : %s\n", buf);
	printf("Offset : %d\n", offset);

	char my_path[100];
        strcpy(my_path,tmp_path);
        strcat(my_path,path);

        int res;

        // I have to send the path from here to the namenode..
        // Get back the list of files..
        COMMAND_NAME = malloc (1+sizeof(char)*strlen(READ));
        strcpy(COMMAND_NAME, READ);
        pthread_t recvcmd_thread;
        int cmd_rc = 0;
        pthread_create(&recvcmd_thread, NULL, receiveresponse_client, NULL);
        if (cmd_rc)
        {
                printf("Name node not to able to initiate receive comamnd thread\n");
                free(COMMAND_NAME);
                exit(1);
        }
        char buffer[30];
        memset(buffer,'\0',30);
        strcat(my_path,",");
        snprintf(buffer, 10,"%d",size);
        strcat(my_path,",");
        snprintf(buffer, 10,"%d",offset);
        strcat(my_path,buffer);

        if (sendcommand(namenode, my_path, READ) == -1)
        {
                printf("Send command failed\n");
                pthread_kill(recvcmd_thread,0);
                free(COMMAND_NAME);
                return -1;
        }
        printf("Waiting for pthread join\n");
        pthread_join(recvcmd_thread, NULL);
        free(COMMAND_NAME);
        printf("End of command\n");

	/*
	int fd;

	(void) fi;
	fd = open(path, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;
	
	close(fd);*/
	printf("Read Buf : %s\n",read_buf);
	//free(buf);
	//buf = malloc (strlen(read_buf)+1);
	//strcat(buf,"");
	strcpy(buf,read_buf);
	res = strlen(read_buf);
        if (res == -1)
                return -errno;
	
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	printf("\n\nI am in write\n\n");
	printf("Path : %s\n",path);
        printf("Buf : %s\n", buf);
        printf("Size : %s\n", size);
        printf("Offset : %d\n", offset);

        char my_path[10000];
        strcpy(my_path,tmp_path);
        strcat(my_path,path);

	int res;
	// I have to send the path from here to the namenode..
        // Get back the list of files..
        COMMAND_NAME = malloc (1+sizeof(char)*strlen(WRITE));
        strcpy(COMMAND_NAME, WRITE);
        pthread_t recvcmd_thread;
        int cmd_rc = 0;
        pthread_create(&recvcmd_thread, NULL, receiveresponse_client, NULL);
        if (cmd_rc)
        {
                printf("Name node not to able to initiate receive comamnd thread\n");
                free(COMMAND_NAME);
                exit(1);
        }
        char buffer[30];
        memset(buffer,'\0',30);
        strcat(my_path,",");
        snprintf(buffer, 10,"%d",size);
        strcat(my_path,",");
        snprintf(buffer, 10,"%d",offset);
        strcat(my_path,buffer);
	strcat(my_path,",");
	strcat(my_path,buf);

        if (sendcommand(namenode, my_path, WRITE) == -1)
        {
                printf("Send command failed\n");
                pthread_kill(recvcmd_thread,0);
                free(COMMAND_NAME);
                return -1;
        }
        printf("Waiting for pthread join\n");
        pthread_join(recvcmd_thread, NULL);
        free(COMMAND_NAME);
        printf("End of command\n");

	res = write_return;
	if (res == -1)
		res = -errno;
	/*
	int fd;
	(void) fi;
	fd = open(path, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;
	
	close(fd);*/
	return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	printf("\n\nI am in statfs\n\n");
	int res;

	res = statvfs(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	printf("\n\nI am in release\n\n");
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
	printf("\n\nI am in setxattr\n\n");
	int res = lsetxattr(path, name, value, size, flags);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
	printf("\n\nI am in getxattr\n\n");
	int res = lgetxattr(path, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
{
	printf("\n\nI am in list xattr\n\n");
	int res = llistxattr(path, list, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_removexattr(const char *path, const char *name)
{
	printf("\n\nI am in remove xattr\n\n");
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
	printf("namenode %s\n",namenode);
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
