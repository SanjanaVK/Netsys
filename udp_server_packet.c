#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <dirent.h>

/* You will have to modify the program below */

#define MAXBUFSIZE 5*1024

struct packet_t{
    int sequence_number;
    unsigned int datasize;
    char data[MAXBUFSIZE];
    char command[50];
    char filename[50];
    unsigned int valid;
};
struct packet_t sender_packet, receive_packet;

int main (int argc, char * argv[] )
{


	int sockfd ,fd;                           //This will be our socket
        fd_set readset;
	struct sockaddr_in sin, remote;     //"Internet socket address structure"
	unsigned int remote_length;         //length of the sockaddr_in structure
	int nbytes;                        //number of bytes we receive in our message
	char buffer[MAXBUFSIZE];             //a buffer to store our received message
       
              
 	if (argc != 2)
	{
		printf ("USAGE:  <port>\n");
		exit(1);
	}

	/******************
	  This code populates the sockaddr_in struct with
	  the information about our socket
	 ******************/
	bzero(&sin,sizeof(sin));                    //zero the struct
	sin.sin_family = AF_INET;                   //address family
	sin.sin_port = htons(atoi(argv[1]));        //htons() sets the port # to network byte order
	sin.sin_addr.s_addr = INADDR_ANY;           //supplies the IP address of the local machine

// create socket in client side
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
 
  if(sockfd==-1)
    {
      printf(" socket not created in server\n");
      exit(0);
    }
  else
    {
      printf("socket created in server\n");
    }
 

	/******************
	  Once we've created a socket, we must bind that socket to the 
	  local address and port we've supplied in the sockaddr_in struct
	 ******************/
	if (bind(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		printf("unable to bind socket\n");
	}

	remote_length = sizeof(remote);
        
        char command[] = "#End#of#File#";

	//waits for an incoming message
	bzero(buffer,sizeof(buffer));
//	printf("socket\n");
	while(1)
        {
          printf("\nServer Listening.... Waiting for a command.......\n");
          
          if(recvfrom(sockfd, &receive_packet, sizeof(receive_packet),0,  (struct sockaddr *)&remote, &remote_length) <= 0)
          {
              perror("listener:");
              exit(-1);
           }
          printf("Received command: %s\n",receive_packet.command);
                      
      //       printf("%s", receive_packet.data);
          if(strcmp(receive_packet.command, "put") == 0)
          {
              unsigned int expected_sequence_number = 1;
              fd = open( "foo_serverend", O_RDWR|O_CREAT|O_TRUNC , 0666);
              while(1)
              {
                  if (!(strcmp(receive_packet.data, command)))
                  break;
            //printf("received packet\n");
           //if (receive_packet.sequence_number == 1)
           //{
             /*printf("Obtained first packet\n");
             
             sender_packet.sequence_number == 1;
             strcpy(sender_packet.data, receive_packet.data);*/
                  sendto(sockfd, &receive_packet, sizeof(receive_packet), 0, (struct sockaddr *)&remote, remote_length); //send the packet
                  if (receive_packet.sequence_number != expected_sequence_number)
                         printf("This packet is not the expected packet. Expected->%d Received->%d\n",expected_sequence_number, receive_packet.sequence_number);
                  else
                  {
                  printf("Sequence number of the packet received %d Number of bytes to write %d\n", receive_packet.sequence_number, receive_packet.datasize);
                  write(fd, receive_packet.data, receive_packet.datasize);
                  expected_sequence_number++;
                  }
                  bzero(receive_packet.data, sizeof(receive_packet.data));
                  if(recvfrom(sockfd, &receive_packet, sizeof(receive_packet),0,  (struct sockaddr *)&remote, &remote_length) <= 0)
                  {
                      perror("listener:");
                      exit(-1);
                  }
              }
              printf("Received file\n");
              
          }
           
          else if(strcmp(receive_packet.command, "get") == 0)
          {
               int fd, nbytes;
               char buffer[MAXBUFSIZE];
               int result = 0;
               fd = open(receive_packet.filename , O_RDONLY); //file open with read only option
               if(fd == -1)
               {
                   perror("File not opened: ");
                   exit(-1);
               }
               char command[] = "#End#of#File#";
	       /******************
	       sendto() sends immediately.  
	         it will report an error if the message fails to leave the computer
	          however, with UDP, there is no error if the message is lost in the network once it leaves the computer.
	              ******************/
               int seq_number_count = 1;
               while(nbytes = read(fd, sender_packet.data, MAXBUFSIZE)) 
               {
                   sender_packet.sequence_number = seq_number_count; // pack all the required data into one packet
                    //      strcpy(sender_packet.data , buffer) ;
                   sender_packet.datasize = nbytes;
                    int resent_number =0;
                do
                {
                   if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1) //send the packet
                   {

                       perror("talker:sendto");
                       // continue;
                   }
                   printf("Sent packet to the client with sequence number %d and datasize %d\n", sender_packet.sequence_number, sender_packet.datasize);
                   bzero(sender_packet.data, nbytes);
                   bzero(buffer, nbytes);
                  FD_ZERO(&readset);
                  FD_SET(sockfd, &readset);
                  struct timeval timeout = {0,100000};
                  result = select(sockfd+1, &readset, NULL, NULL, &timeout);
                  if( result == -1)
                  {
                      printf("Error in select, try again");
                      exit(-1);
                  }
                  else if( result == 0)
                  {
                      printf("Time out, Sending the packet with sequence number %d again\n",sender_packet.sequence_number);
                  }
                  
                  else if (result > 0 && FD_ISSET(sockfd, &readset))
                  {
                      recvfrom(sockfd, &receive_packet, sizeof(receive_packet), 0, (struct sockaddr *)&remote, &remote_length);
                      if(receive_packet.sequence_number != seq_number_count)
                      {
                          printf("sequence number of the received packet ack packet is %d but expected is %d , waiting for 10 ms .... \n", receive_packet.sequence_number, seq_number_count);
                 
                          FD_ZERO(&readset);
                          FD_SET(sockfd, &readset);
                          struct timeval timeout_delayed_ack = {0,10000};
                          result = select(sockfd+1, &readset, NULL, NULL, &timeout_delayed_ack);
                          if(result ==0)
                          {
                              printf("Time out after receiving false/ delayed ack, Sending packet with sequence number %d again\n", seq_number_count);
                          }
                          else if (result > 0 && FD_ISSET(sockfd, &readset))
                          {
                              recvfrom(sockfd, &receive_packet, sizeof(receive_packet), 0, (struct sockaddr *)&remote, &remote_length);
                              if(receive_packet.sequence_number == seq_number_count)
                              {
                                  printf(" Ack received for %d Packet\n",seq_number_count);
                                  seq_number_count++;
                              }
                              else 
                              {
                                  printf("sequence number of the received packet ack packet is %d but expected is %d, sending again packet %d\n", receive_packet.sequence_number, seq_number_count, seq_number_count);
                                  result = 0;
                              }
                          }
                      }
                 
                      else if(receive_packet.sequence_number == seq_number_count)
                      {
                          printf(" Ack received for %d Packet\n",seq_number_count);
                          seq_number_count++;
                      } 
                  }
              }while (result == 0 && resent_number < 6);
              if(resent_number >=6)
              {
                  printf("Not able to transfer this file, please try again later\n");
                  break;
              }
           }
      
              
               strcpy(sender_packet.data, command);

               sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length);
               bzero(sender_packet.data, sizeof(sender_packet));
               bzero(receive_packet.data, sizeof(receive_packet));
               
        	// Blocks till bytes are received
	       /*struct sockaddr_in from_addr;
	        int addr_length = sizeof(struct sockaddr);
	         bzero(buffer,sizeof(buffer));
           	nbytes = recvfrom(sockfd,buffer,100,0,  (struct sockaddr *)&remote, &addr_length);
    

	        printf("Server says %s\n", buffer);*/
          }
  
          else if(strcmp(receive_packet.command, "delete") == 0)
          {
              if(remove(receive_packet.filename) ==  0)
                   printf("Delele successful of file %s\n", receive_packet.filename);
              else
                   printf("Error: Delete Unsuccesful\n");
              
          }
          else if(strcmp(receive_packet.command, "ls") == 0)
          {
             DIR *d;
             struct dirent *dir;
             int file_count  = 0;
             d = opendir(".");
             bzero(sender_packet.data, sizeof(sender_packet));

             if(d)
             {
                 if( (dir = readdir(d)) != NULL)
                     strcpy(sender_packet.data,dir->d_name);
                     file_count++;
                 while( (dir =readdir(d)) != NULL)
                 {
                      printf("%s\n", dir->d_name);
                      strcat(sender_packet.data, "#");
                      strcat(sender_packet.data,dir->d_name);
                      file_count++;
                      
                      
                 }
             }
             sender_packet.datasize = file_count;
             closedir(d);
             printf("Number files in server directory is %d\n", file_count);
             if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1)
                 perror("sendto:");
             
          }

          else if(strcmp(receive_packet.command, "exit") == 0)
          {
             printf("Server is exiting\n");
             strcpy(sender_packet.command, "exit");
             if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1)
                 perror("sendto:");
             exit(1);
          }

          else if(receive_packet.valid == 0)
          {
               strcpy(sender_packet.data, "Server says, Please enter a valid command\n");
               if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1)
                 perror("sendto:");
          }
       
             
}
        



	/*printf("The client says %s\n", buffer);

	char msg[] = "orange";
		if (nbytes = sendto(sockfd, msg, sizeof(msg), 0,
			 (struct sockaddr *)&remote, remote_length) == -1) {
		perror("listener: sendto");
		exit(1);
	}*/

	close(sockfd);
}

