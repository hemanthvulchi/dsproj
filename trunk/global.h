#define MAX_PATH 100


#define PING_PORT 8011
#define COMMAND_PORT 8012
#define RESPONSE_PORT 8013
#define ACCESS "access"
#define GETATTR "getattr"
#define DATANODE "datanode"

#ifndef _global_h
#define _global_h
char *COMMAND_NAME;
struct stat *st_getattr;
#endif
