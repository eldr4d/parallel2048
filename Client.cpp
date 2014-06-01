#include "Communication/Client-comm.hpp"
#include "BitBoard.hpp"
#include <chrono>
#include <ctime>
#include "ThreadPool/ThreadPool.hpp"

//#define NDEBUG
#define NUM_OF_THREADS 5

using namespace std;

typedef BitBoard<4u> BitBoard_t;
BitBoard_t board;

typedef struct{
    int dir;
    int row, col, v;
}Move;

typedef struct{
    player pl;
    bool color;
    int32_t alpha, beta, depth;
    bool root;
    BitBoard_t board;
    Move *move;
    int32_t *writeResult;
}maskedArguments;

int32_t negaScout(BitBoard_t board, bool amIroot, Move *move, player pl, int32_t depth, bool color, int32_t alpha, int32_t beta);

void negaScoutWrapper(maskedArguments args){
    int32_t data = negaScout(args.board, args.root, args.move, args.pl, args.depth, args.color, args.alpha, args.beta);
    *(args.writeResult) = data;
}

ThreadPool<maskedArguments> myThreadPool(NUM_OF_THREADS,&negaScoutWrapper);

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

int32_t veryVeryGreedyAndStupidEvaluationFunction(BitBoard_t boardForEv){	
	if(boardForEv.existsPlacerMove()){
		int32_t v1;
		int32_t v2 = boardForEv.getHigherTile();
		if(v2 == boardForEv.getTile(0,0) || v2 == boardForEv.getTile(0,BOARD_SIZE-1) || v2 == boardForEv.getTile(BOARD_SIZE-1,0) || v2 == boardForEv.getTile(BOARD_SIZE-1,BOARD_SIZE-1)){
			v1 = 1;
		}else{
			v1 = 0;
		}
		return ((1<<(v2))<<v1)+1;
	}else{
		//int32_t v2 = boardForEv.getHigherTile(); 
		return 0;
		//return (double)boardForEv.score/100.0; //We do not want to prefer the end of game compared to any other move
	}
}         


