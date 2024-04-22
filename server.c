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
#include "server.h"


// work by Anja Matic and Mija Pilkaite (worked together)

int num_players = 0; // connected clients
char board[3][3];    // keeping track of the board

char FYI_msg[29]; // The FYI message, continously updated
char MYM_msg[1]; 

int move_count = 0;
struct sockaddr_in active_players[2];
int active_game = 0;

int main(int argc, char *argv[])
{
    memset(FYI_msg, '\0', 29);
    FYI_msg[0] = FYI;

    MYM_msg[0] = MYM;

    memset(board, 0, sizeof(board));

    if (argc < 2)
    {
        fprintf(stderr, "Missing some arguments, expecting a POR4T_NUMBER.\n");
        return USER_INPUT_ERROR;
    }
    char *port = argv[1];

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("ERROR when creating socket");
        return SOCKET_ERROR;
    }
    //printf("Socket created \n");

    struct sockaddr_in servaddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(atoi(port)),
    };

    struct sockaddr *addr;
    addr = (struct sockaddr *)&servaddr;

    if (bind(sockfd, addr, sizeof(struct sockaddr_in)) != 0)
    {
        perror("ERROR in bind");
        return BIND_ERROR;
    }

    //printf("Bound to port %s \n", port);

    // Set all the fields of the players to 0
    memset(active_players, 0, sizeof(struct sockaddr_in) * 2);

    // Structs for storing infor about the clients
    struct sockaddr_in NewClientAddrIn;
    memset(&NewClientAddrIn, 0, sizeof(struct sockaddr_in));
    socklen_t newClilen = sizeof(struct sockaddr_in);

    char buf[1024];

    while (1)
    {

        // Recieve messages from the clients
        memset(&NewClientAddrIn, 0, sizeof(struct sockaddr_in));
        memset(buf, 0x00, 1024);

        printf("Waiting for input \n");
        int received = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&NewClientAddrIn, &newClilen);

        if (received < 0)
        {
            perror("ERROR in recieve");
            return -1;
        }

        // printf("Recieved %d bytes\n", received);
        // printf("Received message: %s \n", buf);
        // printf("In binary %d \n", binaryToDecimal(buf[0]));

        if (*buf == TXT)
        {
            printf("[TXT]\n");

            if (strncmp(buf + 1, "Hello", 5) == 0)
            {
                // Check that there is space in the game for a new player and that this player is not aleady playing

                int n = sizeof(struct sockaddr_in);

                if (num_players < 2 && memcmp(&active_players[0], &NewClientAddrIn, n) != 0)
                {
                    // Check if its a known player or not
                    // just need to check first one since the second one should be empty if there is less than two players connected
                    active_players[num_players] = NewClientAddrIn;
                    num_players++;

                    send_welcome(sockfd);
                }

                if (num_players == 2)
                {
                    if (active_game == 0)
                    {
                        // No game is active, start one
                        active_game = 1;

                        send_msg(sockfd, active_players[0], FYI_msg, sizeof(FYI_msg));
                        send_msg(sockfd, active_players[0], MYM_msg, sizeof(MYM_msg));
                    }
                    else
                    {
                        char msg[64];
                        msg[0] = TXT;
                        memcpy(msg + 1, "Too many players connected, try again later \n", sizeof("Too many players connected, try again later \n"));
                        printf("Message: %s \n", msg);
                        send_msg(sockfd,NewClientAddrIn, msg, sizeof(msg));
                        
                        /*if (sendto(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&NewClientAddrIn, sizeof(struct sockaddr_in)) < 0)
                        {
                            perror("Failed to send!");
                            return -1;}*/
                        
                        char msg2[2];
                        msg2[0] = END;
                        msg2[1] = 0xFF;
                        send_msg(sockfd, NewClientAddrIn, msg2, sizeof(msg2));
                    }
                }
            }
            else
            {
                printf("Unkown message \n");
                // send_msg(message unkown disconnetc)
            }
        }

        if (*buf == MOV)
        {
            printf("[MOV] \n");
            int s = -1;
            int n = sizeof(struct sockaddr_in);
            if (memcmp(&NewClientAddrIn, &active_players[0], sizeof(struct sockaddr_in)) == 0)
            {
                printf("Player 1 \n");
                s = play_move(sockfd, 1, buf);
                if(s== 0){
                    // send_msg(sockfd, active_players[1], FYI_msg);
                    send_msg(sockfd, active_players[1], MYM_msg, sizeof(MYM_msg));
                }
            }
            else if (memcmp(&NewClientAddrIn, &active_players[1], n) == 0)
            {
                printf("Player 2 \n");
                s = play_move(sockfd, 2, buf);

                if (s == 0)
                {
                    // send_msg(sockfd, active_players[0], FYI_msg);
                    send_msg(sockfd, active_players[0], MYM_msg, sizeof(MYM_msg));
                }
            }
            else
            {
                printf("Unkown player trying to move \n");
                return UNKOWN_PLAYER_ERROR;
            }

            if (s == 1)
            {
                // Game is over, we wipe everything, to start over
                active_game = 0;
                num_players = 0;
                memset(board, 0, sizeof(board));
                memset(FYI_msg + 1, 0, 28);
                move_count = 0;
                memset(active_players, 0, sizeof(active_players));
            }
        }
    }

    close(sockfd);
    return 0;
}

