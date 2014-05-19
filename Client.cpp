#include "Communication/Client-comm.hpp"
#include "Board.hpp"

using namespace std;
Board board;
typedef struct{
	int dir;
	int row, col, v;
}Move;
float negaMax(Board board, bool amIroot, Move *move, player pl, int depth, int color, float alpha, float beta);

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

player getOtherPlayer(player pl){
	return pl == PLACER ? NORMAL : PLACER;
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

double veryVeryGreedyAndStupidEvaluationFunction(Board boardForEv){
	if(boardForEv.CanNormalMove()){
		double v1 = (boardForEv.biggestTileIsInCorner() == true) ? 1 : 0.5;
		double v2 = boardForEv.higherTile;
		return v1*v2*10000;
	}else{
		double v2 = boardForEv.higherTile; 
		return Board::myPow(v2);
		//return (double)boardForEv.score/100.0; //We do not want to prefer the end of game compared to any other move
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

/************************** MiniMax functions *************************/
float negaMax(Board board, bool amIroot, Move *move, player pl, int depth, int color, float alpha, float beta){
    
    player other = getOtherPlayer(pl);
    bool root = false;
    if(amIroot == true){
    	root = true;
    }

    //An ftasame sta katw katw fyla tote gyrname thn katastash
    if(depth == 0 || (pl == NORMAL && !board.CanNormalMove())){
        float temp = color*veryVeryGreedyAndStupidEvaluationFunction(board);
        return temp;
    }

    float alph = alpha, bet = beta;
    float best = -100000;
    if(pl == PLACER){
    	for(unsigned int i=0; i<BOARD_SIZE; i++){
	    	for(unsigned int j=0; j<BOARD_SIZE; j++){
	    		if(!board.IsLegalPlacerMove(i,j,1)){
	    			continue;
	    		}
				for(unsigned int k=1; k<3; k++){
					Board board2 = board;
					board2.DoPlacerMove(i,j,k);
					float v = -negaMax(board2,false, NULL, other, depth - 1, -color, -bet, -alph);
					if(v>best){
						best = v;
						if(root){
							move->dir = -1;
							move->row = i;
							move->col = j;
							move->v = k;
						}
					}
				    if(best > alph)
				        alph = best;
				    if(alph >= bet) { // prun brunch as soon as alpha and beta crosses.
				        return alph; //lower bound
				    }
				}
	    	}
    	}
    }else{
    	for(unsigned int d=UP; d<DIR_SIZE; d++){
	    	if(!board.IsLegalNormalMove(d)){
	    		continue;
	    	}
			Board board2 = board;
			board2.DoNormalMove(d);
			float v = -negaMax(board2,false, NULL, other, depth - 1, -color, -bet, -alph);
			if(v>best){
				best = v;
				if(root){
					move->dir = d;
					move->row = -1;
				}
			}
            if(best > alph)
                alph = best;
            if(alph >= bet) { // prun brunch as soon as alpha and beta crosses.
                return alph; //lower bound
            }
    	}
    }
    return best;
}

int ExploreTree(Board board, Move *move, player pl)
{
   	float alpha = -1000;
    float beta = 1000;

    int depth = 9;


	float bestcost;
	if(pl == NORMAL){
		bestcost = negaMax(board,true, move, NORMAL, depth, 1, -10000, +10000);
   	 	cout << "Best dir = " << move->dir << " and best cost = " << bestcost << endl;
	}else{
		bestcost = -negaMax(board,true, move, PLACER, depth, -1, -10000, +10000);
   	 	cout << "Best row = " << move->row << ",col = " << move->col << " ,value = " << move->v << " and best cost = " << bestcost << endl;
	}
    
    return 0;
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
		  
	

    do {

        StartGame (socket, &msg);
		board.higherTile = 1;
		for(int x=0; x<BOARD_SIZE; x++){
			for(int y=0; y<BOARD_SIZE; y++){
				board.grid[x][y] = msg.grid[x][y];
				if(msg.grid[x][y] == 2){
					board.higherTile = 2;
				}
			}
		}
		board.PrettyPrint();
		
        while (msg.status != GAME_ENDED && msg.status != ABORT) {
			if(side == PLACER){
				GetRandomPlacerMove (&row, &col, &value);
				board.DoPlacerMove(row,col,value);
				board.PrettyPrint();
		        SendPlacerAndGetNormalMove (socket, row, col, value, &msg);
		        board.DoNormalMove(msg.dir);
				//board.PrettyPrint();
				cout << "My move is " << row << "," << col << " v = " << value << endl << endl;
			}else{
				//GetUserMove(&dir);
				Move move;
				ExploreTree(board, &move, NORMAL);
				cout << "My move is " << move.dir << endl << endl;
		        board.DoNormalMove(move.dir);
				board.PrettyPrint();
				SendNormalAndGetPlacerMove(socket, move.dir, &msg);
				int value = ((msg.two == true) ? 1 : 2);
				board.DoPlacerMove(msg.row,msg.col,value);
				//board.PrettyPrint();
			}


			sleep (0.2);

	
        }
        if (msg.status == GAME_ENDED){
        	board.InitializeBoard();
        	cout << "Game ended!!" << endl;
        }

    } while (msg.status != ABORT);

    EndSession (socket);
   	cout << "Session ended!!" << endl;


    exit (EXIT_SUCCESS);
}

