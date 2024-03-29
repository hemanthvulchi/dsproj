#ifndef __COMMON_H
#define __COMMON_H


#include <stdint.h>
#include <sys/types.h>

#include "protocol.h"
#include "corefs_api.h"


#define SERVER_PORT 4444
#define DATANODE_PORT 5555
inline int donothing(FILE* f, const char* fmt, ...);
#ifdef DEBUG
#define dprintf fprintf
#else
#define dprintf donothing
#endif

/*  **CAUTION: standard buffer size 128k ***/
/*  133120 = 130*1024. */
/*  This limit is because fuse single request limits 128K data
 *  transfer. This is defined in FUSE_MAX_PAGES_PER_REQ in
 *  kernel/fuse_i.h. If you are using more than 128KB request, then
 *  make sure to change the following two macros. Make sure to keep
 *  additional 2KB for corefs packets. */
    
#define BUFFERSIZE 133120


#define MAX_PATH MAX_COREFS_PATH
#define TOKEN_LEN 81
#define PRKEY 1
#define PUBKEY 2

/* Handles encoding/ decoding and endianess */
unsigned int encap_corefs_header(char * buf, corefs_packet * pkt);
unsigned int decap_corefs_header(char * buf, corefs_packet * pkt);
unsigned int encap_corefs_response(char* bug, corefs_packet *pkt);
unsigned int decap_corefs_response(char* bug, corefs_packet *pkt);
unsigned int encap_corefs_request(char* bug, corefs_packet *pkt);
unsigned int decap_corefs_request(char* bug, corefs_packet *pkt);



/* Fuctions to estimate the size of various structures */

#define SIZEOF_HEADER(header) (sizeof(header.magic) + sizeof(header.type) + sizeof(header.sequence)+  sizeof(header.payload_size)) 

#define REQUEST_BASE_SIZE(req) (sizeof(req.type) + sizeof(req.user_ids.uid) + sizeof(req.user_ids.gid) + sizeof(req.user_ids.num_sgids) + sizeof(req.user_ids.gid) * req.user_ids.num_sgids)

#define RESPONSE_BASE_SIZE(resp) (sizeof(resp.type) +  sizeof(resp.more_offset))

#define SIZEOF_FILEOP(fileop) (sizeof(fileop.type) + sizeof(fileop.offset) + sizeof(fileop.size) + sizeof(fileop.pathlen) + fileop.pathlen)

#define SIZEOF_SIMPLEOP(simple) 	(sizeof(simple.type)+ sizeof(simple.offset) + sizeof(simple.mode1) + sizeof(simple.path1len) + sizeof(simple.path2len) + simple.path1len + 1 + simple.path2len)

#define SIZEOF_XATTR(xattr) ( sizeof(xattr.type) + sizeof(xattr.flags) + sizeof(xattr.sizeofvalue) + sizeof(xattr.pathlen) + sizeof(xattr.namelen) + xattr.pathlen + 1 + xattr.namelen)

#define SIZEOF_STATUS(status) sizeof(status.bits)

#define SIZEOF_ATTR(attr) (sizeof(attr.mode) + sizeof(attr.uid) + sizeof(attr.gid) +  sizeof(attr.size) + sizeof(attr.mtime) +  sizeof(attr.atime) +  sizeof(attr.ctime) + sizeof(attr.nlinks))


/* Functions to estimate the size of various structures without variable data, such as path etc. */
#define SIZEOF_FILEOP_NOPATH(fileop) sizeof(fileop.type) + sizeof(fileop.offset) + sizeof(fileop.size) + sizeof(fileop.pathlen)
#define SIZEOF_SIMPLEOP_NOPATH(simple) 	sizeof(simple.type)+ sizeof(simple.offset) + sizeof(simple.mode1) + sizeof(simple.path1len) + sizeof(simple.path2len)


void init_sizes(void);


/* Common functions to fill up the packet fields. */
unsigned int build_header(corefs_packet* packet, unsigned int type);
unsigned int build_fileop(corefs_packet* packet, unsigned int type, unsigned int offset, unsigned int size, const char* path);
unsigned int build_simple(corefs_packet* packet, unsigned int type, const char* path1, off_t offset, mode_t mode1, const char * opt_path); // opt_path added for symlink, rename and such commands
unsigned int build_request_data(corefs_packet* packet, const char* data, size_t size);
unsigned int build_response_data(corefs_packet* packet, const char* data, size_t size);
unsigned int build_response_with_moredata(corefs_packet* packet, const char* data, size_t size, unsigned int offset);
unsigned int build_status(corefs_packet* packet, unsigned int status, unsigned int type);
unsigned int build_xattr(corefs_packet * packet, unsigned int type, const char * name, const char *path, int size, int flags);

void print_packet(corefs_packet p);
int socket_read(int sock, char* buffer, size_t size);
int socket_write(int sock, char* buffer, size_t size);
int receive_packet(COMMCTX* ctx, char* buffer);
int send_packet(COMMCTX* ctx, char* buffer, size_t size);
int server_receive_specified(COMMCTX* ctx, char* buffer, unsigned int type);
int client_receive_specified(COMMCTX* ctx, char* buffer, unsigned int type);


#endif
