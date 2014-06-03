#include "TranspositionTable.hpp"
#include "Communication/Client-comm.hpp"
#include "BitBoard.hpp"
#include <chrono>
#include <ctime>
#include "ThreadPool/ThreadPool.hpp"
//#define NDEBUG
#define NUM_OF_THREADS 4

using namespace std;

TranspositionTable tt;

typedef BitBoard<4u> BitBoard_t;
BitBoard_t board;

typedef struct{
    int dir;
    int row, col, v;
}Move;

typedef struct{
    player pl;
    int32_t alpha, beta, depth;
    BitBoard_t board;
    Move *move;
    int32_t *writeResult;
}maskedArguments;

int32_t negaScout(BitBoard_t board, Move *move, player pl, int32_t depth, int32_t alpha, int32_t beta);

void negaScoutWrapper(maskedArguments args){
    int32_t data = negaScout(args.board, args.move, args.pl, args.depth, args.alpha, args.beta);
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
    bool inCorner = false;
	int32_t v2 = boardForEv.getHigherTile(&inCorner);
    int32_t score = (v2 << 6) + 1;
	if(inCorner){
		score <<= 2;
	}
    int tmp = boardForEv.countFreeTiles() - 7;
    score += (tmp < 0) ? -tmp : tmp;
    assert(score >= -5);
	return score;
}         


int32_t negaMaxExpandOneChild(BitBoard_t board, Move *move, player pl, int32_t depth, int32_t alpha, int32_t beta){
     player other = getOtherPlayer(pl);

    int32_t alph = alpha, bet = beta;
    vector<int32_t> best;
    vector<unsigned int> moves;
    if(pl == PLACER){
       ;
    }else{
        best.reserve(4);
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
            args.board = board2;
            args.move = move;
            args.writeResult = &(best.back());
            myThreadPool.useNewThread(args);
        }

        int32_t bestCost = -numeric_limits<int32_t>::max();
        int32_t bestMove;
        for(unsigned int i=0; i<best.size(); i++){
            while(best[i] == -99999){
                // usleep(10);
                //cout << "Koimame pali " << best.size() << " " << best[i]  << endl;
            }
            best[i] = -best[i];
            if(best[i] > bestCost){
                assert(moves[i] < 4);
                bestMove = moves[i];
                bestCost = best[i];
            }
        }
        move->dir = bestMove;
        return bestCost;
        
    }
    return 0;
}

int32_t search_deeper(BitBoard_t board, player other, int32_t depth, int32_t alpha, int32_t beta, bool &firstChild){
    int32_t score;
    if(firstChild == true){
        score = -negaScout(board, NULL, other, depth, -beta, -alpha);
    } else {
        score = -negaScout(board, NULL, other, depth, -alpha-1, -alpha);
        if(alpha < score){
            score = -negaScout(board, NULL, other, depth, -beta, -score);
        }
    }
    return score;
}

/************************** MiniMax functions *************************/
int32_t negaScout(BitBoard_t board, Move *move, player pl, int32_t depth, int32_t alpha, int32_t beta){
    
    player other = getOtherPlayer(pl);

    //An ftasame sta katw katw fyla tote gyrname thn katastash
    if(depth == 0){
        int32_t temp = veryVeryGreedyAndStupidEvaluationFunction(board);
        temp = (pl == NORMAL) ? temp : -temp;
        return temp;
    }

    uint64 hash = board.getHash();
    int killerMove = -1;
    if (move == NULL){
        killerMove = tt.retrieveTTEntry(hash, depth, alpha, beta, pl == PLACER);
        //if (ttResult == ttExactScoreReturned) return alpha;
        if (alpha >= beta) {
            // statistics(++hashHitCutOff);
            return alpha;
        }
    }
    int bmove = -1;
    unsigned int bis2 = 0;


#ifndef NDEBUG
	board.assert_state();
#endif
    int32_t alph = alpha, bet = beta;
    int32_t best = -numeric_limits<int32_t>::max();
    bool firstChild = true;

    if(pl == PLACER){
        uint64 empty = board.getEmptyTiles();
        while (empty){
            uint64 p = pop_lsb(empty);
            for (unsigned int is2 = 0; is2 < 2 ; ++is2){ // 2 or 4
                board.makePlace(p, is2);
                int32_t score = search_deeper(board, other, depth - 1, alph, bet, firstChild);
                board.undoPlace(p, is2);

                if (score >= bet){
                    assert(move == NULL);
                    tt.addTTEntry(hash, depth, square(p), score, pl == PLACER, Cut_Node);
                    return bet;                         //fail-hard beta cut-off
                }
                firstChild = false;
                if (score > alph){
                    alph = score;
                    bmove = square(p);
                    bis2 = is2;
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
            int32_t score = search_deeper(board2, other, depth - 1, alph, bet, firstChild);

            if (score >= bet){
                assert(move == NULL);
                tt.addTTEntry(hash, depth, 1 << d, score, pl == PLACER, Cut_Node);
                return bet;                             //fail-hard beta cut-off
            }
            firstChild = false;
            if (score > alph){
                alph = score;
                bmove = 1 << d;
            }
    	}
        if (noMove){
            bool a;
            return board.getHigherTile(&a) << 2;
        }
    }
    if(move){
        if (pl == PLACER){
            ptile pt = BitBoard_t::mask2xy(1 << bmove);
            move->dir = -1;
            move->row = pt.row;
            move->col = pt.col;
            move->v = bis2;
        } else {
            move->dir = square(bmove);
            move->row = -1;
        }
    }
    NodeType nt = PV__Node;
    if (bmove < 0){ //All-Node
        nt    = All_Node;
        bmove = 0;
    }
    tt.addTTEntry(hash, depth, bmove, alph, pl == PLACER, nt);
    return alph;
}

int32_t ExploreTree(BitBoard_t board, Move *move, player pl)
{

    int32_t depth = 7;

    maskedArguments args;
    args.alpha = -(numeric_limits<int32_t>::max()-100000);
    args.beta = (numeric_limits<int32_t>::max()-100000);
    args.depth = depth;
    args.board = board;
    args.move = move;



    std::chrono::time_point<std::chrono::system_clock> start, end;

    std::chrono::duration<double> elapsed_seconds;
	double totalSeconds = 0.2;
    do{
		start = std::chrono::system_clock::now();
		int32_t bestcost = -99999;

        args.writeResult = &bestcost;
		if(pl == NORMAL){
            //args.pl = NORMAL;
            //args.color = true;
            //myThreadPool.useNewThread(args);
            //while(bestcost == -99999)
            //    usleep(10);
            move->dir = 1000;
			bestcost = negaMaxExpandOneChild(board, move, NORMAL, depth, -(numeric_limits<int32_t>::max()-100000), numeric_limits<int32_t>::max()-100000);
	   	 	cout << "Best dir = " << move->dir << " and best cost = " << bestcost <<  " @depth " << depth << endl;
		}else{
            // args.pl = PLACER;
            // args.color = false;
            // myThreadPool.useNewThread(args);
            // while(bestcost == -99999)
            //     usleep(10);
            // bestcost = -bestcost;
			bestcost = -negaScout(board, move, PLACER, depth, -(numeric_limits<int32_t>::max()-100000), (numeric_limits<int32_t>::max()-100000));
	   	 	cout << "Best row = " << move->row << ",col = " << move->col << " ,value = " << move->v << " and best cost = " << bestcost <<  " @depth " << depth << endl;
		}
		end = std::chrono::system_clock::now();
		elapsed_seconds= end-start;
		totalSeconds -= elapsed_seconds.count();
		depth++;
		
    }while(totalSeconds>0 && depth < 60);


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

