#include "Communication/Client-comm.hpp"
#include "BitBoard.hpp"

using namespace std;
typedef BitBoard<4u> BitBoard_t;
BitBoard_t board;
typedef struct{
	int dir;
	int row, col, v;
}Move;
float negaMax(BitBoard_t board, bool amIroot, Move *move, player pl, int depth, int color, float alpha, float beta);

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
        		*dir = UP;
        		break;
        	case 'r':
        		*dir = RIGHT;
        		break;
        	case 'd':
        		*dir = DOWN;
        		break;
        	case 'l':
        		*dir = LEFT;
        		break;
        	default:
        		continue;
        }
        if (board.existsMove(*dir))
            legal = true;
    }
}

double veryVeryGreedyAndStupidEvaluationFunction(BitBoard_t boardForEv){
	return 0;
 //    if(boardForEv.CanNormalMove()){
	// 	double v1 = (boardForEv.biggestTileIsInCorner() == true) ? 1 : 0.5;
	// 	double v2 = boardForEv.higherTile;
	// 	return v1*v2*10000;
	// }else{
	// 	double v2 = boardForEv.higherTile; 
	// 	return Board::myPow(v2);
	// 	//return (double)boardForEv.score/100.0; //We do not want to prefer the end of game compared to any other move
	// }
}         

// void GetRandomPlacerMove (int *row, int *col, int* value)
// {
//     bool legal = false;
    
//     while (!legal) {
//         *row = (int)(drand48() * BOARD_SIZE);
//         *col = (int)(drand48() * BOARD_SIZE);
//         *value = (int)(drand48()*2)+1;
//         if (board.IsLegalPlacerMove(*row,*col,*value))
//             legal = true;
//     }
// }

/************************** MiniMax functions *************************/
float negaMax(BitBoard_t board, bool root, Move *move, player pl, int depth, int color, float alpha, float beta){
    
    player other = getOtherPlayer(pl);

    //An ftasame sta katw katw fyla tote gyrname thn katastash
    if(depth == 0){
        float temp = color*veryVeryGreedyAndStupidEvaluationFunction(board);
        return temp;
    }

    float alph = alpha, bet = beta;
    float best = -100000;
    if(pl == PLACER){
        uint64 empty = board.getEmptyTiles();
        while (empty){
#ifndef NDEBUG
            BitBoard_t testb = board;
#endif
            uint64 p = pop_lsb(empty);
            for (unsigned int is2 = 0; is2 < 2 ; ++is2){ // 2 or 4
                board.makePlace(p, is2);
                float v = -negaMax(board, false, NULL, other, depth - 1, -color, -bet, -alph);
                board.undoPlace(p, is2);
                assert(testb == board);
                if(v>best){
                    best = v;
                    if(root){
                        ptile pt = BitBoard_t::mask2xy(p);
                        move->dir = -1;
                        move->row = pt.row;
                        move->col = pt.col;
                        move->v = is2;
                    }
                }
                if(best > alph)
                    alph = best;
                if(alph >= bet) { // prun brunch as soon as alpha and beta crosses.
                    return alph; //lower bound          //fail-soft cut-off
                }
            }
        }
    // 	for(unsigned int i=0; i<BOARD_SIZE; i++){
	   //  	for(unsigned int j=0; j<BOARD_SIZE; j++){
    //             for (unsigned int k = 0; k < 2 ; ++k){
    //                 BitBoard_t board2 = board;
	   //  		    if(!board2.tryPlace(i,j,k)) continue;
				// 	float v = -negaMax(board2,false, NULL, other, depth - 1, -color, -bet, -alph);
				// 	if(v>best){
				// 		best = v;
				// 		if(root){
				// 			move->dir = -1;
				// 			move->row = i;
				// 			move->col = j;
				// 			move->v = k;
				// 		}
				// 	}
				//     if(best > alph)
				//         alph = best;
				//     if(alph >= bet) { // prun brunch as soon as alpha and beta crosses.
				//         return alph; //lower bound
				//     }
				// }
	   //  	}
    // 	}
    }else{
        bool noMove = true;
    	for(unsigned int d=LEFT; d<DIR_SIZE; d++){
            BitBoard_t board2 = board;
	    	if(!board2.tryMove(d)) continue;
            noMove = false;
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
                return alph; //lower bound          //fail-soft cut-off
            }
    	}
        if (noMove){
            float temp = color*veryVeryGreedyAndStupidEvaluationFunction(board);
            return temp;
        }
    }
    return best;
}

int ExploreTree(BitBoard_t board, Move *move, player pl)
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
    int             dir, socket;
    int             side;
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
		// board.higherTile = 1;
		// for(int x=0; x<BOARD_SIZE; x++){
		// 	for(int y=0; y<BOARD_SIZE; y++){
		// 		board.grid[x][y] = msg.grid[x][y];
		// 		if(msg.grid[x][y] == 2){
		// 			board.higherTile = 2;
		// 		}
		// 	}
		// }
        cout << board << endl;
		
        while (msg.status != GAME_ENDED && msg.status != ABORT) {
			if(side == PLACER){
				ptile c = board.placeRandom();
                cout << board << endl;
		        SendPlacerAndGetNormalMove (socket, c.row, c.col, c.vlog, &msg);
                if (msg.status == GAME_ENDED || msg.status == ABORT) break;
#ifndef NDEBUG
                bool legal = 
#endif
                board.tryMove(msg.dir);
                assert(legal);
				//board.PrettyPrint();
				cout << "My move is " << c.row << "," << c.col << " v = " << (1 << c.vlog) << endl << endl;
			}else{
				//GetUserMove(&dir);
				Move move;
				ExploreTree(board, &move, NORMAL);
				cout << "My move is " << move.dir << endl << endl;
		        board.move(move.dir);
				cout << board << endl;
				SendNormalAndGetPlacerMove(socket, move.dir, &msg);
                if (msg.status == GAME_ENDED || msg.status == ABORT) break;
#ifndef NDEBUG
                bool legal = 
#endif
				board.tryPlace(msg.row,msg.col,msg.two);
                assert(legal);
				//board.PrettyPrint();
			}


			sleep (0.2);

	
        }
        if (msg.status == GAME_ENDED){
        	board.initialize();
        	cout << "Game ended!!" << endl;
        }

    } while (msg.status != ABORT);

    EndSession (socket);
   	cout << "Session ended!!" << endl;


    exit (EXIT_SUCCESS);
}

