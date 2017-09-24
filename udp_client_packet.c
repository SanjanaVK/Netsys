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

void put_file(struct sockaddr_in remote, int remote_length, int sockfd, fd_set readset);
void get_file(struct sockaddr_in remote, int remote_length, int sockfd);
void delete_file(struct sockaddr_in remote, int remote_length, int sockfd);
void get_list_of_files(struct sockaddr_in remote, int remote_length, int sockfd);




/* You will have to modify the program below */

struct packet_t{
    int sequence_number;
    unsigned int datasize;
    char data[MAXBUFSIZE];
    char command[50];
    char filename[50];
    unsigned int valid;
};

struct packet_t sender_packet, receiver_packet;

void display_menu()
{
   printf("\n================================================Command Menu=====================================================================================\n");
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
        fd_set readset;
	int remote_length;
	char user_command[50];
	struct sockaddr_in remote;              //"Internet socket address structure"
       // int fd;
        char *transfer_file;
        unsigned int server_exit_flag = 0;
        
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
       printf("\n Please enter a valid command from the command menu\n");
       if(server_exit_flag == 1)
           printf("Note: Server has been exited. The command wont be succesful unless both client and server are restarted again\n");
       gets(user_input);
       printf("Command entered: %s\n", user_input);
       token = strtok(user_input, " [ ]");
       strcpy(sender_packet.command, token);
       while (token != NULL)
       {
           token = strtok(NULL, " [ ]");
           if(token != NULL)
           {
               strcpy(sender_packet.filename, token);
           }
       }
      
       if (strcmp(sender_packet.command , "get") == 0)
       {
           printf("Obtaining file %s from the server\n", sender_packet.filename);
           sender_packet.valid = 1;
           get_file(remote, remote_length, sockfd);

       }
       else if (strcmp(sender_packet.command , "put") == 0)
       {
          printf("Sending file %s to the server\n", sender_packet.filename);
          sender_packet.valid = 1;
          put_file(remote, remote_length, sockfd, readset);
       }
       else if (strcmp(sender_packet.command, "delete") == 0)
       {
          printf("Deleting File %s from the server directory\n", sender_packet.filename);
          sender_packet.valid = 1;
          delete_file(remote, remote_length, sockfd);
       }
       else if (strcmp(sender_packet.command , "ls") == 0)
       {
           sender_packet.valid = 1;
           get_list_of_files(remote, remote_length, sockfd);
       }
       else if (strcmp(sender_packet.command , "exit") == 0)
       {
           sender_packet.valid = 1;
           if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1) //send the packet
           {
               perror("talker:sendto");
           }
           recvfrom(sockfd, &receiver_packet, sizeof(receiver_packet), 0, (struct sockaddr *)&remote, &remote_length);
           if(!(strcmp(receiver_packet.command ,"exit")))
           {
               server_exit_flag = 1;
               printf("Server exited gracefully\n");
           }
       }
       else
       {
          sender_packet.valid = 0;
          if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1) //send the packet
          {
              perror("talker:sendto");
          }
          recvfrom(sockfd, &receiver_packet, sizeof(receiver_packet), 0, (struct sockaddr *)&remote, &remote_length);
          printf("%s\n",receiver_packet.data);
          
       }

   }
       
	close(sockfd);

}

void get_list_of_files(struct sockaddr_in remote, int remote_length, int sockfd)
{
   char *token;
   if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1) //send the packet
   {
       perror("talker:sendto");
          // continue;
   }
   recvfrom(sockfd, &receiver_packet, sizeof(receiver_packet), 0, (struct sockaddr *)&remote, &remote_length);
   token = strtok(receiver_packet.data, "#");
   while (token != NULL)
   {
       printf("%s\n", token);
       token = strtok(NULL, "#");
   }
   printf("Number of files is %d\n", receiver_packet.datasize);

}
void delete_file(struct sockaddr_in remote, int remote_length, int sockfd)
{
   if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1) //send the packet
   {
       perror("talker:sendto");
          // continue;
   }
}

void get_file(struct sockaddr_in remote, int remote_length, int sockfd)
{
   char command[] = "#End#of#File#";
   int fd;
   fd = open( "foo_clientend", O_RDWR|O_CREAT|O_TRUNC , 0666);
   
   int expected_sequence_number = 1;
   if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1) //send the packet
   {
       perror("talker:sendto");
          // continue;
   }
   while(1)
   {
      recvfrom(sockfd, &receiver_packet, sizeof(receiver_packet), 0, (struct sockaddr *)&remote, &remote_length);
      if(strcmp(receiver_packet.data, command) == 0)
          break;
      if(sendto(sockfd, &receiver_packet, sizeof(receiver_packet), 0, (struct sockaddr *)&remote, remote_length) == -1)
      {    perror("listener:sendto"); 
      }
      if (receiver_packet.sequence_number != expected_sequence_number)
          printf("This packet is not the expected packet. Expected packet: %d, Received packet:%d\n", expected_sequence_number, receiver_packet.sequence_number);
      else
      {
          printf("Sequence number of the packet received: %d. Number of bytes to write %d\n", receiver_packet.sequence_number, receiver_packet.datasize);
          write(fd, receiver_packet.data, receiver_packet.datasize);
          expected_sequence_number++;
      }
      bzero(receiver_packet.data, sizeof(receiver_packet.data));
      
   }
      printf("Received file\n");
      


}
void put_file(struct sockaddr_in remote, int remote_length, int sockfd, fd_set readset)
{
    int fd, nbytes;
    int result = 0;
    char buffer[MAXBUFSIZE];
    fd = open(sender_packet.filename , O_RDONLY); //file open with read only option
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
//      strcpy(sender_packet.data , buffer);
      sender_packet.datasize = nbytes;
      int resent_number =0;
      
      do
      {
         if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1) //send the packet
         {

           perror("talker:sendto");
          // continue;
         }
          resent_number++;
          printf("Sent packet to the server with sequence number %d and datasize %d\n",sender_packet.sequence_number,sender_packet.datasize);
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
             recvfrom(sockfd, &receiver_packet, sizeof(receiver_packet), 0, (struct sockaddr *)&remote, &remote_length);
             if(receiver_packet.sequence_number != seq_number_count)
             {
                 printf("sequence number of the received packet ack packet is %d but expected is %d , waiting for 10 ms .... \n", receiver_packet.sequence_number, seq_number_count);
                 
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
                     recvfrom(sockfd, &receiver_packet, sizeof(receiver_packet), 0, (struct sockaddr *)&remote, &remote_length);
                     if(receiver_packet.sequence_number == seq_number_count)
                     {
                          printf(" Ack received for %d Packet\n",seq_number_count);
                          seq_number_count++;
                          
                        
                     }
                     else 
                     {
                         printf("sequence number of the received packet ack packet is %d but expected is %d, sending again packet %d\n", receiver_packet.sequence_number, seq_number_count, seq_number_count);
                         result = 0;
                     }
                 }
               }
                 
              else if(receiver_packet.sequence_number == seq_number_count)
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
        bzero(receiver_packet.data, sizeof(receiver_packet));
	// Blocks till bytes are received
	/*struct sockaddr_in from_addr;
	int addr_length = sizeof(struct sockaddr);
	bzero(buffer,sizeof(buffer));
	nbytes = recvfrom(sockfd,buffer,100,0,  (struct sockaddr *)&remote, &addr_length);
    

	printf("Server says %s\n", buffer);*/

}

