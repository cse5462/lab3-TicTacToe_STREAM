/**********************************************************/
/* This program is a 'pass and play' version of tictactoe */
/* Two users, player 1 and player 2, pass the game back   */
/* and forth, on a single computer                        */
/**********************************************************/

/* #include files go here */
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

/* The number of command line arguments. */
#define NUM_ARGS 2
/* TODO */
#define BACKLOG_MAX 5
/* #define section, for now we will define the number of rows and columns */
#define ROWS  3
#define COLUMNS  3

/* C language requires that you predefine all the routines you are writing */
void print_error(const char *msg, int errnum, int terminate);
void handle_init_error(const char *msg, int errnum);
void extract_args(char *argv[], int *port);
int create_endpoint(struct sockaddr_in *socketAddr, unsigned long address, int port);
int init_shared_state(char board[ROWS][COLUMNS]);
int tictactoe(int sd, char board[ROWS][COLUMNS]);
int check_win(char board[ROWS][COLUMNS]);
void print_board(char board[ROWS][COLUMNS]);

int main(int argc, char *argv[]) {
    int sd, portNumber;
    char board[ROWS][COLUMNS];
    struct sockaddr_in serverAddr;

    /* If arg count correct, extract arguments to their respective variables */
	if (argc != NUM_ARGS) handle_init_error("argc: Invalid number of command line arguments", 0);
	extract_args(argv, &portNumber);

    /* TODO */
    /* Create server socket from user provided data */
	sd = create_endpoint(&serverAddr, INADDR_ANY, portNumber);
    if (listen(sd, BACKLOG_MAX) == 0) {
        printf("Server listening at %s on port %hu\n", "TEST", serverAddr.sin_port);
        while (1) {
            int connected_sd;
            struct sockaddr_in clientAddress;
            socklen_t fromLength;

            if ((connected_sd = accept(sd, (struct sockaddr *)&clientAddress, &fromLength)) >= 0) {
                printf("PLAYER connected...\n");
                init_shared_state(board); // Initialize the 'game' board
                tictactoe(connected_sd, board); // call the 'game'
                if (close(connected_sd) < 0) print_error("close client-connection", errno, 0);
            } else {
                print_error("accept", errno, 0);
            }
        }
    } else {
        print_error("listen", errno, 1);
    }

  return 0;
}

/**
 * @brief Prints the provided error message and corresponding errno message (if present) and
 * terminates the process if asked to do so.
 * 
 * @param msg The error description message to display.
 * @param errnum This is the error number, usually errno.
 * @param terminate Whether or not the process should be terminated.
 */
void print_error(const char *msg, int errnum, int terminate) {
	/* Check for valid error code and generate error message */
	if (errnum) {
		printf("ERROR: %s: %s\n", msg, strerror(errnum));
	} else {
		printf("ERROR: %s\n", msg);
	}
	/* Exits process if it should be terminated */
	if (terminate) exit(EXIT_FAILURE);
}

/**
 * @brief Prints a string describing the initialization error and provided error number (if
 * nonzero), the correct command usage, and exits the process signaling unsuccessful termination. 
 * 
 * @param msg The error description message to display.
 * @param errnum This is the error number, usually errno.
 */
void handle_init_error(const char *msg, int errnum) {
	print_error(msg, errnum, 0);
	printf("Usage is: tictactoeP1 <remote-port>\n");
	/* Exits the process signaling unsuccessful termination */
	exit(EXIT_FAILURE);
}

/**
 * @brief Extracts the user provided arguments to their respective local variables and performs
 * validation on their formatting. If any errors are found, the function terminates the process.
 * 
 * @param argv Pointer to the first element of an array of argc + 1 pointers, of which the
 * last one is null and the previous ones, if any, point to strings that represent the
 * arguments passed to the program from the host environment. If argv[0] is not a null
 * pointer (or, equivalently, if argc > 0), it points to a string that represents the program
 * name, which is empty if the program name is not available from the host environment.
 * @param port The remote port number that the server should listen on
 */
void extract_args(char *argv[], int *port) {
	/* Extract and validate remote port number */
	*port = strtol(argv[1], NULL, 10);
	if (*port < 1 || *port != (u_int16_t)(*port)) handle_init_error("remote-port: Invalid port number", 0);
}

/**
 * @brief Creates the comminication endpoint with the provided IP address and port number. If any
 * errors are found, the function terminates the process.
 * 
 * @param socketAddr The socket address structure creates for the comminication endpoint.
 * @param address The IP address for the socket address structure.
 * @param port The port number for the socket address structure.
 * @param backlog TODO
 * @return The socket descriptor of the created comminication endpoint.
 */
int create_endpoint(struct sockaddr_in *socketAddr, unsigned long address, int port) {
	int sd;
	/* Create socket */
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) != -1) {
		socketAddr->sin_family = AF_INET;
		/* Assign IP address to socket */
		socketAddr->sin_addr.s_addr = address;
		/* Assign port number to socket */
		socketAddr->sin_port = htons(port);
	} else {
		print_error("create_endpoint: socket", errno, 1);
	}
    /* Bind socket */
	if (bind(sd, (struct sockaddr *)socketAddr, sizeof(*socketAddr)) == 0) {
		printf("[+]Server socket created successfully.\n");
	} else {
		print_error("create_endpoint: bind", errno, 1);
	}
    
	return sd;
}

