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


#define MAXBUFSIZE 5*1024 //Max buffer size to read from the file

unsigned int server_exit_flag = 0;


int put_file(struct sockaddr_in remote, int remote_length, int sockfd, fd_set readset);
void get_file(struct sockaddr_in remote, int remote_length, int sockfd);
void delete_file(struct sockaddr_in remote, int remote_length, int sockfd);
void get_list_of_files(struct sockaddr_in remote, int remote_length, int sockfd);
void server_exit(struct sockaddr_in remote, int remote_length, int sockfd);
void process_invalid_command(struct sockaddr_in remote, int remote_length, int sockfd);


/* Creating struct for packet transfer */

struct packet_t{
    int sequence_number;
    unsigned int datasize;
    char data[MAXBUFSIZE];
    char command[50];
    char filename[50];
    unsigned int valid;
    unsigned int total_packets;
};

struct packet_t sender_packet, receiver_packet;

/*Display menu function*/
void display_menu()
{
   printf("\n================================================Command Menu=====================================================================================\n");
   printf("\"get [file_name]\"       : The server transmits the requested file to the client \n");
   printf("\"put [file_name]\"       : The server receives transmitted file from the server and stores it locally \n");
   printf("\"delete [file_name]\"    : The server deletes the file if exists otherwise does nothing \n");
   printf("\"ls\"                    : The server searches for all files in its directory and sends the list of all the files to the client \n");
   printf("\"exit\"                  : The server exits gracefully \n");
}

/*This function provides data encryption and decryption*/
char * encryptdecrypt(char * message, int length)
{
    int i;
    char key = 'S';
    for(i=0; i < length; i++)
    {
        message[i] ^= key; //Bitwise XOR with character 'S'
    }
    return message;
}

/*This function calculates the filesize and calculates number of packets*/
unsigned int calculate_filesize(FILE *temp_fp)
{
    fseek(temp_fp, 0L, SEEK_END);
    unsigned long temp_filesize = ftell(temp_fp); //get file size
    printf("File size of the file to be transmitted is %ld\n", temp_filesize);
    fseek(temp_fp, 0L, SEEK_SET);
    int temp_bufsize = MAXBUFSIZE;
    unsigned int number_of_packets = (temp_filesize/(temp_bufsize));//get number of packets
    if ((temp_filesize % MAXBUFSIZE) != 0);
        number_of_packets ++;
    
    return number_of_packets; //return number of packets
    
}


int main (int argc, char * argv[])
{
    int sockfd;                               //this will be our socket
    fd_set readset;
    int remote_length;
    char user_command[50];
    struct sockaddr_in remote;              //"Internet socket address structure"
      
    char *transfer_file;
        
    //check for arguments: server ip address and server port number    
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
    //infinite loop until client is exited
    while(1)
    {
        display_menu();
        printf("\n Please enter a valid command from the command menu\n");
        if(server_exit_flag == 1)
           printf("Note: Server has been exited. The command wont be succesful unless both client and server are restarted again\n");
        gets(user_input);// Get user input
        printf("Command entered: %s\n", user_input);
        strcpy(sender_packet.command , "\0");
        strcpy(sender_packet.filename, "\0");
        //Tokenize user input to get command and filename(if required)
        token = NULL;
        token = strtok(user_input, " [ ]");
        if (token == NULL)
        {
            sender_packet.valid =0;
        }
        else
            strcpy(sender_packet.command, token);
        while (token != NULL)
        {
            token = strtok(NULL, " [ ]");
            if(token != NULL)
            {
                strcpy(sender_packet.filename, token);
            }
        }
        
        /* Execute appropriate functions according to commands*/
        if (strcmp(sender_packet.command , "get") == 0)
        {
            if(strcmp(sender_packet.filename, "\0") == 0) //Check if filename is present, else ask user to retry
            {
                printf("Error: Please Enter a Filename: get [<filename>]");
                continue;
            }
 
            printf("Obtaining file %s from the server\n", sender_packet.filename);
            sender_packet.valid = 1;
            get_file(remote, remote_length, sockfd); //Get file from the server
        }
        
        else if (strcmp(sender_packet.command , "put") == 0)
        {
            if(strcmp(sender_packet.filename, "\0") == 0) //Check if filename is present, else ask user to retry
            {
                printf("Error: Please Enter a Filename: put [<filename>]");
                continue;
            }
 
            printf("Sending file %s to the server.....\n", sender_packet.filename);
            sender_packet.valid = 1;
            if(put_file(remote, remote_length, sockfd, readset) == -1)// Put file into server only if file exists in client side
                continue;
        }
        else if (strcmp(sender_packet.command, "delete") == 0)
        {
            if(strcmp(sender_packet.filename, "\0") == 0) //Check if filename is present, else ask user to retry
            {
                printf("Error: Please Enter a Filename: delete [<filename>]");
                continue;
            }
            printf("Deleting File %s from the server directory\n", sender_packet.filename);
            sender_packet.valid = 1;
            delete_file(remote, remote_length, sockfd);// Delete the given functin from the server end
        }
       
        else if (strcmp(sender_packet.command , "ls") == 0)
        {
            if(strcmp(sender_packet.filename, "\0") != 0)
            {
                printf("Did you mean <ls>?, try again\n");
                continue;
            }
            sender_packet.valid = 1;
            get_list_of_files(remote, remote_length, sockfd); //List the files in the server directory
        }
       
        else if (strcmp(sender_packet.command , "exit") == 0)
        {
            if(strcmp(sender_packet.filename, "\0") != 0)
            {
                printf("Did you mean <exit>?, try again\n");
                continue;
            } 
            sender_packet.valid = 1;
            server_exit(remote, remote_length, sockfd);//Command to server to exit
        }
        else
        {
            /*If invalid command, send to server with invalid flag*/
            sender_packet.valid = 0;
            process_invalid_command(remote, remote_length, sockfd); //Processes invalid command
        }
    }
       
	close(sockfd); // close the socket
}

