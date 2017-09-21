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
/* You will have to modify the program below */

#define MAXBUFSIZE 5*1024

struct packet_t{
    int sequence_number;
    double time;
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
	struct sockaddr_in sin, remote;     //"Internet socket address structure"
	unsigned int remote_length;         //length of the sockaddr_in structure
	int nbytes;                        //number of bytes we receive in our message
	char buffer[MAXBUFSIZE];             //a buffer to store our received message
        time_t send_time, receive_time;
        
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
        
        char command[] = "End of File";

	//waits for an incoming message
	bzero(buffer,sizeof(buffer));
//	printf("socket\n");
	while(1)
        {
          
          if(recvfrom(sockfd, &receive_packet, sizeof(receive_packet),0,  (struct sockaddr *)&remote, &remote_length) <= 0)
          {
              perror("listener:");
              exit(-1);
           }
          printf("Received packet\n");
                      
      //       printf("%s", receive_packet.data);
          if(strcmp(receive_packet.command, "put") == 0)
          {
              fd = open( "testfoo3", O_RDWR|O_CREAT , 0666);
              while(1)
              {
                  if (!(strcmp(receive_packet.data, command)))
                      break;
            //printf("received packet\n");
           //if (receive_packet.sequence_number == 1)
           //{
             /*printf("Obtained first packet\n");
             sender_packet.time= time(&receive_time);
             sender_packet.sequence_number == 1;
             strcpy(sender_packet.data, receive_packet.data);*/
                  sendto(sockfd, &receive_packet, sizeof(receive_packet), 0, (struct sockaddr *)&remote, remote_length); //send the packet
                  printf("Number of bytes to write %d\n", receive_packet.datasize);
                  write(fd, receive_packet.data, receive_packet.datasize);
                  bzero(receive_packet.data, sizeof(receive_packet.data));
                  if(recvfrom(sockfd, &receive_packet, sizeof(receive_packet),0,  (struct sockaddr *)&remote, &remote_length) <= 0)
                  {
                      perror("listener:");
                      exit(-1);
                  }
              }
              printf("Received file\n");
              exit(-1);
          }
           
          if(strcmp(receive_packet.command, "get") == 0)
          {
               int fd, nbytes;
               time_t send_time, receive_time;
               char buffer[MAXBUFSIZE];
               fd = open(receive_packet.filename , O_RDONLY); //file open with read only option
               if(fd == -1)
               {
                   perror("File not opened: ");
                   exit(-1);
               }
               char command[] = "End of File";
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
                   sender_packet.time = time(&send_time);
                   sender_packet.datasize = nbytes;
                   if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1) //send the packet
                   {

                       perror("talker:sendto");
                       // continue;
                   }
                   printf("Sent packet to the client with datasize %d\n",sender_packet.datasize);
                   bzero(sender_packet.data, nbytes);
                   bzero(buffer, nbytes);

                   recvfrom(sockfd, &receive_packet, sizeof(receive_packet), 0, (struct sockaddr *)&remote, &remote_length);
                   printf("sequence number is %d\n", receive_packet.sequence_number);
                   time(&receive_time);
                   if(receive_packet.sequence_number == seq_number_count)
                   {
                       printf(" %d Packet Ack Obtained",seq_number_count);
                       printf(" RTT is %f\n", difftime(receive_time, send_time));
                   }

                   seq_number_count++;
               } 
              
               strcpy(sender_packet.data, command);

               sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length);
               bzero(sender_packet.data, sizeof(sender_packet));
               bzero(receive_packet.data, sizeof(receive_packet));
               exit(-1);
        	// Blocks till bytes are received
	       /*struct sockaddr_in from_addr;
	        int addr_length = sizeof(struct sockaddr);
	         bzero(buffer,sizeof(buffer));
           	nbytes = recvfrom(sockfd,buffer,100,0,  (struct sockaddr *)&remote, &addr_length);
    

	        printf("Server says %s\n", buffer);*/
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

