#include <time.h>
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
#include <errno.h>
#include <string.h>


#define MAXBUFSIZE 5*1024

void put_file(struct sockaddr_in remote, int remote_length, int sockfd);

/* You will have to modify the program below */

struct packet_t{
    int sequence_number;
    double time;
    unsigned int datasize;
    char data[MAXBUFSIZE];
    char command[50];
    char filename[50];
    unsigned int valid;
};

struct packet_t sender_packet, receiver_packet;

void display_menu()
{
   printf("================================================Command Menu=====================================================================================\n");
   printf("\"get [file_name]\"       : The server transmits the requested file to the client \n");
   printf("\"put [file_name]\"       : The server receives transmitted file from the server and stores it locally \n");
   printf("\"delete [file_name]\"    : The server deletes the file if exists otherwise does nothing \n");
   printf("\"ls\"                    : The server searches for all files in its directory and sends the list of all the files to the client \n");
   printf("\"exit\"                  : The server exits gracefully \n");
}


int main (int argc, char * argv[])
{

	//int nbytes;                             // number of bytes send by sendto()
	int sockfd;                               //this will be our socket
	int remote_length;
	char user_command[50];
	struct sockaddr_in remote;              //"Internet socket address structure"
       // int fd;
        char *transfer_file;
        
	if (argc < 3)
	{
		printf("USAGE:  <server_ip> <server_port>\n");
		exit(1);
	}
        

        
    // create socket in client side
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
 
 
  if(sockfd==-1)
    {
      printf(" socket not created in client\n");
      exit(0);
    }
  else
    {
      printf("socket created in  client\n");
    }
 
	/******************
	  Here we populate a sockaddr_in struct with
	  information regarding where we'd like to send our packet 
	  i.e the Server.
	 ******************/
	bzero(&remote,sizeof(remote));               //zero the struct
	remote.sin_family = AF_INET;                 //address family
	remote.sin_port = htons(atoi(argv[2]));      //sets port to network byte order
	remote.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address

    remote_length = sizeof(remote);

    char user_input[100];
    char *token;
    while(1)
    {

       display_menu();
       printf("\n Please enter a valid command\n");
       gets(user_input);
       printf("User input is %s\n", user_input);
       token = strtok(user_input, " [ ]");
       strcpy(sender_packet.command, token);
       while (token != NULL)
       {
           printf("%s\n", token);
           token = strtok(NULL, " [ ]");
           if(token != NULL)
           {
               strcpy(sender_packet.filename, token);
           }
       }
      
       printf("Command: %s\n", sender_packet.command);
       printf("before put");

       if (sender_packet.command == "get")
       {
           printf("Filename is %s\n", sender_packet.filename);
           sender_packet.valid = 1;
       }
       else if (strcmp(sender_packet.command , "put") == 0)
       {
          printf(" In put\n");
          printf("Filename is %s\n", sender_packet.filename);
          sender_packet.valid = 1;
          put_file(remote, remote_length, sockfd);
       }
       else if (sender_packet.command == "delete")
       {
          printf("Filename is %s\n", sender_packet.filename);
          sender_packet.valid = 1;
       }
       else if (sender_packet.command == "ls")
       {
           sender_packet.valid = 1;
       }
       else if (sender_packet.command == "exit")
       {
           sender_packet.valid = 1;
       }
       else
       {
          sender_packet.valid = 0;
       }
       exit(-1);
   }
       
	close(sockfd);

}


void put_file(struct sockaddr_in remote, int remote_length, int sockfd)
{
    int fd, nbytes;
    time_t send_time, receive_time;
    char buffer[MAXBUFSIZE];
    fd = open(sender_packet.filename , O_RDONLY); //file open with read only option
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
//      strcpy(sender_packet.data , buffer);
      sender_packet.time = time(&send_time);
      sender_packet.datasize = nbytes;
      if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1); //send the packet
      {

           perror("talker:sendto");
          // continue;
      }
      printf("Sent packet to the server with datasize %d\n",sender_packet.datasize);
      bzero(sender_packet.data, nbytes);
      bzero(buffer, nbytes);

      recvfrom(sockfd, &receiver_packet, sizeof(receiver_packet), 0, (struct sockaddr *)&remote, &remote_length);
      printf("sequence number is %d\n", receiver_packet.sequence_number);
      time(&receive_time);
      if(receiver_packet.sequence_number == seq_number_count)
      {
         printf(" %d Packet Ack Obtained",seq_number_count);
         printf(" RTT is %f\n", difftime(receive_time, send_time));
      }

      seq_number_count++;
} 
              
        strcpy(sender_packet.data, command);

        sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length);

	// Blocks till bytes are received
	/*struct sockaddr_in from_addr;
	int addr_length = sizeof(struct sockaddr);
	bzero(buffer,sizeof(buffer));
	nbytes = recvfrom(sockfd,buffer,100,0,  (struct sockaddr *)&remote, &addr_length);
    

	printf("Server says %s\n", buffer);*/

}

