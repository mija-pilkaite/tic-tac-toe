#ifndef CLIENT_H
#define CLIENT_H


// work by Anja Matic and Mija Pilkaite (worked together)


//Error codes
#define SOCKET_ERROR 1
#define USER_INPUT_ERROR 2
#define INET_PTON_ERROR 3
#define SEND_ERROR 4
#define RECEIVE_ERROR 6
#define GET_LINE_ERROR 7

//types of messages (given in the assignment)
#define FYI 0x01
#define MYM 0x02
#define END 0x03
#define TXT 0x04
#define MOV 0x05
#define LFT 0x06

/*my_server struct used to keep track of the socket and IPv4 (and pass it into the helper functions)*/
typedef struct
{
    int sockfd;
    struct sockaddr_in *IPv4;
} my_server;

// Function for printing the board and format the FYI message
void print_board();

// Handles MYM msg
// returns the move
int move(my_server *server);

// Handles END msg and announces the winner
void end_game();

/*Source: Code from https://www.geeksforgeeks.org/c-program-to-convert-binary-to-decimal/
Input: n, number represented in hex
Output: the decimal representation of n
*/
int binaryToDecimal(int n);

//manages all types of messages then calling their helper functions
int receive_msg(my_server* server);



#endif // CLIENT_H
