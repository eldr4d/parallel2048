#include "TranspositionTable.hpp"
#include "Communication/Client-comm.hpp"
#include "BitBoard.hpp"
#include <chrono>
#include <ctime>
#include "ThreadPool/ThreadPool.hpp"
#include "MoveIterator.hpp"
#include "Search.hpp"
//#define NDEBUG

using namespace std;

TranspositionTable tt;
BitBoard_t board;

uint64_t horizonNodes;
uint64_t totalNodes;

typedef struct{
    player pl;
    int32_t alpha, beta, depth;
    BitBoard_t board;
    Move *move;
    int32_t *writeResult;
}maskedArguments;

// int32_t negaScout(BitBoard_t board, Move *move, player pl, int32_t depth, int32_t alpha, int32_t beta);

void negaScoutWrapper(maskedArguments args){
    int32_t data;
    if (args.pl == player::PLACER){
        data = negaScout<player::PLACER>(args.board, args.depth, args.alpha, args.beta);
    } else {
        data = negaScout<player::NORMAL>(args.board, args.depth, args.alpha, args.beta);
    }
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


void GetUserMove (int *dir)
{
    bool legal = false;
    
    while (!legal) {
    	char ch;
        cout << "Input move (u,r,d,l): " << endl;
        cin >> ch;
        switch(ch){
        	case 'u':
        		*dir = bitNormalMove::b_up;
        		break;
        	case 'r':
        		*dir = bitNormalMove::b_right;
        		break;
        	case 'd':
        		*dir = bitNormalMove::b_down;
        		break;
        	case 'l':
        		*dir = bitNormalMove::b_left;
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


// int32_t negaMaxExpandOneChild(BitBoard_t board, Move *move, player pl, int32_t depth, int32_t alpha, int32_t beta){
//      player other = getOtherPlayer(pl);

//     int32_t alph = alpha, bet = beta;
//     vector<int32_t> best;
//     vector<unsigned int> moves;
//     if(pl == PLACER){
//        ;
//     }else{
//         best.reserve(4);
//         for(unsigned int d=LEFT; d<DIR_SIZE; d++){
//             BitBoard_t board2 = board;
//             if(!board2.tryMove(d)){
//                 continue;
//             }
//             moves.push_back(d);
//             best.push_back(-99999);
//             maskedArguments args;
//             args.alpha = -bet;
//             args.beta = -alph;
//             args.pl = other;
//             args.depth = depth-1;
//             args.board = board2;
//             args.move = move;
//             args.writeResult = &(best.back());
//             myThreadPool.useNewThread(args);
//         }

//         int32_t bestCost = -numeric_limits<int32_t>::max();
//         int32_t bestMove;
//         for(unsigned int i=0; i<best.size(); i++){
//             while(best[i] == -99999){
//                 // usleep(10);
//                 //cout << "Koimame pali " << best.size() << " " << best[i]  << endl;
//             }
//             best[i] = -best[i];
//             if(best[i] > bestCost){
//                 assert(moves[i] < 4);
//                 bestMove = moves[i];
//                 bestCost = best[i];
//             }
//         }
//         move->dir = bestMove;
//         return bestCost;
        
//     }
//     return 0;
// }

// template<player other>
// int32_t search_deeper(BitBoard_t board, int32_t depth, int32_t alpha, int32_t beta, bool &firstChild){
//     int32_t score;
//     if(firstChild == true){
//         score = -negaScout<other>(board, NULL, other, depth, -beta, -alpha);
//     } else {
//         score = -negaScout<other>(board, NULL, other, depth, -alpha-1, -alpha);
//         if(alpha < score){
//             score = -negaScout<other>(board, NULL, other, depth, -beta, -score);
//         }
//     }
//     return score;
// }


int32_t ExploreTree(BitBoard_t board, Move *move, player pl)
{

    int32_t depth = 7;

    maskedArguments args;
    args.alpha = -(numeric_limits<int32_t>::max()-100000);
    args.beta = (numeric_limits<int32_t>::max()-100000);
    args.depth = depth;
    args.board = board;
    args.move = move;
    
    horizonNodes = 0;
    totalNodes   = 0;


    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::chrono::time_point<std::chrono::system_clock> glob_start = 
                                            std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds;
	double totalSeconds = 0.2;
    do{
		start = std::chrono::system_clock::now();
		int32_t bestcost = -99999;

        args.writeResult = &bestcost;
        tt.preparePVposition(board);
		if(pl == NORMAL){
            //args.pl = NORMAL;
            //args.color = true;
            //myThreadPool.useNewThread(args);
            //while(bestcost == -99999)
            //    usleep(10);
            move->dir = 1000;
			bestcost = negaScout<NORMAL>(board, depth, -(numeric_limits<int32_t>::max()-100000), numeric_limits<int32_t>::max()-100000);
		}else{
            // args.pl = PLACER;
            // args.color = false;
            // myThreadPool.useNewThread(args);
            // while(bestcost == -99999)
            //     usleep(10);
            // bestcost = -bestcost;
			bestcost = -negaScout<PLACER>(board, depth, -(numeric_limits<int32_t>::max()-100000), (numeric_limits<int32_t>::max()-100000));
        }
        tt.extractBest(board, pl, move);

        //based on UCI format
        cout << "info";
        cout << " score " << bestcost;
        cout << " depth " << depth;
        cout << " time "  << chrono::duration_cast<chrono::milliseconds>(
                                    chrono::system_clock::now() - glob_start
                                ).count();
        cout << " nodes " << totalNodes;
        cout << " pv "    << tt.extractPV(board, pl, 120);
        cout << endl;
        //end of info message

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
                cout << "My move is " << move << endl << endl;
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