int send_welcome(int sockfd)
{
    char msg[64];
    msg[0] = TXT;

    if (num_players == 1)
    {
        memcpy(msg + 1, "Welcome! You are player 1, you play with X.\n", sizeof("Welcome! You are player 1, you play with X.\n"));
    }
    else
    {
        memcpy(msg + 1, "Welcome! You are player 2, you play with O.\n", sizeof("Welcome! You are player O, you play with O.\n"));
    }

    if (sendto(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&active_players[num_players - 1], sizeof(struct sockaddr_in)) < 0)
    {
        perror("Failed to send!");
        return SEND_ERROR;
    }
    return 0;
}


int play_move(int sockfd, int play_num, char *move)
{

    //printf("Making moves \n");
    // int col = active_player->buffer[1];
    // int row = active_player->buffer[2];

    int col = binaryToDecimal(move[1]);
    int row = binaryToDecimal(move[2]);

    printf("Column: %d, row: %d \n", col, row);

    if (col > 2 || col < 0 || row > 2 || row < 0)
    {
        char msg[64];
        msg[0] = TXT;
        memcpy(msg + 1, "Not a valid move. Please make a new move.\n", sizeof("Not a valid move. Please make a new move.\n"));
        send_msg(sockfd, active_players[play_num - 1], msg, strlen("Not a valid move. Please make a new move.\n") + 1);
        send_msg(sockfd, active_players[play_num - 1], MYM_msg, sizeof(MYM_msg));
        return -1;
    }

    if (board[row][col] == 0)
    {

        printf("Valid move \n");
        board[row][col] = play_num;

        printf("Placed \n");
        // printf("Board before update \n");
        // print_board();

        FYI_msg[move_count * 3 + 2] = play_num;
        FYI_msg[move_count * 3 + 3] = col;
        FYI_msg[move_count * 3 + 4] = row;
        move_count++;
        //printf("%d \n", move_count);
        FYI_msg[1] = move_count;

        //printf("Updated FYI \n");
        int j;

        printf("Move count: %d \n", binaryToDecimal(FYI_msg[1]));

        for (j = 0; j < move_count; j++)
        {
            printf("Player: %d, col: %d, row: %d \n", binaryToDecimal(FYI_msg[j * 3 + 2]),
                   binaryToDecimal(FYI_msg[j * 3 + 3]), binaryToDecimal(FYI_msg[j * 3 + 4]));
        }

        if( sendto(sockfd, FYI_msg, sizeof(FYI_msg), 0, (struct sockaddr *)&active_players[-1 * (play_num - 3) - 1], sizeof(struct sockaddr_in)) < 0){
            perror("ERROR in sendto");
            return SEND_ERROR;
        }

        // printf("Board after update \n");
        // print_board();
    }
    else
    {
        char msg[64];
        msg[0] = TXT;
        memcpy(msg + 1, "Position already taken. Please make a new move\n", sizeof("Position already taken. Please make a new move\n"));
        send_msg(sockfd, active_players[play_num - 1], msg, strlen("Position already taken. Please make a new move\n") + 1);
        send_msg(sockfd, active_players[play_num - 1], MYM_msg, sizeof(MYM_msg));
        return -1;
    }
    int w = check_win(play_num, col, row);

    if (w != -1)
    {
        char end_msg[2];
        end_msg[0] = END;
        end_msg[1] = w;
        send_msg(sockfd, active_players[0], end_msg, sizeof(end_msg));
        send_msg(sockfd, active_players[1], end_msg, sizeof(end_msg));
        return 1;
    }

    return 0;
}

int send_msg(int sockfd, struct sockaddr_in playerAddr, char *msg, size_t msg_len)
{
    int s = sendto(sockfd, msg, msg_len, 0, (struct sockaddr *)&playerAddr, sizeof(struct sockaddr_in));
    if (s < 0)
    {
        perror("ERROR in sendto");
        return SEND_ERROR;
    }
    return 0;
}

int check_win(int player, int col, int row)
{
    // Source: https://stackoverflow.com/questions/1056316/algorithm-for-determining-tic-tac-toe-game-over
    // Check column
    int i;
    // check col
    for (i = 0; i < 3; i++)
    {
        if (board[row][i] != player)
        {
            break;
        }
        if (i == 2)
        {
            return player;
        }
    }
    // check row
    for (i = 0; i < 3; i++)
    {
        if (board[i][col] != player)
        {
            break;
        }
        if (i == 2)
        {
            return player;
        }
    }

    // check diag
    if (row == col)
    {
        // we're on a diagonal
        for (i = 0; i < 3; i++)
        {
            if (board[i][i] != player)
            {
                break;
            }
            if (i == 2)
            {
                return player;
            }
        }
    }
    // check anti diag (thanks rampion)
    if (row + col == 2)
    {
        for (i = 0; i < 3; i++)
        {
            if (board[i][2 - i] != player)
            {
                break;
            }
            if (i == 2)
            {
                return player;
            }
        }
    }

    if (move_count == 9)
    {
        return 0;
    }

    return -1;
}

int binaryToDecimal(int n)
{
    // Code from https://www.geeksforgeeks.org/c-program-to-convert-binary-to-decimal/
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
