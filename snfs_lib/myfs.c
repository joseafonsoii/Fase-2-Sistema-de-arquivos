/* 
 * File System Interface
 * 
 * myfs.c
 *
 * Implementation of the SNFS programming interface simulating the 
 * standard Unix I/O interface. This interface uses the SNFS API to
 * invoke the SNFS services in a remote server.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <myfs.h>
#include <snfs_api.h>
#include <snfs_proto.h>
#include <unistd.h>
#include "queue.h"


#ifndef SERVER_SOCK
#define SERVER_SOCK "/tmp/server.socket"
#endif

#define MAX_OPEN_FILES 10		// how many files can be open at the same time


static queue_t *Open_files_list;		// Open files list
static int Lib_initted = 0;           // Flag to test if library was initted
static int Open_files = 0;		// How many files are currently open


int mkstemp(char *template);

int myparse(char *pathname);

int my_init_lib()
{
	char CLIENT_SOCK[] = "/tmp/clientXXXXXX";
	if(mkstemp(CLIENT_SOCK) < 0) {
		printf("[my_init_lib] Unable to create client socket.\n");
		return -1;
	}
	
	if(snfs_init(CLIENT_SOCK, SERVER_SOCK) < 0) {
		printf("[my_init_lib] Unable to initialize SNFS API.\n");
		return -1;
	}
	Open_files_list = queue_create();
	Lib_initted = 1;
	
	return 0;
}


int my_open(char* name, int flags)
{
	if (!Lib_initted) {
		printf("[my_open] Library is not initialized.\n");
		return -1;
	}
	
	if(Open_files >= MAX_OPEN_FILES) {
		printf("[my_open] All slots filled.\n");
		return -1;
	}
	
        if ( myparse(name) != 0 ) {
		printf("[my_open] Malformed pathname.\n");
		return -1;
            
        }
           	

	
	snfs_fhandle_t dir, file_fh;
	unsigned fsize = 0;
	char newfilename[MAX_FILE_NAME_SIZE];
	char newdirname[MAX_PATH_NAME_SIZE];
	char fulldirname[MAX_PATH_NAME_SIZE];
	char *token;
	char *search="/";
	int i=0;
	
	memset(&newfilename,0,MAX_FILE_NAME_SIZE);
	memset(&newdirname,0,MAX_PATH_NAME_SIZE);
	memset(&fulldirname,0,MAX_PATH_NAME_SIZE);

	strcpy(fulldirname,name);
	token = strtok(fulldirname, search);


        while(token != NULL) {
             i++;
             strncpy(newfilename,token, strlen(token));

             token = strtok(NULL, search);
         } 
         if ( i > 1)
              strncpy(newdirname, name, strlen(name)-(strlen(newfilename)+1));
         else {  
            strncpy(newfilename, &name[1], (strlen(name)-1)); 
            strcpy(newdirname, name);   
          }    	

	if(newfilename == NULL) {
		printf("[my_open] Error looking for directory in server.\n");
		return -1;
	}
	
        snfs_call_status_t status = snfs_lookup(name,&file_fh,&fsize);
	 
	
	if (status != STAT_OK) {
			
	         snfs_lookup(newdirname,&dir,&fsize);        
         
                 if (i==1)  //Create a file in Root directory
	             dir = ( snfs_fhandle_t)  1;
	} 
	
	if(flags == O_CREATE && status != STAT_OK) {
		if (snfs_create(dir,newfilename,&file_fh) != STAT_OK) {
			printf("[my_open] Error creating a file in server.\n");
			return -1;
		}
	} else	if (status != STAT_OK) {
		printf("[my_open] Error opening up file.\n");
		return -1;
	}
	
	fd_t fdesc = (fd_t) malloc(sizeof(struct _file_desc));
	fdesc->fileId = file_fh;
	fdesc->size = fsize;
	fdesc->write_offset = 0;
	fdesc->read_offset = 0;
	
	queue_enqueue(Open_files_list, fdesc);
	Open_files++;
	
	return file_fh;
}

int my_read(int fileId, char* buffer, unsigned numBytes)
{

	if (!Lib_initted) {
		printf("[my_read] Library is not initialized.\n");
		return -1;
	}
	
	fd_t fdesc = queue_node_get(Open_files_list, fileId);
	if(fdesc == NULL) {
		printf("[my_read] File isn't in use. Open it first.\n");
		return -1;
	}
	
	// EoF ?
	if(fdesc->read_offset == fdesc->size)
		return 0;
	
	int nread;
	unsigned rBytes;
	int counter = 0;
	
	// If bytes to be read are greater than file size
	if(fdesc->size < ((unsigned)fdesc->read_offset) + numBytes)
		numBytes = fdesc->size - (unsigned)(fdesc->read_offset);
	
	//if data must be read using blocks
	if(numBytes > MAX_READ_DATA)
		rBytes = MAX_READ_DATA;
	else
		rBytes = numBytes;
	
	while(numBytes) {
		if(rBytes > numBytes)
			rBytes = numBytes;
		
		if (snfs_read(fileId,(unsigned)fdesc->read_offset,rBytes,&buffer[counter],&nread) != STAT_OK) {
			printf("[my_read] Error reading from file.\n");
			return -1;
		}
		fdesc->read_offset += nread;
		numBytes -= (unsigned)nread;
		counter += nread;
	}
	
	return counter;
}

int my_write(int fileId, char* buffer, unsigned numBytes)
{
	if (!Lib_initted) {
		printf("[my_write] Library is not initialized.\n");
		return -1;
	}
	
	fd_t fdesc = queue_node_get(Open_files_list, fileId);
	if(fdesc == NULL) {
		printf("[my_write] File isn't in use. Open it first.\n");
		return -1;
	}
	
	unsigned fsize;
	unsigned wBytes;
	int counter = 0;
	
	if(numBytes > MAX_WRITE_DATA)
		wBytes = MAX_WRITE_DATA;
	else
		wBytes = numBytes;
	
	while(numBytes) {
		if(wBytes > numBytes)
			wBytes = numBytes;
		
		if (snfs_write(fileId,(unsigned)fdesc->write_offset,wBytes,&buffer[counter],&fsize) != STAT_OK) {
			printf("[my_write] Error writing to file.\n");
			return -1;
		}
		fdesc->size = fsize;
		fdesc->write_offset += (int)wBytes;
		numBytes -= (unsigned)wBytes;
		counter += (int)wBytes;
	}
	
	return counter;
}

int my_close(int fileId)
{
	if (!Lib_initted) {
		printf("[my_close] Library is not initialized.\n");
		return -1;
	}
	
	fd_t temp = queue_node_remove(Open_files_list, fileId);
	if(temp == NULL) {
		printf("[my_close] File isn't in use. Open it first.\n");
		return -1;
	}
	
	free(temp);
	Open_files--;
	
	return 0;
}


int my_listdir(char* path, char **filenames, int* numFiles)
{
	if (!Lib_initted) {
		printf("[my_listdir] Library is not initialized.\n");
		return -1;
	}
	
	snfs_fhandle_t dir;
	unsigned fsize;	
	
        if ( myparse(path) != 0 ) {
       	     printf("[my_listdir] Error looking for folder in server.\n");
            return -1;
         }   
     		
	if ((strlen(path)==1) && (path[0]== '/') ) dir = ( snfs_fhandle_t)  1;
	else
          if(snfs_lookup(path, &dir, &fsize) != STAT_OK) {
	     printf("[my_listdir] Error looking for folder in server.\n");
	     return -1;
	   }
	
	
	snfs_dir_entry_t list[MAX_READDIR_ENTRIES];
	unsigned nFiles;
	char* fnames;
	
	if (snfs_readdir(dir, MAX_READDIR_ENTRIES, list, &nFiles) != STAT_OK) {
		printf("[my_listdir] Error reading directory in server.\n");
		return -1;
	}
	
	*numFiles = (int)nFiles;
	
	*filenames = fnames = (char*) malloc(sizeof(char)*((MAX_FILE_NAME_SIZE+1)*(*numFiles)));
	for (int i = 0; i < *numFiles; i++) {
		strcpy(fnames, list[i].name);
		fnames += strlen(fnames)+1;
	}
	
	return 0;
}

int my_mkdir(char* dirname) {
	if (!Lib_initted) {
		printf("[my_mkdir] Library is not initialized.\n");
		return -1;
	}

        if ( myparse(dirname) != 0 ) {
		printf("[my_mkdir] Malformed pathname.\n");
		return -1;
            
        }
	
	
	
	snfs_fhandle_t dir, newdir;
	unsigned fsize;
	char newfilename[MAX_FILE_NAME_SIZE];
	char newdirname[MAX_PATH_NAME_SIZE];
	char fulldirname[MAX_PATH_NAME_SIZE];
	char *token;
	char *search="/";
	int i=0;
	
	memset(&newfilename,0,MAX_FILE_NAME_SIZE);
	memset(&newdirname,0,MAX_PATH_NAME_SIZE);
	memset(&fulldirname,0,MAX_PATH_NAME_SIZE);
	
        if(snfs_lookup(dirname, &dir, &fsize) == STAT_OK) {
		printf("[my_mkdir] Error creating a  subdirectory that already exists.\n");
		return -1;
	}
	

	strcpy(fulldirname,dirname);
	token = strtok(fulldirname, search);


        while(token != NULL) {
             i++;
             strncpy(newfilename,token, strlen(token));
             token = strtok(NULL, search);
         } 
         if ( i > 1)
              strncpy(newdirname, dirname, strlen(dirname)-(strlen(newfilename)+1));
         else {  
            strncpy(newfilename, &dirname[1], (strlen(dirname)-1)); 
            strcpy(newdirname, dirname);   
          }    
	

	if(newdirname == NULL) {
		printf("[my_mkdir] Error looking for directory in server.\n");
		return -1;
	}
	
	
	if (i==1)  //Create a directory in Root
	     dir = ( snfs_fhandle_t)  1;
	else   //Create a directory elsewhere
	   if(snfs_lookup(newdirname, &dir, &fsize) != STAT_OK) {
			printf("[my_mkdir] Error creating a  subdirectory which has a wrong pathname.\n");
			return -1;
	   }


	if(snfs_mkdir(dir, newfilename, &newdir) != STAT_OK) {
		printf("[my_mkdir] Error creating new directory in server.\n");
		return -1;
	}
	
	return 0;
}

int myparse(char* pathname) {

	char line[MAX_PATH_NAME_SIZE]; 
	char *token;
	char *search = "/";
	int i=0;

	strcpy(line,pathname); 
	if(strlen(line) >= MAX_PATH_NAME_SIZE || (strlen(line) < 1) ) {
	   //printf("[myparse] Wrong pathname size.\n"); 
	   return -1; 
	   }
	if (strchr(line, ' ') != NULL || strstr( (const char *) line, "//") != NULL || line[0] != '/' ) {
	   //printf("[myparse] Malformed pathname.\n");
	   return -1; 
	   }


	if ((i=strlen(pathname)) && line[i]=='/') {
	   //printf("[myparse] Malformed pathname.\n");
	   return -1; 
	   }
	   	i=0;
	token = strtok(line, search);

	while(token != NULL) {
	  if ( strlen(token) > MAX_FILE_NAME_SIZE -1) { 
	      //printf("[myparse] File/Directory name size exceeded limits.:%s \n", token);
	      return -1; 
	      }
	  i++;

	  token = strtok(NULL, search);
	}

	return 0;
}