/*This function processes the invalid command*/
void process_invalid_command(struct sockaddr_in remote, int remote_length, int sockfd)
{
  int result=0;
  fd_set readset;
  if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1) //send the packet with invalid command
  {
      perror("talker:sendto");
  }
  FD_ZERO(&readset);
  FD_SET(sockfd, &readset); //set active socket to fd_set
  struct timeval timeout = {0,800000}; //Sent timeout to 400ms
  result = select(sockfd+1, &readset, NULL, NULL, &timeout);
  if( result == -1)
  {
      printf("Error in select, try again");
      exit(-1);
  } 
  else if( result == 0)
  {
      printf("Time out, Server did not respond in past 800ms. Please check if server is still active and try again\n"); //If client is inactive for 800ms, then it stops waiting for server to respond and goes back to display menu
  }

  else if (result > 0 && FD_ISSET(sockfd, &readset))
  {
      recvfrom(sockfd, &receiver_packet, sizeof(receiver_packet), 0, (struct sockaddr *)&remote, &remote_length); //wait for server to respond to invalid command
      printf("%s\n",receiver_packet.data);
   }
  
}

/*This function sends a command to server to exit*/
void server_exit(struct sockaddr_in remote, int remote_length, int sockfd)
{
    int result=0;
    fd_set readset;
    /*Send command to server to exit*/
    if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1) //send the packet with command exit
    {
        perror("talker:sendto");
    }
    FD_ZERO(&readset);
    FD_SET(sockfd, &readset); //set active socket to fd_set
    struct timeval timeout = {0,400000}; //Sent timeout to 400ms
    result = select(sockfd+1, &readset, NULL, NULL, &timeout);
    if( result == -1)
    {
        printf("Error in select, try again");
        exit(-1);
    }  
    else if( result == 0)
    {
        printf("Time out, Server did not exit in past 400ms. Please check if server is still active and try again\n"); //If client is inactive for 400ms, then it stops waiting for server to exit and goes back to display menu
    }

    else if (result > 0 && FD_ISSET(sockfd, &readset))
    {
        recvfrom(sockfd, &receiver_packet, sizeof(receiver_packet), 0, (struct sockaddr *)&remote, &remote_length);//wait from ack from server that it is received the command exit
        if(!(strcmp(receiver_packet.command ,"exit"))) //If ack is correct then client displays message that server exited
        {
            server_exit_flag = 1;
            printf("Server exited gracefully\n");
        }
        else
            printf("Server could not exit\n");
    }
  
}
/*This function obtains list of files from the server directory*/
void get_list_of_files(struct sockaddr_in remote, int remote_length, int sockfd)
{
    char *token;
    int result=0;
    fd_set readset;
    if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1) //send the packet with ls command
    {
        perror("talker:sendto");
    }
    FD_ZERO(&readset);
    FD_SET(sockfd, &readset); //set active socket to fd_set
    struct timeval timeout = {0,400000}; //Sent timeout to 400ms
    result = select(sockfd+1, &readset, NULL, NULL, &timeout);
    if( result == -1)
    {
        printf("Error in select, try again");
        exit(-1);
    }
    else if( result == 0)
    {
            printf("Time out, Not received any packet in 400ms. Please check if server is still active and try again\n"); //If client is inactive for 400ms, then it stops waiting for list of files and goes back to display menu
    }

    else if (result > 0 && FD_ISSET(sockfd, &readset))
    {
        recvfrom(sockfd, &receiver_packet, sizeof(receiver_packet), 0, (struct sockaddr *)&remote, &remote_length); //receive the list of files from server
        token = strtok(receiver_packet.data, "#");
        while (token != NULL)
        {
            printf("%s\n", token); //display each filename
            token = strtok(NULL, "#");
        }
        printf("Number of files is %d\n", receiver_packet.datasize);
    }
}

