/* 
 * SNFS API Layer
 * 
 * snfs_api.c
 *
 * Implementation of the SNFS API layer of the client side SNFS library.
 * 
 * Read the SNFS API interface (snfs_api.h) and the project specification 
 * for futher information about the services.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <strings.h>

#include <snfs_api.h>
#include <snfs_proto.h>


/*
 * Internal private global variables
 */

// the client socket file descriptor
static int Cli_sock;

// the client socket address
static struct sockaddr_un Cli_addr;

/* multi-server */
#define NUMBER_OF_SERVERS_TO_CONTACT 1

// the server socket address
static struct sockaddr_un Serv_addr;

// serial number to use with next request
static snfs_req_serial_num_t next_sn;


/*
 * Internal private auxiliary functions
 */

/*
 * Makes a remote call, sending 'req' to the server (defined in the global variable Serv_addr)
 * and waiting for the response.

 * SUPPORT FOR MULTIPLE REPLICATED SERVERS [TO BE IMPLEMENTED]:
 If multiple servers are supported, then 'to_all_servers == 1'
 means that we must send the request to every server and then wait until a response from
 every server is received. 
 If, otherwise, 'to_all_servers == 0', then we only need to send the request to 
 NUMBER_OF_SERVERS_TO_CONTACT servers and then return as soon as the response from the first server
 arrives.
*/

static int remote_call(snfs_msg_req_t *req, int reqsz, snfs_msg_res_t *res, 
   int ressz, int to_all_servers)
{
   int status;
   
   req->sn = (next_sn ++);
   //printf("DEBUG: Sent request with serial number %d.\n", req->sn);

   // sends the request to the server
   status = sendto(Cli_sock, (void*)req, reqsz, 0, 
      (struct sockaddr *)&Serv_addr, sizeof(Serv_addr));
   if (status < 0) {
     //printf("DEBUG: serv_addr: %s\n", Serv_addr.sun_path);
      printf("[snfs_api] sendto error: %s.\n", strerror(errno));
      return -1;
   }
  
   // waits for an answer
   status = recvfrom(Cli_sock, res, ressz, 0, NULL, NULL);
   if (status < 0) {
      printf("[snfs_api] recvfrom error: %s.\n", strerror(errno));
      return -1;
   }
   if (status == 0) {
      printf("[snfs_api] server is closed.\n");
      return -1;
   }
   //printf("DEBUG: Received response with serial number %d.\n", res->sn);
   if (res->sn != req->sn) {
     printf("[snfs_api] received response to wrong request serial number.\n");
     return -1;
   }

   return status;
}


/*
 * SNFS API implementation (see snfs_api.h)
 */

