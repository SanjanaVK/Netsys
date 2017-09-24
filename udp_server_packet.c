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

/* MAXBUFSIZE is number of bytes that can be read from a file at an instant */

#define MAXBUFSIZE 5*1024

void get_file(struct sockaddr_in remote, int remote_length, int sockfd, fd_set readset);
void put_file(struct sockaddr_in remote, int remote_length, int sockfd);
void get_list_of_files(struct sockaddr_in remote, int remote_length, int sockfd);

/*Create struct packet to transfer file and acks*/
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
    int sockfd ;                          //This will be our socket
    fd_set readset;
	struct sockaddr_in sin, remote;     //"Internet socket address structure"
	unsigned int remote_length;         //length of the sockaddr_in structure
	int nbytes;                        //number of bytes we receive in our message
	char buffer[MAXBUFSIZE];             //a buffer to store our received message
       
    //Check for arguments. Should provide port number          
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

    // create socket in server side
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
	bzero(buffer,sizeof(buffer));

	while(1)
    {
        printf("\nServer Listening.... Waiting for a command.......\n");
          
        if(recvfrom(sockfd, &receive_packet, sizeof(receive_packet),0,  (struct sockaddr *)&remote, &remote_length) <= 0) //receive command from client
        {
            perror("listener:");
            exit(-1);
        }
        printf("Received command: %s\n",receive_packet.command);
        
        if(strcmp(receive_packet.command, "put") == 0)
        {
            put_file(remote, remote_length, sockfd); //If command is put, then client puts file into server
        }
           
        else if(strcmp(receive_packet.command, "get") == 0)
        {
            get_file(remote, remote_length, sockfd, readset); //If command is get, then client gets a file from server
        }
        else if(strcmp(receive_packet.command, "delete") == 0) //If command is delete, then server deletes particular file
        {
            if(remove(receive_packet.filename) ==  0) //If file present then delete file
                printf("Delele successful of file %s\n", receive_packet.filename);
            else
                printf("Error: Delete Unsuccesful\n");
              
        }
        else if(strcmp(receive_packet.command, "ls") == 0)
        {
            get_list_of_files(remote, remote_length, sockfd); //If command is ls then get list of all files in server directory
                        
        }

        else if(strcmp(receive_packet.command, "exit") == 0) //If command is exit, server exits
        {
            printf("Server is exiting\n");
            strcpy(sender_packet.command, "exit");
            if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1) //send to client that server exited
                perror("sendto:");
            exit(1);
        }

        else if(receive_packet.valid == 0) //If invalid command then tell client that server does not understand the command
        {
            strcpy(sender_packet.data, "Server says, Please enter a valid command\n");
            if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1) //send invalid command alert to client
                perror("sendto:");
        }
    }
        
    close(sockfd); // close the socket
}

/*This function gets list of all files from server*/ 
void get_list_of_files(struct sockaddr_in remote, int remote_length, int sockfd)
{
    DIR *d;
    struct dirent *dir;
    int file_count  = 0;
    d = opendir(".");
    bzero(sender_packet.data, sizeof(sender_packet));
    if(d)
    {
        if( (dir = readdir(d)) != NULL) //read current directory
        strcpy(sender_packet.data,dir->d_name);
        file_count++;
        while( (dir =readdir(d)) != NULL)
        {
            printf("%s\n", dir->d_name);
            //get a string of all filenames separated by "#"
            strcat(sender_packet.data, "#");
            strcat(sender_packet.data,dir->d_name);
            file_count++;
        }
    }
    sender_packet.datasize = file_count;
    closedir(d);
    printf("Number files in server directory is %d\n", file_count);
    if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1) //send list of files to client
        perror("sendto:");   
}