/*This function is to delete a particular file from the server end*/
void delete_file(struct sockaddr_in remote, int remote_length, int sockfd)
{
    int result=0;
    fd_set readset;
    if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1) //send the packet with delete command
    {
        perror("talker:sendto");
    }
    FD_ZERO(&readset);
    FD_SET(sockfd, &readset); //set active socket to fd_set
    struct timeval timeout = {0,400000}; //Sent timeout to 400ms
    result = select(sockfd+1, &readset, NULL, NULL, &timeout);
    if( result == -1)
    {
        printf("Error in select, try again");
        exit(-1);
    }
    else if( result == 0)
    {
            printf("Time out, Server did not respond after command delete. Please check if server is still active and try again\n"); //If client is inactive for 400ms, then it stops waiting for server's response and goes back to display menu
    }

    else if (result > 0 && FD_ISSET(sockfd, &readset))
    {
        recvfrom(sockfd, &receiver_packet, sizeof(receiver_packet), 0, (struct sockaddr *)&remote, &remote_length); //receive ack of command delete
        if(strcmp(receiver_packet.data, "success") == 0)
            printf("Delete Successful on server end\n");
        else
            printf("Delete unsuccessful please try again\n");
    
    }
}

/*This function is to get a particular file from the server*/
void get_file(struct sockaddr_in remote, int remote_length, int sockfd)
{
    char command[] = "#End#of#File#";
    int fd, result=0, count=0;
    unsigned int temp_number_of_packets=0; 
    fd_set readset;
    fd = open( sender_packet.filename, O_RDWR|O_CREAT|O_TRUNC , 0666);
    
    int expected_sequence_number = 1;
    
    if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1) //send the packet with get command
    {
        perror("talker:sendto");
    }
    while(1)
    { 
        FD_ZERO(&readset);
        FD_SET(sockfd, &readset); //set active socket to fd_set
        struct timeval timeout = {0,400000}; //Sent timeout to 100ms
        result = select(sockfd+1, &readset, NULL, NULL, &timeout);
        if( result == -1)
        {
            printf("Error in select, try again");
            exit(-1);
        }
        else if( result == 0)
        {
            printf("Time out, Not received any packet in 400ms. Please check if server is still active and try again\n"); //If client is inactive for 400ms, then it stopd the transfer and goes back to display menu
            break;
        }

        else if (result > 0 && FD_ISSET(sockfd, &readset))
        {
            recvfrom(sockfd, &receiver_packet, sizeof(receiver_packet), 0, (struct sockaddr *)&remote, &remote_length); //wait from file packet
            if(count == 0)
            {    temp_number_of_packets = receiver_packet.total_packets;
                 printf("Number of packets expecting to be received : %d\n", temp_number_of_packets);
            }
            
            count = 1;
            if(strcmp(receiver_packet.data, command) == 0) //exit if file transfer is complete
                break;
            if(sendto(sockfd, &receiver_packet, sizeof(receiver_packet), 0, (struct sockaddr *)&remote, remote_length) == -1) // send ack
            {    
                perror("listener:sendto"); 
            }
            if (receiver_packet.sequence_number != expected_sequence_number) //check if received packet sequence number is expected sequence number
                printf("This packet is not the expected packet. Expected packet: %d, Received packet:%d\n", expected_sequence_number, receiver_packet.sequence_number);
            /*If sequence number matches then write to file*/
            else
            {
                printf("Sequence number of the packet received: %d. Number of bytes to decrypt and write: %d\n", receiver_packet.sequence_number, receiver_packet.datasize);
                char * decrypted = encryptdecrypt(receiver_packet.data, receiver_packet.datasize); //decrypt file packet
                write(fd, decrypted, receiver_packet.datasize);
                expected_sequence_number++;
            }
        }
                bzero(receiver_packet.data, sizeof(receiver_packet.data)); //clear buffer
    }
    printf("Number of packets received %d\n", (expected_sequence_number -1));

    if ((expected_sequence_number -1) == temp_number_of_packets) //Check if file transfer complete
    {
        printf("Received file %s successfully\n", receiver_packet.filename);
    }
    else
        printf("Error: Could not receive the complete file\n");
 
}

