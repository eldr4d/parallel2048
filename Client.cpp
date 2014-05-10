#include "Client-comm.hpp"
#include "Board.hpp"


#define DEFAULT_NODELIMIT 200000

Board board;

int GetNodeLimit(int argc, char * argv[])
{
  if (argc < 5) return DEFAULT_NODELIMIT;
  return atoi(argv[4]);
}


void GetHost(int argc, char * argv[], char *host)
{
  if (argc < 4) 
    strcpy(host, "localhost");
  else 
    strcpy(host, argv[3]);
}


int GetPort(int argc, char * argv[])
{
  if (argc < 3) return PORT;
  return atoi(argv[2]);
}


int GetSide(int argc, char *argv[])
{
    
    if (!strcmp(argv[1], "normal") || !strcmp(argv[1], "Normal") ||
       !strcmp(argv[1], "NORMAL"))
       return NORMAL;
    else
        if (!strcmp(argv[1], "placer") || !strcmp(argv[1], "Placer") ||
            !strcmp(argv[1], "PLACER"))
            return PLACER;
        else {
            printf ("Usage: %s <normal|placer> [server-port] [server-host] [nodelimit]\n", argv[0]);
            exit (0);
        }
}


void GetUserMove (int *dir)
{
    bool legal = false;
    
    while (!legal) {
    	char ch;
        printf ("Input move (u,r,d,l): ");
        scanf ("%c", &ch);
        switch(ch){
        	case 'u':
        		*dir = 0;
        		break;
        	case 'r':
        		*dir = 1;
        		break;
        	case 'd':
        		*dir = 2;
        		break;
        	case 'l':
        		*dir = 3;
        		break;
        	default:
        		continue;
        }
        if (board.IsLegalNormalMove(*dir))
            legal = true;
    }
}
            

void GetRandomPlacerMove (int *row, int *col, int* value)
{
    bool legal = false;
    
    while (!legal) {
        *row = (int)(drand48() * BOARD_SIZE);
        *col = (int)(drand48() * BOARD_SIZE);
        *value = (int)(drand48()*2)+1;
        if (board.IsLegalPlacerMove(*row,*col,*value))
            legal = true;
    }
}

int main (int argc, char *argv[])
{
    MsgFromServer   msg;
    int             row, col, dir, socket, value;
    int            side;
    int             totalnodes;
    int             nodelimit = GetNodeLimit(argc, argv);
    unsigned short  port;
    char            host[100];

    srand48 (time(NULL) % (1 << 24));


    if (argc < 2) {
         printf ("Usage: %s <normal|placer> [server-port] [server-host] [nodelimit]\n", 
		 argv[0]);
         exit (0);
    }

    side = GetSide(argc, argv); /* Decide whether the player is WHITE or RED */
    port = GetPort(argc, argv);
    GetHost(argc, argv, host);
    printf("Server Port: %d\n", port);
    printf("Server Host: %s\n", host );
	printf("Side: %d\n",side);
    StartSession (&socket, side, port, host);
		        
	for(int x=0; x<BOARD_SIZE; x++){
		for(int y=0; y<BOARD_SIZE; y++){
			board.grid[x][y] = msg.grid[x][y];
		}
	}

    do {

        StartGame (socket, &msg);

        while (msg.status != GAME_ENDED && msg.status != ABORT) {

			if(side == PLACER){
				GetRandomPlacerMove (&row, &col, &value);
		        SendPlacerAndGetNormalMove (socket, row, col, value, &msg);
		        
				for(int x=0; x<BOARD_SIZE; x++){
					for(int y=0; y<BOARD_SIZE; y++){
						board.grid[x][y] = msg.grid[x][y];
					}
				}
				printf ("My move is %d,%d v = %d\n\n", row, col, value);
			}else{
				GetUserMove(&dir);
				SendNormalAndGetPlacerMove(socket, dir, &msg);
		        
				for(int x=0; x<BOARD_SIZE; x++){
					for(int y=0; y<BOARD_SIZE; y++){
						board.grid[x][y] = msg.grid[x][y];
					}
				}
				printf ("My move is %d\n\n", dir);
			}

			sleep (1);

	
        }
        if (msg.status == GAME_ENDED)
        	printf ("Game ended!!\n");

    } while (msg.status != ABORT);

    EndSession (socket);
    printf ("Session ended!!\n");

    exit (EXIT_SUCCESS);
}

