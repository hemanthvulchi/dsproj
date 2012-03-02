#define PING_PORT 8011
#define COMMAND_PORT 8012
#define RESPONSE_PORT 8013
#define ACCESS "access"
#define GETATTR "getattr"
#define DATANODE "datanode"
#define READDIR "readdir"
#ifndef _global_h
#define _global_h
#define DNULL 0 
#define EOS '\0'
#define MAXF 200
#define REP  100
#define MAX_PATH 100
char *COMMAND_NAME;
struct stat *st_getattr;
//struct readdir_list *rlist_all[1024];
struct readdir_list *rlist_all;
#endif
