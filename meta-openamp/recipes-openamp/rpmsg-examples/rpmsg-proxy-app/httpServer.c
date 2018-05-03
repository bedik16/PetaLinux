#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <time.h>

#include "httpServer.h"
#include "httpServerRemoteFunc.h"


#define PORT 5000  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold

int handleRequest (int sock);
int serverRunning = 0;


int httpServerThread(void)  {
    
    int sockfd, newsockfd, portno, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0) {
       fprintf(stderr,"ERROR opening socket");
       printf(stderr,"ERROR opening socket");
       return -1;
    }
    
    /* Use SO_REUSEADDR, which tells the kernel that even if this port is busy (in
       the TIME_WAIT state), go ahead and reuse it anyway. 
     */ 
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
         fprintf(stderr,"setsockopt(SO_REUSEADDR) failed");

    // Fill serv_addr with 0s
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    portno = (int)PORT;
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { 
        fprintf(stderr,"ERROR on binding");
        return -1;
    }
    
    listen(sockfd,5);
    clilen = sizeof(cli_addr);

    printf("Waiting for incoming connections\n");
    fflush( stdout );
    
    serverRunning = 1;
   // while(serverRunning) {
    while(1) {
            
        if ( ( newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen) ) < 0 )
            fprintf(stderr,"ERROR on accept");
       // pid = fork();
       // if (pid < 0)
       //     fprintf(stderr,"ERROR on fork");
       // if (pid == 0)  {
       //     close(sockfd);
            handleRequest(newsockfd);
       //     exit(0);
       // }
       // else 
            close(newsockfd);
    } /* end of while */
    printf("Exiting\n");
    close(sockfd);
    return 0; /* we never get here */
}

void startHttpServer(void) {
  printf("SERVER IS BEING STARTED\n");
  httpServerThread();
    
//    pthread_t serverThread;
//    if(serverRunning) {
//        fprintf(stderr,"Server running already");
//        return -1;
//    } else
//        pthread_create(&serverThread, NULL, httpServerThread, 0);
//    return 1;
}

uint8_t stopHttpServer(void) {
    if(serverRunning==0) {
        fprintf(stderr,"Server stopped already");
        return -1;
    }
    else {
       serverRunning=1;
    }
    return 0;
}

void sendReply(char *buf, int tcp_socket) {
    int tobewritten = strlen(buf);
    int total_bytes_written = 0;
    while (total_bytes_written != tobewritten)
    {
        assert(total_bytes_written < tobewritten);
        ssize_t bytes_written = write(tcp_socket, &buf[total_bytes_written], tobewritten - total_bytes_written);
        if (bytes_written == -1)
        {
            /* Report failure and exit. */
            break;
        }
        total_bytes_written += bytes_written;
    }
}


int Output_HTTP_Headers(int sock, char *responseMsg) {

    char buffer[256];
    ssize_t bytestobewritten = 0;
    ssize_t total_bytes_written = 0;

    sprintf(buffer, "HTTP/1.1 200 OK\r\nServer: Flex-E-Charge\r\n"
            "Last-Modified: Fri, 13 Apr 2018 15:58:33 GMT\r\n"
            "Content-Length: %d\r\n"
            "Content-Type: text/html\r\n"
            "Connection: Closed\r\n\r\n"
            "%s",strlen(responseMsg),responseMsg);
   
    bytestobewritten = strlen(buffer);
    while (total_bytes_written != bytestobewritten)
    {
        assert(total_bytes_written < bytestobewritten);
        ssize_t bytes_written = write(sock,
                                      &buffer[total_bytes_written],
                                      bytestobewritten - total_bytes_written);
        if (bytes_written == -1)
        {
            fprintf(stderr,"ERROR writing to socket");
            break;
        }
        total_bytes_written += bytes_written;
    }
    return 0;
}


