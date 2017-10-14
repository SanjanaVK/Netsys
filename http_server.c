//Reference: http://www.cs.dartmouth.edu/~campbell/cs50/socketprogramming.html

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stddef.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/sendfile.h>

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 8888 /*port*/
#define LISTENQ 8 /*maximum number of client connections*/

char response [] = 
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>Welcome to my SHS</title>\r\n"
//"</head><style>body {background-image: url(\"images/background.png\");}</style>\r\n"
"<body><!-- Main content --><h1>Select from the menu</h1><p>Welcome to my Simple HTTP Server<ul class=\"navbar\"></ul>\r\n"
"<img src=\"welcome.png\" alt=\"Image\" style=\"width:304px;height:228px\" align=\"center\"></body></html>\r\n";

//void response_index(int connfd, char* filename);

int main()
{
    int listenfd, connfd, n;
    pid_t childpid;
    socklen_t clilen;
    char buf[MAXLINE];
    struct sockaddr_in cliaddr, servaddr;

    //Create a socket for the soclet
    //If sockfd<0 there was an error in the creation of the socket
    if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) 
	{
        perror("Problem in creating the socket");
        exit(2);
    }
	//preparation of the socket address
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
	
	//bind the socket
	if(bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
            printf("Unable to bind");
	//listen to the socket by creating a connection queue, then wait for clients
	listen (listenfd, LISTENQ);
	printf("%s\n","Server running...waiting for connections.");
	for(; ;)
	{
	    clilen = sizeof(cliaddr);
	    //accept a connection
	    connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);
	    printf("%s\n","Received request...");
	    if ( (childpid = fork ()) == 0 ) 
	    {
	        //if it’s 0, it’s child process
		printf ("%s\n","Child created for dealing with client requests");
		//close listening socket
		close (listenfd);
	        while ( (n = recv(connfd, buf, MAXLINE,0)) > 0)  
	        {
	            printf("%s","String received from:");
                    puts(buf);
                    char * token = NULL;
                    token = strtok(buf, " ");
                    puts(token);
                    if((strcmp(token , "GET")) ==0)
                    {
                       // while (token != NULL)
                        //{
                         token = strtok(NULL, " ");
                         if(token != NULL)
                         {
                             puts(token);
                             long fsize;
                             char filename[100];
                             strcpy(filename, token);
                             printf("filename is %s\n", filename);
                             if (strcmp(filename, "/index.html") == 0)
                             {
                                 printf("welcome page\n");   
                                 FILE * fp = fopen("./www/files/text1.txt", "rb");
                                 if (!fp){
                                     perror("The file was not opened");    
                                     exit(1);    
                                } 
                                printf("The file was opened\n");

                                if (fseek(fp, 0, SEEK_END) == -1)
                                {
                                    perror("The file was not seeked");
                                    exit(1);
                                }

                                fsize = ftell(fp);
                                if (fsize == -1) {
                                perror("The file size was not retrieved");
                                exit(1);
                                }
                                fseek(fp, 0, SEEK_SET);
                                char msg[fsize+1];

    
                                if (fread(msg, fsize, 1, fp) != 1){
                                    perror("The file was not read\n");
                                    exit(1);
                                }
                                fclose(fp);
                                //write(connfd, response, sizeof(response)-1);
                                char status[] = "HTTP/1.1 200 OK\r\n";
                                char contenttype[] = "Content-Type: text/html; charset=UTF-8\r\n\r\n";
                                //strcat(status,contenttype);
                                //strcat(status,msg);
                                send(connfd,"HTTP/1.1 200 OK\r\n", 17,0);
                                send(connfd,"Content-Type: image/html; charset=UTF-8\r\n\r\n", 44, 0);

                                write(connfd, msg, sizeof(msg));
                            }

    
                            else if(strcmp(filename, "/welcome.png") == 0)
                            {
                               /* FILE * fp = fopen("./www/images/welcome.png", "rb");
                                if (!fp){
                                     perror("The file was not opened");    
                                     exit(1);    
                                } 
                                printf("The file was opened\n");

                                if (fseek(fp, 0, SEEK_END) == -1)
                                {
                                    perror("The file was not seeked");
                                    exit(1);
                                }

                                fsize = ftell(fp);
                                if (fsize == -1) {
                                perror("The file size was not retrieved");
                                exit(1);
                                }
                                fseek(fp, 0, SEEK_SET);
                                char msg[fsize+1];

    
                                if (fread(msg, fsize, 1, fp) != 1){
                                    perror("The file was not read\n");
                                    exit(1);
                                }
                                fclose(fp);*/
                                //write(connfd, response, sizeof(response)-1);
                                int fimage;
                                char image_header[] = 
                                "HTTP/1.1 200 OK\r\n"
                                "Content-Type: image/png; charset=UTF-8\r\n\r\n";
                                fimage = open("./www/images/welcome.png", O_RDONLY);
                                
                               /* if(send(connfd,"HTTP/1.1 200 OK\r\n", 17,0) == -1)
                                    perror("Status: ");
                                if(send(connfd,"Content-Type: image/png; charset=UTF-8\r\n\r\n", 43, 0) == -1)
                                    perror("Content type: ");*/
                                if(write(connfd, image_header, sizeof(image_header)-1) < 0)
                                    perror("write :");
                                if(sendfile(connfd, fimage, NULL, 18916) < 0)
                                    perror("sendfile: ");
                                //if(write(connfd, msg, sizeof(msg)) < 0)
                                  //  perror("Write content: "); 
                                close(fimage);
                                close(connfd);

                            }
                            else if(strcmp(filename, "/images/background.png") == 0)
                            {
                                FILE* fp = fopen("./www/images/background.png", "rb");
                                if (!fp){
                                     perror("The file was not opened");    
                                     exit(1);    
                                } 
                                printf("The file was opened\n");

                                if (fseek(fp, 0, SEEK_END) == -1)
                                {
                                    perror("The file was not seeked");
                                    exit(1);
                                }

                                fsize = ftell(fp);
                                if (fsize == -1) {
                                perror("The file size was not retrieved");
                                exit(1);
                                }
                                fseek(fp, 0, SEEK_SET);
                                char msg[fsize+1];

    
                                if (fread(msg, fsize, 1, fp) != 1){
                                    perror("The file was not read\n");
                                    exit(1);
                                }
                                fclose(fp);
                                write(connfd, response, sizeof(response)-1);
                                write(connfd,"HTTP/1.1 200 OK\r\n", 16);
                                write(connfd,"Content-Type: text/html; charset=UTF-8\r\n\r\n", 43);
                                write(connfd, &msg, fsize);

                            }
                                printf("data sent\r\n");
                        }
                    }
                        // send(connfd, response, n, 0);
                        // write(connfd, response, sizeof(response)-1);
                }
	            if (n < 0)
                        printf("%s\n", "Read error");
                    close(connfd);
                    printf("Closing socket.........\n");
                    exit(0);
                }  
                //close socket of the server
                close(connfd);
                printf("socket closed\n");
            }
}

