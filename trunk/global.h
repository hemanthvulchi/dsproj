#ifndef _global_h
#define _global_h

#define DATANODE_PATH "/tmp/CS545"
#define DATANODE_DIR "/tmp/CS545/datanode"
#define PING_PORT 8011
#define COMMAND_PORT 8012
#define RESPONSE_PORT 8013

#define REPLICATION_PORT 8019

#define ACCESS "access"
#define GETATTR "getattr"
#define DATANODE "datanode"
#define READDIR "readdir"
#define MKDIR "mkdir"
#define MKNOD "mknod"
#define TRUNCATE "truncate"
#define OPEN "open"
#define READ "read"
#define WRITE "write"
#define UNLINK "unlink"
#define RMDIR "rmdir"
#define STATFS "statfs"
#define CHMOD "chmod"
#define RENAME "rename"
#define DNULL 0 
#define EOS '\0'
#define MAXF 200
#define REP  5
#define MAX_PATH 100
#define SERV_PATH 100


#define R_FILEEXIST "replica_fileexists"
char *R_COMMAND_NAME;
char *COMMAND_NAME;
char getattr_buf[1000];
//struct readdir_list *rlist_all[1024];
struct readdir_list *rlist_all;
char readdir_buf[10000];
char access_retbuf[100];
char open_retbuf[100];
char read_retbuf[4096];
char write_retbuf[100];
char mknod_retbuf[100];
char truncate_retbuf[100];
char mkdir_buf[10];
char unlink_retbuf[100];
char rmdir_retbuf[100];
char statvfs_retbuf[1000];
char rename_buf[10];
char chmod_buf[10];

char r_fexist_buf[100];
#endif