int handleRequest (int sock)
{
   int n;
   char buffer[256];
   char response[256];
   char  *end, *start;
   char  *path=NULL;
   char  *command=NULL;
   char  *arg=NULL;
   

   size_t  len;   
   
   bzero(buffer,256);

   n = read(sock,buffer,255);
   
   if (n < 0) { 
       fprintf(stderr,"ERROR reading from socket");
       printf(stderr,"ERROR reading from socket");
       return -1;
   }
   
   start = buffer;
   end=start;
   
   //Parse request
    if(!strncmp("GET ", start, 4))
        start+=4;

    else if(!strncmp("POST ", start, 5))
        start+=5;
        
    else {
        fprintf(stderr, "Unknown command.\n");
        sprintf(response, "FAIL\r\nUnknown command.\n");
        printf(response, "FAIL\r\nUnknown command.\n");
        goto sendResponse;
    }
        /* From the start position, set the end pointer to the first white-space character found in the string. */
   
   //check if there is a path
   if(*start!='/') {
       sprintf(response, "FAIL\r\nCommand missing or incorrect formatted.\n");
       printf(response, "FAIL\r\nCommand missing or incorrect formatted.\n");
       goto sendResponse;
   }
   
   end = start+1;
   while(*end && !isspace(*end))
      ++end;
   
   start++;     //Get rid of starting '/'
   /* Calculate the path length, and allocate sufficient memory for the path plus string termination. */
   len = (end - start);
   path = malloc(len + 1);
   if(NULL == path)
   {
       fprintf(stderr, "malloc() failed. \n");
       free(path);
       return -1;
   }

   /* Copy the path string to the path storage. */
   memcpy(path, start, len);
   
   /* Terminate the string. */
   path[len] = '\0';
   
   /* Use buffer for the HTTP response */
   bzero(buffer,256);
   
   if(!strncmp("get", path, 3)) {
       /* 
        * Parsing the getXX command ...
        */
       
       end = path+3;
       while(*end && !isspace(*end)) 
            ++end;
       
       len = (end - path);               // Calculate the command string length;
       command = malloc(len+1);          // Allocate memory for the command string plus the \0 termination
       memcpy(command, path, len);       // Copy the command string to memory.
       command[len] = '\0';              // Terminate the command string.
       sprintf(response,"%s\n", commandInterpreter(command, arg));
       free(command);
       
   }
   else if(!strncmp("set", path, 3)) {
       /* 
        * Parsing the setXX command ...
        */
       
       end = path+3;
       while(*end && *end!='/' && !isspace(*end)) 
            ++end;
       
       len = (end - path);               // Calculate the command string length;
       command = malloc(len+1);          // Allocate memory for the command string plus the \0 termination
       memcpy(command, path, len);       // Copy the command string to memory.
       command[len] = '\0';              // Terminate the command string.
       
        /* 
        * Now get the argument ...
        */
       if(*end != '/') {
            sprintf(response, "FAIL\r\nArgument missing or incorrect.\n");
            printf(response, "FAIL\r\nArgument missing or incorrect.\n");
            free(command);
            free(path);
            goto sendResponse;
       }
       end++;
       start = end;
       while(*end && !isspace(*end))
            ++end;
       
       len = (end - start);
       
       if(len<=0){
           sprintf(response, "FAIL\r\nArgument missing or incorrect. Argument length is %d.\n",len-1);
           printf(response, "FAIL\r\nArgument missing or incorrect. Argument length is %d.\n",len-1);
           free(command);
           free(path);
           goto sendResponse;
       }
           
           
       arg = malloc(len+1);
       memcpy(arg, start, len);
       arg[len] = '\0';
       
       sprintf(response,"%s\n", commandInterpreter(command, arg));
       printf(response,"%s\n", commandInterpreter(command, arg));
       free(command);
       free(arg);
   }

   else  {
       sprintf(response, "ERROR: Can't parse set/get command, or command unknown.\n");
       printf(response, "ERROR: Can't parse set/get command, or command unknown.\n");
       goto sendResponse;
   }
   free(path);

   sendResponse:  
   
   Output_HTTP_Headers(sock, response);
    
}
