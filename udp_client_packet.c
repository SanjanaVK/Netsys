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

#define MAXBUFSIZE 5*1024

/* You will have to modify the program below */

struct packet_t{
    int sequence_number;
    double time;
    char data[MAXBUFSIZE];
};

struct packet_t sender_packet, receiver_packet;

int main (int argc, char * argv[])
{

	int nbytes;                             // number of bytes send by sendto()
	int sockfd;                               //this will be our socket
	int remote_length;
	char buffer[MAXBUFSIZE];
        struct addrinfo hints, *servinfo, *p;
	struct sockaddr_in remote;              //"Internet socket address structure"
        int fd;
        char *transfer_file;
        time_t send_time, receive_time;

	if (argc < 4)
	{
		printf("USAGE:  <server_ip> <server_port> <filename>\n");
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
    transfer_file = argv[3];
    fd = open(transfer_file , O_RDONLY); //file open with read only option
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
  while(nbytes = read(fd, sender_packet.data, MAXBUFSIZE)) 
  {
      sender_packet.sequence_number = 1; // pack all the required data into one packet
//      strcpy(sender_packet.data , buffer);
      sender_packet.time = time(&send_time);
      if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1); //send the packet
      {
  //         printf("error");
           perror("talker:sendto");
//           exit(-1);
      }
    //  printf("client says :%s", sender_packet.data);
      bzero(sender_packet.data, nbytes);
      bzero(buffer, nbytes);
   //   recvfrom(sockfd, &receiver_packet, sizeof(receiver_packet), 0, (struct sockaddr *)&remote, &remote_length);
     // printf("sequence number is %d\n", receiver_packet.sequence_number);
//      time(&receive_time);
  //    if(receiver_packet.sequence_number == 1)
    //  {
      //   printf("1st Packet Ack Obtained");
        // printf(" RTT is %f\n", difftime(receive_time, send_time));
     // }
} 
              
	/*char command[] = "apple";	
	if (nbytes = sendto(sockfd, command, sizeof(command), 0,
			 (struct sockaddr *)&remote, remote_length) == -1) {
		perror("talker: sendto");
		exit(1);*/
        strcpy(sender_packet.data, command);

        sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length);

	// Blocks till bytes are received
	/*struct sockaddr_in from_addr;
	int addr_length = sizeof(struct sockaddr);
	bzero(buffer,sizeof(buffer));
	nbytes = recvfrom(sockfd,buffer,100,0,  (struct sockaddr *)&remote, &addr_length);
    

	printf("Server says %s\n", buffer);*/

	close(sockfd);

}

