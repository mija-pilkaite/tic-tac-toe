#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include "client.h"


// work by Anja Matic and Mija Pilkaite (worked together)


/*process: UDP Server :

Create a UDP socket.
Bind the socket to the server address.
Wait until the datagram packet arrives from the client.
Process the datagram packet and send a reply to the client.
Go back to Step 3.
Source: https://www.geeksforgeeks.org/udp-server-client-implementation-c/*/
/*My other online references:
1. https://www.ibm.com/docs/en/zos/2.2.0?topic=programs-c-socket-udp-client
2. Moodle of course
3. https://www.geeksforgeeks.org/handling-multiple-clients-on-server-with-multithreading-using-socket-programming-in-c-cpp/ */

char rec_buf[1024];

int main(int argc, char *argv[])
{
    // Get the input from user
    unsigned short port;
    if (argc < 3)
    {
        fprintf(stderr, "Missing some arguments, expecting IP_ADDRESS and PORT_NUMBER.\n");
        return USER_INPUT_ERROR;
    }

    //Initialize sockaddr_in for client
    char *dest_add = argv[1];
    port = htons(atoi(argv[2]));
    struct sockaddr_in IPv4;
    IPv4.sin_family = AF_INET;
    IPv4.sin_port = port;

    int binary_form = inet_pton(AF_INET, dest_add, &IPv4.sin_addr);
    if (binary_form <= 0)
    {
        perror("Invalid IPv4 address");
        return INET_PTON_ERROR;
    }

    //Initialize socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR in creating socket");
        return SOCKET_ERROR;
    }

    //Create server struct
    my_server *server = (my_server *)malloc(sizeof(my_server));
    server->sockfd = sockfd;
    memcpy(&(server->IPv4), &IPv4, sizeof(struct sockaddr_in));

    //Make and send hello message to server
    char hello[7];
    hello[0] = TXT;
    memcpy(hello + 1, "Hello", sizeof(char) * 5);
    hello[6] = '\0';

    int sent = sendto(server->sockfd, hello, sizeof(hello), 0, (struct sockaddr *)&(server->IPv4), sizeof(struct sockaddr_in));
    if (sent < 0)
    {
        perror("Failed to send!");
        return SEND_ERROR;
    }

    // Sent hello, now we can receive

    receive_msg(server);
    close(sockfd);

    return 0;
}

int receive_msg(my_server *server)
{
    //Initalize sockaddr_in for the recieving server
    struct sockaddr_in RecAddrIn;
    memset(&RecAddrIn, 0, sizeof(RecAddrIn));
    socklen_t RecLen = sizeof(RecAddrIn);

    while (1)
    {

        memset(rec_buf, 0, sizeof(rec_buf)); //wipe the buffer
        int recvd = recvfrom(server->sockfd, rec_buf, sizeof(rec_buf), 0, (struct sockaddr *)&RecAddrIn, &RecLen);
        if (recvd < 0)
        {
            perror("Error in receiving data");
            close(server->sockfd);
            return RECEIVE_ERROR;
        }

        //Check message type
        if (*rec_buf == FYI)
        {
            print_board();
        }
        if (*rec_buf == MYM)
        {
            move(server);
        }

        if (*rec_buf == END)
        {
            end_game();
            close(server->sockfd);
            return 0;
        }
        if (*rec_buf == TXT)
        {
            printf("%s \n", rec_buf + 1);
        }
    }
    return 0;
}

void print_board()
{
    // Create empty board
    char board[3][3];
    memset(board, ' ', 9 * sizeof(char));

    int j;
    int i;

    int n_pos = binaryToDecimal(rec_buf[1]);

    //Get inofrmation from FYI message
    for (j = 0; j < n_pos; j++)
    {

        if (binaryToDecimal(rec_buf[3 * j + 2]) == 1)
        {
            board[binaryToDecimal(rec_buf[3 * j + 4])][binaryToDecimal(rec_buf[3 * j + 3])] = 'X';
        }
        else
        {
            board[binaryToDecimal(rec_buf[3 * j + 4])][binaryToDecimal(rec_buf[3 * j + 3])] = 'O';
        }
    }

    //Print the board
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            if (j % 3 == 2)
            {
                printf("%c \n", board[i][j]);
                if (i != 2) //dont want to print this under the last line
                {

                    printf("- + - + - \n");
                }
            }
            else
            {
                printf("%c |", board[i][j]);
            }
        }
    }
}

int move(my_server *server)
{
    //Initalize variobles to store the move, the column and the message to send
    char *my_move = NULL; 
    char send_message[3];
    int column, row; 
    memset(send_message, 0, sizeof(send_message));
    size_t length = 0;
    
    printf("Make a move ! Example: 2 1 (column 2 row 1), first column/row starts with 0! \n");

    if (getline(&my_move, &length, stdin) == 0)
    {
        return GET_LINE_ERROR;
    }

    // Convert char into int, 
    column = my_move[0] - '0';
    row = my_move[2] - '0';

    //Set the fields of the message to send
    send_message[0] = MOV;
    send_message[1] = column;
    send_message[2] = row;

    // Send the message
    int send = sendto(server->sockfd, send_message, sizeof(send_message), 0, (struct sockaddr *)&(server->IPv4), sizeof(struct sockaddr_in));
    if (send < 0)
    {
        perror("ERROR in send");
        return SEND_ERROR;
    }
    return 0;

}

void end_game()
{
    if (rec_buf[1] == -1)
    {
        // Does not print anything as the server will send TXT message
        return;
    }
    if (rec_buf[1] == 0)
    {
        printf("It's a draw! Maybe next time!:) \n");
    }
    else
    {
        printf("The winner is %d. \n", rec_buf[1]);
    }
}

int binaryToDecimal(int n)
{ //Source: Code from https://www.geeksforgeeks.org/c-program-to-convert-binary-to-decimal/
    
    int num = n;
    int dec_value = 0;

    // Initializing base value to 1, i.e 2^0
    int base = 1;

    int temp = num;
    while (temp)
    {
        int last_digit = temp % 10;
        temp = temp / 10;

        dec_value += last_digit * base;

        base = base * 2;
    }

    return dec_value;
}