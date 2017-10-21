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
#define MAXBUF 1024
#define FILENAME "ws.conf"

char response [] = 
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>Welcome to my SHS</title>\r\n"
//"</head><style>body {background-image: url(\"images/background.png\");}</style>\r\n"
"<body><!-- Main content --><h1>Select from the menu</h1><p>Welcome to my Simple HTTP Server<ul class=\"navbar\"></ul>\r\n"
"<img src=\"welcome.png\" alt=\"Image\" style=\"width:304px;height:228px\" align=\"center\"></body></html>\r\n";

void response_index(int connfd, char* filename);

struct config
{
   char key[100];
   char value[100][100];
};

struct config_details
{
    char serverport_number[50];
    char default_webpage[10][50];
    char document_root[100];
    char content_type[50][100];
};
struct config_details get_config(char *filename)
{
    struct config configstruct_map[100];
    struct config_details configstruct;
    char *token;
    FILE *file = fopen (filename, "r");
    if(file == NULL)
    {
        perror("File not opened :");
        exit(-1);
    }

    if (file != NULL)
    {
        char line[MAXBUF];
        int i = 0;
        int j = 0;
        int k = 0;
        int extension_index = 0;

        while(fgets(line, sizeof(line), file) != NULL)
        {
            char *cfline;
            if( strchr(line, '#') !=NULL)
            {
                j=0;
                if((token = strtok(line, "#")) != NULL)
                    strcpy(configstruct_map[i].key,token);
                    i++;
            }
            else
            {
                int temp_i = (i-1);
                strcpy(configstruct_map[temp_i].value[j],line);
                if( strstr(configstruct_map[temp_i].key, "port") != NULL)
                {
                    token = strtok(line, " ");
                    while(token != NULL)
                    {
                        if((token = strtok(NULL, " ")) != NULL)
                        { 
                            strcpy(configstruct.serverport_number, token);
                           
                        }
                        break;
                    }
                }
                else if( strstr(configstruct_map[temp_i].key, "root") != NULL)
                {
                    token = strtok(line, " ");
                    while(token != NULL)
                    {
                        if((token = strtok(NULL, " ")) != NULL)
                        { 
                            strcpy(configstruct.document_root, token);
                        }
                        break;
                             
                    }
                }
                else if( strstr(configstruct_map[temp_i].key, "default") != NULL)
                {
                    token = strtok(line, " ");
                    while(token != NULL)
                    {
                        strcpy(configstruct.default_webpage[k], token);
                        token = strtok(NULL, " ");
                        k++;
                    }
                }
                else if( strstr(configstruct_map[temp_i].key, "Content-Type") != NULL)
                {
                    token = strtok(line, "\n");
                    while(token != NULL)
                    {
                        strcpy(configstruct.content_type[extension_index], token);
                        token = strtok(NULL, "\n");
                        extension_index++;
                    }
                }

                j++;
            }
        } // End while
        fclose(file);
        printf("==========Contents of ws.conf===========\n");
        for(i = 0; i<4; i++)
        {
            printf("%s", configstruct_map[i].key);
            for(j=0; j< (sizeof(configstruct_map[i].value) / sizeof(configstruct_map[i].value[j])); j++)
            {
                printf("%s", configstruct_map[i].value[j]);
            }
        }
      /*  printf("\nServer port number is %s",configstruct.serverport_number);
        printf("Document root is %s", configstruct.document_root);
        printf("Default webpage: \n");
        for(i=0; i<4; i++)
            printf("%s\n", configstruct.default_webpage[i]);
        printf("Content type: \n"); 
        for(i=0; i<9; i++)
            printf("%s\n", configstruct.content_type[i]);*/
                    
    } // End if file
       
    return configstruct;

}

int main()
{
    int listenfd, connfd, n;
    pid_t childpid;
    socklen_t clilen;
    char buf[MAXLINE];
    struct sockaddr_in cliaddr, servaddr;
    struct config_details configstruct;
    configstruct = get_config(FILENAME);
    
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
	printf("\n%s\n","Server running...waiting for connections.");
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
                    printf("n is %d\n",n);
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
                             response_index(connfd, filename);
                                
                             printf("data sent\r\n");
                             printf("n is %d\n", n);
                        }
                    }
                        // send(connfd, response, n, 0);
                        // write(connfd, response, sizeof(response)-1);
                }
	            if (n < 0)
                    {
                        printf("%s\n", "Read error");
                        printf("n is %d\n", n);
                    }
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
    char length_header[100];
    printf("filename is %s\n", filename);
  /*  char temp_filename[100];
    strcpy(temp_filename, "./www/images");
    strcat(temp_filename, filename);
    printf("filename is %s\n", temp_filename);

    FILE* fp = fopen(temp_filename, "rb");
    if (!fp){
        perror("The file was not opened");    
       // exit(1);    
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
    fclose(fp);
    sprintf(length_header, "Content-Length: %ld\r\n\r\n", fsize);
    printf("Header is %s\n", length_header);*/


    if (strcmp(filename, "/index.html") == 0)
    {
      printf("welcome page\n");   
      int findex;
                                char index_header[] = 
                                "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/html; charset=UTF-8\r\n\r\n";
                                findex = open("./www/index.html", O_RDONLY);
                                
                               /* if(send(connfd,"HTTP/1.1 200 OK\r\n", 17,0) == -1)
                                    perror("Status: ");
                                if(send(connfd,"Content-Type: image/png; charset=UTF-8\r\n\r\n", 43, 0) == -1)
                                    perror("Content type: ");*/
                                if(write(connfd, index_header, sizeof(index_header)-1) < 0)
                                    perror("write :");
                                if(sendfile(connfd, findex, NULL, 3400) < 0)
                                    perror("sendfile: ");
                                
                                close(findex);
                                close(connfd);

   
    }

    
    else if(strcmp(filename, "/welcome.png") == 0)
    {
        int fimage;       
        char image_header[] = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: image/png; charset=UTF-8\r\n\r\n";
         strcat(image_header, length_header);
        fimage = open("./www/images/welcome.png", O_RDONLY);
        printf("image header is %s\n", image_header);
                                
        if(write(connfd, image_header, sizeof(image_header)-1) < 0)
            perror("write header :");
        //if(write(connfd, length_header, sizeof(length_header)-1) < 0)
            //perror("write length header :");

        if(sendfile(connfd, fimage, NULL, 18916) < 0)
            perror("sendfile: ");
                                
        close(fimage);
        close(connfd);

                            

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
    else
    {
        char temp_filename[100];
        strcpy(temp_filename, "./www");
        strcat(temp_filename, filename);
        printf("filename is %s\n", temp_filename);
         
                                                               int fimage;
                                char image_header[] = 
                                "HTTP/1.1 200 OK\r\n"
                                "Content-Type: image/png; charset=UTF-8\r\n\r\n";
                                fimage = open(temp_filename, O_RDONLY);
                                
                               /* if(send(connfd,"HTTP/1.1 200 OK\r\n", 17,0) == -1)
                                    perror("Status: ");
                                if(send(connfd,"Content-Type: image/png; charset=UTF-8\r\n\r\n", 43, 0) == -1)
                                    perror("Content type: ");*/
                                if(write(connfd, image_header, sizeof(image_header)-1) < 0)
                                    perror("write :");
                                if(sendfile(connfd, fimage, NULL, 400000) < 0)
                                    perror("sendfile: ");
                                //if(write(connfd, msg, sizeof(msg)) < 0)
                                  //  perror("Write content: ");
                               // printf("n is %d\n", n); 
                                close(fimage);
                                close(connfd);

    }
   
    printf("data sent\r\n");

    
}
