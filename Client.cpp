#include "Communication/Client-comm.hpp"
#include "Board.hpp"

using namespace std;
Board board;

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
            cout << "Usage: " << argv[0] << " <normal|placer> [server-port] [server-host] [nodelimit]" << endl;
            exit (0);
        }
}


void GetUserMove (int *dir)
{
    bool legal = false;
    
    while (!legal) {
    	char ch;
        cout << "Input move (u,r,d,l): " << endl;
        cin >> ch;
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
    unsigned short  port;
    char            host[100];

    srand48 (time(NULL) % (1 << 24));


    if (argc < 2) {
		cout << "Usage: " << argv[0] << " <normal|placer> [server-port] [server-host] [nodelimit]" << endl;
		exit (0);
    }

    side = GetSide(argc, argv); /* Decide whether the player is WHITE or RED */
    port = GetPort(argc, argv);
    GetHost(argc, argv, host);
    cout << "Server Port: " << port << endl;
    cout << "Server Host: " << host << endl;
    cout << "Side: " << side << endl;

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
				cout << "My move is " << row << "," << col << " v = " << value << endl << endl;
			}else{
				GetUserMove(&dir);
				SendNormalAndGetPlacerMove(socket, dir, &msg);
		        
				for(int x=0; x<BOARD_SIZE; x++){
					for(int y=0; y<BOARD_SIZE; y++){
						board.grid[x][y] = msg.grid[x][y];
					}
				}
				cout << "My move is " << dir << endl << endl;
			}

			sleep (1);

	
        }
        if (msg.status == GAME_ENDED)
        	cout << "Game ended!!" << endl;

    } while (msg.status != ABORT);

    EndSession (socket);
   	cout << "Session ended!!" << endl;


    exit (EXIT_SUCCESS);
}