/*This function is to put a particular file into ther server*/
int put_file(struct sockaddr_in remote, int remote_length, int sockfd, fd_set readset)
{
    int fd, nbytes;
    int result = 0;
    char buffer[MAXBUFSIZE];
    FILE *temp_fp;
    unsigned int number_of_packets;
    temp_fp = fopen(sender_packet.filename, "r");
    if (temp_fp == NULL)
    {
        perror("File not opened : ");
        return -1;
    }
    number_of_packets = calculate_filesize(temp_fp) ;
    fclose(temp_fp);
    printf("Number of packets to be sent is %d\n", number_of_packets);
    sender_packet.total_packets = number_of_packets;
    fd = open(sender_packet.filename , O_RDONLY); //file open with read only option
    if(fd == -1)
    {
        perror("File not opened: ");
        return -1;
    }
    char command[] = "#End#of#File#";
	/******************
	  sendto() sends immediately.  
	  it will report an error if the message fails to leave the computer
	  however, with UDP, there is no error if the message is lost in the network once it leaves the computer.
	 ******************/
    int seq_number_count = 1;
    while(nbytes = read(fd, sender_packet.data, MAXBUFSIZE)) //read MAXBUFSIZE from the file
    {
        sender_packet.sequence_number = seq_number_count; // pack all the required data into one packet
        sender_packet.datasize = nbytes; //datasize = bytes read
        int resent_number =0;
        printf("*********************Before encryption *************************************\n");
        printf("%s\n",sender_packet.data);
        char * encrypted = encryptdecrypt(sender_packet.data, nbytes); //encrypt file data packet
        printf("**********************************After encryption*********************************\n");
        printf("%s\n", encrypted);
        memcpy(sender_packet.data, encrypted, nbytes);
       
        do
        {
            if(sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length) == -1) //send the packet
            {
                perror("talker:sendto");
            }
            resent_number++; //Count of number of times same packet is resent
            printf("Sent encrypted packet to the server with sequence number %d and datasize %d\n",sender_packet.sequence_number,sender_packet.datasize);
            bzero(sender_packet.data, nbytes);
            bzero(buffer, nbytes);
            FD_ZERO(&readset);
            FD_SET(sockfd, &readset); //set active socket to fd_set
            struct timeval timeout = {0,100000}; //Sent timeout to 100ms
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
                recvfrom(sockfd, &receiver_packet, sizeof(receiver_packet), 0, (struct sockaddr *)&remote, &remote_length); //receive packet and check if it is the expected one
                if(receiver_packet.sequence_number != seq_number_count)
                {
                    printf("sequence number of the received packet ack packet is %d but expected is %d , waiting for 10 ms .... \n", receiver_packet.sequence_number, seq_number_count);
                    FD_ZERO(&readset);
                    FD_SET(sockfd, &readset);
                    struct timeval timeout_delayed_ack = {0,10000}; //If wrong ack obtained then wait for another 10ms for correct ack before resending the packet
                    result = select(sockfd+1, &readset, NULL, NULL, &timeout_delayed_ack);
                    if(result ==0)
                    {
                        printf("Time out after receiving false/ delayed ack, Sending packet with sequence number %d again\n", seq_number_count);
                    }
                    else if (result > 0 && FD_ISSET(sockfd, &readset))
                    {
                        recvfrom(sockfd, &receiver_packet, sizeof(receiver_packet), 0, (struct sockaddr *)&remote, &remote_length);
                        if(receiver_packet.sequence_number == seq_number_count) //If expected packet obtained then consider ack received and wait for next packet
                        {
                            printf(" Ack received for %d Packet\n",seq_number_count);
                            seq_number_count++; 
                        }
                        else //If packet obtained is not the expected one, then send packet again
                        {
                            printf("Sequence number of the received packet ack packet is %d but expected is %d,sending again packet %d\n", receiver_packet.sequence_number, seq_number_count, seq_number_count);
                            result = 0;
                        }
                    }
                }
                 
                else if(receiver_packet.sequence_number == seq_number_count) //If expected packet obtained then consider ack received and wait for next packet
                {
                    printf(" Ack received for %d Packet\n",seq_number_count);
                    seq_number_count++;
                     
                } 
            }
        }while (result == 0 && resent_number < 6); //loop till correct ack is obtained or until same packet is sent 5 times
      
        if(resent_number >=6)
        {
            printf("Not able to transfer this file, please try again later\n");
            break;
        }
    } 
    
    printf("Number of packets sent %d\n", (seq_number_count - 1)); 
    if((seq_number_count - 1) == number_of_packets)
        printf("Sent file successfully\n");
    else
        printf("Error: File transfer unsuccesful\n");
         
    strcpy(sender_packet.data, command); 
    //Send file transfer is complete to server
    sendto(sockfd, &sender_packet, sizeof(sender_packet), 0, (struct sockaddr *)&remote, remote_length);
    bzero(sender_packet.data, sizeof(sender_packet));
    bzero(receiver_packet.data, sizeof(receiver_packet));
}