int snfs_init(char* cli_name, char* server_name)
{
   if (cli_name == NULL || server_name == NULL) {
      printf("[snfs_api] invalid client/server address names.\n");
      return -1;
   }

   // creates socket datagram domain internet
   if ((Cli_sock = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0){
      printf("[snfs_api] socket error: %s.\n", strerror(errno));
      return -1;
   }

   // reuse client address if it exists
   if (unlink(cli_name) < 0 && errno != ENOENT) {
      printf("[snfs_api] unlink error: %s.\n", strerror(errno));
   }

   // client structure address cleaning
   bzero(&Cli_addr, sizeof(Cli_addr));
   Cli_addr.sun_family = AF_UNIX;
   strcpy(Cli_addr.sun_path, cli_name);

   // binds socket to address
   if (bind(Cli_sock, (struct sockaddr *) &Cli_addr, sizeof(Cli_addr)) < 0){
      printf("[snfs_api] bind error: %s.\n", strerror(errno));
      return -1;
   }

   // server structure address cleaning
   bzero(&Serv_addr, sizeof(Serv_addr));
   Serv_addr.sun_family = AF_UNIX;

     //printf("DEBUG: serv_addr initialized to: %s\n", Serv_addr.sun_path);
   strcpy(Serv_addr.sun_path, server_name);

   // initializes serial number counter
   next_sn = 0;

   return 0;
}


snfs_call_status_t snfs_ping(char* inmsg, int insize, char* outmsg, int outsize)
{
   snfs_msg_req_t req;
   snfs_msg_res_t res;
  
   memset(&req,0,sizeof(req));
   memset(&res,0,sizeof(res));
 
   // format request
   req.type = REQ_PING;
   strncpy(req.body.ping.msg,inmsg,insize);

   int status = remote_call(&req, sizeof(req.sn) + sizeof(req.type) + sizeof(req.body.ping), 
      &res, sizeof(res), 0);

   // format response
   if (status < 0 || res.status != RES_OK) {
      return STAT_ERROR;
   }
   strncpy(outmsg,res.body.ping.msg,outsize);
   return STAT_OK;
}


/* 
   snfs_call_status_t snfs_lookup(char* pathname, ...) */

snfs_call_status_t snfs_lookup(char* pathname, 
   snfs_fhandle_t* file, unsigned* fsize)
{

	snfs_msg_req_t req;
	snfs_msg_res_t res;
	
	memset(&req,0,sizeof(req));
	memset(&res,0,sizeof(res));
	
	// format request
	req.type = REQ_LOOKUP;
	strcpy(req.body.lookup.pname, pathname);
	
	int status = remote_call(&req, sizeof(req.sn) + sizeof(req.type) + sizeof(req.body.lookup), 
					       &res, sizeof(res), 0);
	
	// format response
	if (status < 0 || res.status != RES_OK) {
		*file = res.body.lookup.file; // myopen() needs this information in case of a wrong pathname
		return STAT_ERROR;
	}
	
	*file = res.body.lookup.file;
	*fsize = res.body.lookup.fsize;
	return STAT_OK;
}


snfs_call_status_t snfs_read(snfs_fhandle_t fhandle, unsigned offset,
   unsigned count, char* buffer, int* nread)
{
	snfs_msg_req_t req;
	snfs_msg_res_t res;
	
	memset(&req,0,sizeof(req));
	memset(&res,0,sizeof(res));
	
	// format request
	req.type = REQ_READ;
	req.body.read.fhandle = fhandle;
	req.body.read.offset = offset;
	req.body.read.count = count;
	 
	int status = remote_call(&req, sizeof(req.sn) + sizeof(req.type) + sizeof(req.body.read), 
				  &res, sizeof(res), 0);
	
	// format response
	if (status < 0 || res.status != RES_OK) {
		return STAT_ERROR;
	}
	
	memcpy(buffer, res.body.read.data, count);
	*nread = res.body.read.nread;
	return STAT_OK;
}


snfs_call_status_t snfs_write(snfs_fhandle_t fhandle, unsigned offset, 
   unsigned count, char* buffer, unsigned int* fsize)
{
	snfs_msg_req_t req;
	snfs_msg_res_t res;
	
	memset(&req,0,sizeof(req));
	memset(&res,0,sizeof(res));
	
	// format request
	req.type = REQ_WRITE;
	req.body.write.fhandle = fhandle;
	req.body.write.offset = offset;
	req.body.write.count = count;
	memcpy(req.body.write.data, buffer, count);
	
	int status = remote_call(&req, sizeof(req.sn) + sizeof(req.type) + sizeof(req.body.write), 
				  &res, sizeof(res), 1);
	
	// format response
	if (status < 0 || res.status != RES_OK) {
		return STAT_ERROR;
	}
	
	*fsize = res.body.write.fsize;
	return STAT_OK;
}


snfs_call_status_t snfs_create(snfs_fhandle_t dir, char* name, 
   snfs_fhandle_t* file)
{
	snfs_msg_req_t req;
	snfs_msg_res_t res;
	
	memset(&req,0,sizeof(req));
	memset(&res,0,sizeof(res));

	// format request
	req.type = REQ_CREATE;
	req.body.create.dir = (snfs_fhandle_t)dir;
	strcpy(req.body.create.name, name);
	
	int status = remote_call(&req,sizeof(req.sn) +  sizeof(req.type) + sizeof(req.body.create), 
					       &res, sizeof(res), 1);

	// format response
	if (status < 0 || res.status != RES_OK) {
		return STAT_ERROR;
	}
	
	*file = res.body.create.file;
	return STAT_OK;
}


snfs_call_status_t snfs_mkdir(snfs_fhandle_t dir, char* name, 
   snfs_fhandle_t* file)
{
	snfs_msg_req_t req;
	snfs_msg_res_t res;
	
	memset(&req,0,sizeof(req));
	memset(&res,0,sizeof(res));
	
	// format request
	req.type = REQ_MKDIR;
	req.body.mkdir.dir = dir;
	strcpy(req.body.mkdir.file, name);
	
	int status = remote_call(&req, sizeof(req.sn) + sizeof(req.type) + sizeof(req.body.mkdir), 
				  &res, sizeof(res), 1);

	// format response
	if (status < 0 || res.status != RES_OK) {
		return STAT_ERROR;
	}
	
	*file = res.body.mkdir.newdirid;
	return STAT_OK;
}


snfs_call_status_t snfs_readdir(snfs_fhandle_t dir, unsigned cmax, 
   snfs_dir_entry_t* list, unsigned* count)
{
	snfs_msg_req_t req;
	snfs_msg_res_t res;
	
	memset(&req,0,sizeof(req));
	memset(&res,0,sizeof(res));
	
	// format request
	req.type = REQ_READDIR;
	req.body.readdir.dir = dir;
	req.body.readdir.cmax = cmax;
	
	int status = remote_call(&req, sizeof(req.sn) + sizeof(req.type) + sizeof(req.body.readdir), 
				  &res, sizeof(res), 0);

	// format response
	if (status < 0 || res.status != RES_OK) {
		return STAT_ERROR;
	}
	
	*count = res.body.readdir.count;
	memcpy(list, res.body.readdir.list, sizeof(snfs_dir_entry_t)*(*count));
	return STAT_OK;
}

snfs_call_status_t snfs_copy(char *srcpath, char *tgtpath)
{

	snfs_msg_req_t req;
	snfs_msg_res_t res;
	
	memset(&req,0,sizeof(req));
	memset(&res,0,sizeof(res));
	
	// format request
	req.type = REQ_COPY;
	strcpy(req.body.copy.srcpathname, srcpath);
	strcpy(req.body.copy.tgtpathname, tgtpath);
	
	int status = remote_call(&req, sizeof(req.sn) + sizeof(req.type) + sizeof(req.body.copy), 
					       &res, sizeof(res), 1);
	
	// format response
	if (status < 0 || res.status != RES_OK) {
		return STAT_ERROR;
	}
	
	return STAT_OK;
}

void snfs_finish()
{
   close(Cli_sock);
   unlink(Cli_addr.sun_path);
}