/*This function puts a particular file from client to server*/
void put_file(struct sockaddr_in remote, int remote_length, int sockfd)
{
    int fd;
    unsigned int expected_sequence_number = 1;
    char command[] = "#End#of#File#";
 
    fd = open( "foo_serverend", O_RDWR|O_CREAT|O_TRUNC , 0666);
    while(1)
    {
        if (!(strcmp(receive_packet.data, command))) //check if file transfer is complete
            break;
           
        sendto(sockfd, &receive_packet, sizeof(receive_packet), 0, (struct sockaddr *)&remote, remote_length); //send the packet with ack
        if (receive_packet.sequence_number != expected_sequence_number)
            printf("This packet is not the expected packet. Expected->%d Received->%d\n",expected_sequence_number, receive_packet.sequence_number);
        /*If received packet is of expected sequence number then write to file*/
        else
        {
            printf("Sequence number of the packet received %d Number of bytes to write %d\n", receive_packet.sequence_number, receive_packet.datasize);
            write(fd, receive_packet.data, receive_packet.datasize);
            expected_sequence_number++;
        }
        bzero(receive_packet.data, sizeof(receive_packet.data));
        if(recvfrom(sockfd, &receive_packet, sizeof(receive_packet),0,  (struct sockaddr *)&remote, &remote_length) <= 0) //receive next packet
        {
            perror("listener:");
            exit(-1);
        }
    }
    if (receive_packet.sequence_number == (expected_sequence_number - 1)) //Check if file transfer complete
    {
        printf("Received file %s\n", receive_packet.filename);
    }
   
}

/*This function gets a file from server to the client*/
void get_file(struct sockaddr_in remote, int remote_length, int sockfd, fd_set readset)
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
    while(nbytes = read(fd, sender_packet.data, MAXBUFSIZE)) //read MAXBUFSIZE bytes from the file 
    {
        sender_packet.sequence_number = seq_number_count; // pack all the required data into one packet
        sender_packet.datasize = nbytes;
        int resent_number =0;
        do
        {
            if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1) //send the packet with file data
            {
                perror("talker:sendto");
            }
            printf("Sent packet to the client with sequence number %d and datasize %d\n", sender_packet.sequence_number, sender_packet.datasize);
            bzero(sender_packet.data, nbytes);
            bzero(buffer, nbytes);
            FD_ZERO(&readset);
            FD_SET(sockfd, &readset);
            struct timeval timeout = {0,100000};// set timeout for 100ms
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
                recvfrom(sockfd, &receive_packet, sizeof(receive_packet), 0, (struct sockaddr *)&remote, &remote_length);//receive packet at the socket
                if(receive_packet.sequence_number != seq_number_count) //If received packet is not of expected sequence number, then wait for a while
                {
                    printf("sequence number of the received packet ack packet is %d but expected is %d , waiting for 10 ms .... \n", receive_packet.sequence_number, seq_number_count);
                 
                    FD_ZERO(&readset);
                    FD_SET(sockfd, &readset);
                    struct timeval timeout_delayed_ack = {0,10000}; //timeout set to 10ms
                    result = select(sockfd+1, &readset, NULL, NULL, &timeout_delayed_ack);
                    if(result ==0)
                    {
                        printf("Time out after receiving false/ delayed ack, Sending packet with sequence number %d again\n", seq_number_count);
                    }
                    else if (result > 0 && FD_ISSET(sockfd, &readset))
                    {
                        recvfrom(sockfd, &receive_packet, sizeof(receive_packet), 0, (struct sockaddr *)&remote, &remote_length); //receive packet at socket
                        if(receive_packet.sequence_number == seq_number_count) //If received expected packet, then accept ack and send next packet
                        {
                            printf(" Ack received for %d Packet\n",seq_number_count);
                            seq_number_count++;
                        }
                        //If again wrong packet is obtained, then send the same packet again
                        else 
                        {
                            printf("sequence number of the received packet ack packet is %d but expected is %d, sending again packet %d\n", receive_packet.sequence_number, seq_number_count, seq_number_count);
                            result = 0;
                        }
                    }
                }
                 
                else if(receive_packet.sequence_number == seq_number_count) //If received expected packet, then accept ack and send next packet 
                {
                    printf(" Ack received for %d Packet\n",seq_number_count);
                    seq_number_count++;
                } 
            }
        }while (result == 0 && resent_number < 6); // loop till ack is obtained or untill the same packet is sent 5 times
              
        if(resent_number >=6)
        {
            printf("Not able to transfer this file, please try again later\n");
            break;
        }
        else
            printf("Sent file succesfully\n");
        }
      
        /*Send file transfer complete message to client*/      
        strcpy(sender_packet.data, command);
        sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length);
        bzero(sender_packet.data, sizeof(sender_packet));
        bzero(receive_packet.data, sizeof(receive_packet));
               
}