void response_index(int connfd, char * filename)
{
    long fsize;
    printf("filename is %s\n", filename);
    if (strcmp(filename, "./index.html") == 0)
    {
      printf("welcome page\n");   
      FILE * fp = fopen("./www/files/welcome.html", "rb");
      if (!fp){
        perror("The file was not opened");    
        exit(1);    
        }
    printf("The file was opened\n");

    if (fseek(fp, 0, SEEK_END) == -1)
    {
        perror("The file was not seeked");
      //  exit(1);
    }

    fsize = ftell(fp);
    if (fsize == -1) {
        perror("The file size was not retrieved");
        exit(1);
    }
    fseek(fp, 0, SEEK_SET);
    char msg[fsize+1];

    
    if (fread(msg, fsize, 1, fp) != 1){
        perror("The file was not read\n");
        exit(1);
    }
    fclose(fp);
    write(connfd, response, sizeof(response)-1);
    // write(connfd,"HTTP/1.1 200 OK\r\n", 16);
   // write(connfd,"Content-Type: text/html; charset=UTF-8\r\n\r\n", 43);
    //write(connfd, &msg, fsize);

    }

    
    else if(strcmp(filename, "./www/welcome.png") == 0)
    {
        FILE * fp = fopen("./www/images/welcome.png", "rb");
             if (!fp){
        perror("The file was not opened");    
        exit(1);    
        }
    printf("The file was opened\n");

    if (fseek(fp, 0, SEEK_END) == -1)
    {
        perror("The file was not seeked");
      //  exit(1);
    }

    fsize = ftell(fp);
    if (fsize == -1) {
        perror("The file size was not retrieved");
        exit(1);
    }
    fseek(fp, 0, SEEK_SET);

    char msg[fsize+1];

    if (fread(msg, fsize, 1, fp) != 1){
        perror("The file was not read\n");
        exit(1);
    }
    
    fclose(fp);
     write(connfd,"HTTP/1.1 200 OK\r\n", 16);
    write(connfd,"Content-Type: text/html; charset=UTF-8\r\n\r\n", 43);
    write(connfd, &msg, fsize);


    }
    else if(strcmp(filename, "./www/images/background.png") == 0)
    {
        FILE* fp = fopen("./www/images/background.png", "rb");
               if (!fp){
        perror("The file was not opened");    
        exit(1);    
        }
    printf("The file was opened\n");

    if (fseek(fp, 0, SEEK_END) == -1)
    {
        perror("The file was not seeked");
      //  exit(1);
    }

    fsize = ftell(fp);
    if (fsize == -1) {
        perror("The file size was not retrieved");
        exit(1);
    }
    fseek(fp, 0, SEEK_SET);

    char msg[fsize+1];

    if (fread(msg, fsize, 1, fp) != 1){
        perror("The file was not read\n");
        exit(1);
    }
    fclose(fp);
     write(connfd,"HTTP/1.1 200 OK\r\n", 16);
    write(connfd,"Content-Type: text/html; charset=UTF-8\r\n\r\n", 43);
    write(connfd, &msg, fsize);


    }

   
    printf("data sent\r\n");

    
}
