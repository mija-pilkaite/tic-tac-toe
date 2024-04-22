# tic-tac-toe
Final project for "Introduction to Networks" course (CSE207). 
The project was completed together with Anja Matic. 

The project implements a classic game of Tic-Tac-Toe using a client-server architecture over UDP. The implementation includes both server and client components, with the server managing the game state and client interactions, while the client provides the interface for players to make their moves. This implementation demonstrates basic networking concepts such as message handling, state management, and client-server communication over datagram sockets.

### Server Functionality
The server's responsibilities are as follows:

Initialize and Wait for Clients: the server initializes and waits for clients to send a "Hello" message to connect.

Handle Client Connections: accepts new clients if fewer than two clients are connected. Sends a rejection message (0xFF) if more than two clients attempt to connect.

Game Management: maintains the state of the 3x3 tic-tac-toe board, starts the game when two clients are connected, alternates turns between the two connected players, validates each move for correctness (checks if a cell is already occupied), updates clients about the current game state and prompts the active player to make a move.

Determines Game End: the game ends either when the board is completely filled or when a player wins by aligning three of their markers (X or O) in a row, column, or diagonal.

Notifies players of the game outcome – win or draw.

Communication: uses specific message types to communicate with clients, including text messages, game state updates, move requests, and end-game notifications.

### Client Functionality
The client's responsibilities are as follows:

Connect to Server: sends a "Hello" message upon startup to establish a connection with the server.

Game Interaction: displays the game board and current state to the user, requests and sends player moves to the server.

Handle Server Messages: interprets messages from the server to update the game board, prompt user action, or conclude the game.

### Messages
Server to Client

TXT: Text message for display. Ends with a null character ('\0').

MYM: Prompt to the client to make a move ("Make Your Move").

END: Indicates the game's conclusion with details of the outcome (0 for draw, 1 for player 1 win, 2 for player 2 win, and 255 (0xFF) for no room for new players).

FYI: For Your Information about the occupied positions on the board. It includes details such as player ID, column, and row for each occupied position.

Client to Server

MOV: Contains the coordinates of the player's move (column and row where the player wishes to place their symbol).

TXT: Initial "Hello" message from the client to the server to attempt the connection.

### Usage
To make both the server and the client type: `./make`

Client:
To run the client, type in:

`./client IP_address Port_Number (e.g. ./client 127.0.0.1 5000)`

When two clients are connected to the server, the first client that connected 
is prompted to make a move in a format column row, where each takes a value 0-2.
The game is over once one of the clients wins and both terminate.

Server:
To run the server, type in:

`./server PORT_NUMBER (e.g. ./server 5000)`

The server accepts two clients (any more incoming clients are dropped);
One game happens at a time (more could be added by using multithreading);
