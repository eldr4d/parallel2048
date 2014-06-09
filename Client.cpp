#include "NegaScout/TranspositionTable.hpp"
#include "Communication/Client-comm.hpp"
#include "Board/BitBoard.hpp"
#include <chrono>
#include <ctime>
#include "NegaScout/MoveIterator.hpp"
#include "NegaScout/Search.hpp"
#include <atomic>

using namespace std;

TranspositionTable tt;
BitBoard_t board;

std::atomic<uint64_t> horizonNodes;
std::atomic<uint64_t> totalNodes;

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

int32_t ExploreTree(BitBoard_t board, Move *move, player pl)
{

    int32_t depth = 10;

    horizonNodes = 0;
    totalNodes   = 0;


    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::chrono::time_point<std::chrono::system_clock> glob_start = 
                                            std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds;
	double totalSeconds = 0.2;
    do{
		start = std::chrono::system_clock::now();
		int32_t bestcost(-99999);

        tt.preparePVposition(board);
		if(pl == NORMAL){
            move->dir = 1000;                                                       //TODO REMOVE

			bestcost = negaScout<NORMAL, PARALLELIMPL>(board, depth, MIN_TT_SCORE+1, MAX_TT_SCORE-1, true);
		}else{
			bestcost = -negaScout<PLACER, true>(board, depth, MIN_TT_SCORE+1, MAX_TT_SCORE-1, true);
        }
        tt.extractBest(board, pl, move);

        //based on UCI format
        cout << "info";
        cout << " score " << bestcost;
        cout << " depth " << depth;
        cout << " time "  << chrono::duration_cast<chrono::milliseconds>(
                                    chrono::system_clock::now() - glob_start
                                ).count();
#ifndef NDEBUG
        cout << " nodes " << totalNodes;
#endif
        cout << " pv "    << tt.extractPV(board, pl, 120);
        cout << endl;
        //end of info message

        end = std::chrono::system_clock::now();
        elapsed_seconds= end-start;
        totalSeconds -= elapsed_seconds.count();
		depth++;
    }while(totalSeconds>0 && depth < 50);

    return 0;
}


int main (int argc, char *argv[])
{

    MsgFromServer   msg;
    int             socket, side;
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
                /*uint64 f = board.getEmptyTiles();
                f = pop_lsb(f);
                board.makePlace(f);
                Move m;
                BitBoard_t::intToMove(&m, (int) square(f), PLACER);
                SendPlacerAndGetNormalMove (socket, m.row, m.col, m.v, &msg);
                cout << "My move is " << m.row << "," << m.col << " v = " << (1 << m.v) << endl << endl;*/
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

