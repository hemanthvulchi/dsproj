#ifndef _global_h
#define _global_h

#define PING_PORT 8011
#define COMMAND_PORT 8012
#define RESPONSE_PORT 8013
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
#define DNULL 0 
#define EOS '\0'
#define MAXF 200
#define REP  100
#define MAX_PATH 100
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
char mkdir_buf;
#endif