int init_shared_state(char board[ROWS][COLUMNS]) {    
    /* this just initializing the shared state aka the board */
    int i, j, count = 1;

    printf("[+]Initializing shared state.\n");
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            board[i][j] = count++ + '0';
        }
    }
    return 0;
}

int tictactoe(int sd, char board[ROWS][COLUMNS]) {
    /* this is the meat of the game, you'll look here for how to change it up */
    int player = 1; // keep track of whose turn it is
    int i, choice;  // used for keeping track of choice user makes
    int row, column;
    char mark;      // either an 'x' or an 'o'
    char p1pick, p2pick;

    /* loop, first print the board, then ask player 'n' to make a move */

    do {
        print_board(board); // call function to print the board on the screen
        player = (player % 2) ? 1 : 2;  // Mod math to figure out who the player is
        if (player == 1) {
            printf("Player %d, enter a number:  ", player); // print out player so you can pass game
            scanf("%c", &p1pick); //using scanf to get the choice
            send(sd, &p1pick, sizeof(char), MSG_NOSIGNAL);
        } else {
            recv(sd, &p2pick, sizeof(char), 0);
            printf("Player %d chose:  %c", player, p2pick);
        }

        choice = (player == 1) ? p1pick-'0' : p2pick-'0';

        mark = (player == 1) ? 'X' : 'O'; //depending on who the player is, either us x or o
        /******************************************************************/
        /** little math here. you know the squares are numbered 1-9, but  */
        /* the program is using 3 rows and 3 columns. We have to do some  */
        /* simple math to conver a 1-9 to the right row/column            */
        /******************************************************************/
        row = (int)((choice-1) / ROWS); 
        column = (choice-1) % COLUMNS;

        /* first check to see if the row/column chosen is has a digit in it, if it */
        /* square 8 has and '8' then it is a valid choice                          */

        if (board[row][column] == (choice + '0')) {
            board[row][column] = mark;
        } else {
            printf("Invalid move ");
            player--;
            getchar();
        }
        /* after a move, check to see if someone won! (or if there is a draw */
        i = check_win(board);
        
        player++;
    } while (i ==  - 1); // -1 means no one won
    
    /* print out the board again */
    print_board(board);
        
    if (i == 1) // means a player won!! congratulate them
        printf("==>\aPlayer %d wins\n ", --player);
    else
        printf("==>\aGame draw"); // ran out of squares, it is a draw
    
    return 0;
}


int check_win(char board[ROWS][COLUMNS]) {
    /************************************************************************/
    /* brute force check to see if someone won, or if there is a draw       */
    /* return a 0 if the game is 'over' and return -1 if game should go on  */
    /************************************************************************/
    if (board[0][0] == board[0][1] && board[0][1] == board[0][2] ) { // row matches
        return 1;   
    } else if (board[1][0] == board[1][1] && board[1][1] == board[1][2] ) { // row matches
        return 1;     
    } else if (board[2][0] == board[2][1] && board[2][1] == board[2][2] ) { // row matches
        return 1;     
    } else if (board[0][0] == board[1][0] && board[1][0] == board[2][0] ) { // column
        return 1;    
    } else if (board[0][1] == board[1][1] && board[1][1] == board[2][1] ) { // column
        return 1;  
    } else if (board[0][2] == board[1][2] && board[1][2] == board[2][2] ) { // column
        return 1;
    } else if (board[0][0] == board[1][1] && board[1][1] == board[2][2] ) { // diagonal
        return 1;    
    } else if (board[2][0] == board[1][1] && board[1][1] == board[0][2] ) { // diagonal
        return 1;    
    } else if (board[0][0] != '1' && board[0][1] != '2' && board[0][2] != '3' &&
        board[1][0] != '4' && board[1][1] != '5' && board[1][2] != '6' && 
        board[2][0] != '7' && board[2][1] != '8' && board[2][2] != '9') {

        return 0; // Return of 0 means game over
    } else {
        return  - 1; // return of -1 means keep playing
    }
}


void print_board(char board[ROWS][COLUMNS]) {
    /*****************************************************************/
    /* brute force print out the board and all the squares/values    */
    /*****************************************************************/

    printf("\n\n\n\tCurrent TicTacToe Game\n\n");

    printf("Player 1 (X)  -  Player 2 (O)\n\n\n");


    printf("     |     |     \n");
    printf("  %c  |  %c  |  %c \n", board[0][0], board[0][1], board[0][2]);

    printf("_____|_____|_____\n");
    printf("     |     |     \n");

    printf("  %c  |  %c  |  %c \n", board[1][0], board[1][1], board[1][2]);

    printf("_____|_____|_____\n");
    printf("     |     |     \n");

    printf("  %c  |  %c  |  %c \n", board[2][0], board[2][1], board[2][2]);

    printf("     |     |     \n\n");
}