int32_t negaMaxExpandOneChild(BitBoard_t board, bool root, Move *move, player pl, int32_t depth, bool color, int32_t alpha, int32_t beta){
     player other = getOtherPlayer(pl);

    int32_t alph = alpha, bet = beta;
    vector<int32_t> best;
    vector<unsigned int> moves;
    if(pl == PLACER){
       ;
    }else{
        best.resize(4);
        for(unsigned int d=LEFT; d<DIR_SIZE; d++){
            BitBoard_t board2 = board;
            if(!board2.tryMove(d)){
                continue;
            }
            moves.push_back(d);
            best.push_back(-99999);
            maskedArguments args;
            args.alpha = -bet;
            args.beta = -alph;
            args.pl = other;
            args.depth = depth-1;
            args.root = false;
            args.board = board2;
            args.move = move;
            args.writeResult = &(best.back());
            args.pl = NORMAL;
            args.color = true;
            myThreadPool.useNewThread(args);
        }

        int32_t bestCost = -numeric_limits<int32_t>::max();
        int32_t bestMove;
        for(unsigned int i=0; i<best.size(); i++){
            while(best[i] == -99999){
                usleep(10);
                //cout << "Koimame pali " << best.size() << " " << best[i]  << endl;
            }
            best[i] = -best[i];
            if(best[i] > bestCost){
                bestMove = moves[i];
                bestCost = best[i];
            }
        }
        move->dir = bestMove;
        return bestCost;
        
    }
    return 0;
}
/************************** MiniMax functions *************************/
int32_t negaScout(BitBoard_t board, bool root, Move *move, player pl, int32_t depth, bool color, int32_t alpha, int32_t beta){
    
    player other = getOtherPlayer(pl);

    //An ftasame sta katw katw fyla tote gyrname thn katastash
    if(depth == 0){
        int32_t temp = veryVeryGreedyAndStupidEvaluationFunction(board);
        temp = color ? temp : -temp;
        return temp;
    }

#ifndef NDEBUG
	board.assert_state();
#endif
    int32_t alph = alpha, bet = beta;
    int32_t best = -numeric_limits<int32_t>::max();
    bool bestChanged = false;
    bool firstChild = true;
    if(pl == PLACER){
        uint64 empty = board.getEmptyTiles();
        while (empty){
#ifndef NDEBUG
            BitBoard_t testb = board;
#endif
            uint64 p = pop_lsb(empty);
            for (unsigned int is2 = 0; is2 < 2 ; ++is2){ // 2 or 4
                board.makePlace(p, is2);
                int32_t score;
                if(firstChild == true){
                	score = -negaScout(board, false, NULL, other, depth - 1, !color, -bet, -alph);
                	//firstChild = false;
                }else{
                	score = -negaScout(board, false, NULL, other, depth - 1, !color, -alph-1, -alph);
                	if(alph < score && score < bet){
                		score = -negaScout(board, false, NULL, other, depth - 1, !color, -bet, -score);
                	}
                }
                board.undoPlace(p, is2);
#ifndef NDEBUG
                assert(testb == board);
#endif
                if(score>best){
              		bestChanged = true;
                    best = score;
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
    }else{
        bool noMove = true;
    	for(unsigned int d=LEFT; d<DIR_SIZE; d++){
            BitBoard_t board2 = board;
	    	if(!board2.tryMove(d)){
	    		continue;
	    	}
            noMove = false;
            int32_t score;
            if(firstChild == true){
            	score = -negaScout(board2, false, NULL, other, depth - 1, !color, -bet, -alph);
               	firstChild = false;
            }else{
            	score = -negaScout(board2, false, NULL, other, depth - 1, !color, -alph-1, -alph);
            	if(alph < score && score < bet){
            		score = -negaScout(board2, false, NULL, other, depth - 1, !color, -bet, -score);
            	}
            }
			if(score>best){
				best = score;
              	bestChanged = true;
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
            int32_t temp = veryVeryGreedyAndStupidEvaluationFunction(board);
            temp = color ? temp : -temp;
            return temp;
        }
    }
    return alph;
}

int32_t ExploreTree(BitBoard_t board, Move *move, player pl)
{

    int32_t depth = 9;

    maskedArguments args;
    args.alpha = -(numeric_limits<int32_t>::max()-100000);
    args.beta = (numeric_limits<int32_t>::max()-100000);
    args.depth = depth;
    args.root = true;
    args.board = board;
    args.move = move;



    std::chrono::time_point<std::chrono::system_clock> start, end;

    std::chrono::duration<double> elapsed_seconds;
	double totalSeconds = 0.2;
    do{
    	cout << depth << endl;
		start = std::chrono::system_clock::now();
		int32_t bestcost = -99999;

        args.writeResult = &bestcost;
		if(pl == NORMAL){
            //args.pl = NORMAL;
            //args.color = true;
            //myThreadPool.useNewThread(args);
            //while(bestcost == -99999)
            //    usleep(10);
			bestcost = negaMaxExpandOneChild(board,true, move, NORMAL, depth, true, -(numeric_limits<int32_t>::max()-100000), numeric_limits<int32_t>::max()-100000);
	   	 	cout << "Best dir = " << move->dir << " and best cost = " << bestcost << endl;
		}else{
            args.pl = PLACER;
            args.color = false;
            myThreadPool.useNewThread(args);
            while(bestcost == -99999)
                usleep(10);
            bestcost = -bestcost;
			//bestcost = -negaScout(board,true, move, PLACER, depth, false, -(numeric_limits<int32_t>::max()-100000), numeric_limits<int32_t>::max()-100000);
	   	 	cout << "Best row = " << move->row << ",col = " << move->col << " ,value = " << move->v << " and best cost = " << bestcost << endl;
		}
		end = std::chrono::system_clock::now();
		elapsed_seconds= end-start;
		totalSeconds -= elapsed_seconds.count();
		depth++;
		
    }while(totalSeconds>0);


    return 0;
}


int main (int argc, char *argv[])
{

    MsgFromServer   msg;
    int             dir, socket, side;
    unsigned short  port;
    char            host[100];

    srand(time(NULL) % (1 << 24));


    if(argc < 2) {
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
		board = msg.board;
	    cout << "Starting board = " << endl << board << endl;
        while (msg.status != GAME_ENDED && msg.status != ABORT) {
			if(side == PLACER){
				ptile c = board.placeRandom();
               	cout << board << endl;
		       	SendPlacerAndGetNormalMove (socket, c.row, c.col, c.vlog, &msg);
				cout << "My move is " << c.row << "," << c.col << " v = " << (1 << c.vlog) << endl << endl;
		       	/*
				Move move;
				ExploreTree(board, &move, PLACER);
				cout << "My move is " << move.row << " " << move.col << " " << move.v << endl << endl;
				int twoOrFor = move.v == 0 ? 2 : 1;
               	board.tryPlace(move.row,move.col,move.v);
				cout << board << endl;
		       	SendPlacerAndGetNormalMove (socket, move.row, move.col, twoOrFor, &msg);*/

                if (msg.status == GAME_ENDED || msg.status == ABORT) break;
#ifndef NDEBUG
                bool legal =  board.tryMove(msg.dir);
                cout << board << endl;
                assert(legal);
#else
               	board.tryMove(msg.dir);
#endif
				//board.PrettyPrint();
			}else{
				Move move;
				ExploreTree(board, &move, NORMAL);
				cout << "My move is " << move.dir << endl << endl;
		        board.move(move.dir);
				cout << board << endl;
        		SendNormalAndGetPlacerMove(socket, move.dir, &msg);
                if (msg.status == GAME_ENDED || msg.status == ABORT) break;
#ifndef NDEBUG
                bool legal =  board.tryPlace(msg.row,msg.col,msg.two);
                assert(legal);
#else
               	board.tryPlace(msg.row,msg.col,msg.two);
#endif
			}
			//sleep (0.2);

	
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

