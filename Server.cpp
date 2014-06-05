#include <unistd.h>
#include "Communication/Comm.hpp"
#include "Communication/Protocol.hpp"
#include "Board/BitBoard.hpp"
#include <iostream>

using namespace std;

int SendEndMessage (int sock1, Status status);
BitBoard_t board;

int ReadFromClient (int filedes)
{
    char *buffer = new char[sizeof(MsgToServer)];
    int nbytes;

    nbytes = read (filedes, buffer, sizeof(MsgToServer));
    if (nbytes < 0) {
        /* Read error. */
        return -1;
    }
    else
        if (nbytes == 0)
            /* End-of-file. */
            return -1;
        else {
            /* Data read. */
            cerr << "Server: got message: `" << buffer << endl;
            return 0;
        }
}


void EndServerComm (int sock1, int sock2)
{
    if (sock1 >= 0)
        SendEndMessage (sock1, ABORT);
    if (sock2 >= 0)
        SendEndMessage (sock2, ABORT);

    usleep (100000);

    /* Clear the valid sockets */
    if (sock1 >= 0) {
        ReadFromClient (sock1); /* Get close socket messages */
        close (sock1);
    }

    if (sock2 >= 0) {
        ReadFromClient (sock2); /* Get close socket messages */
        close (sock2);
    }
}

    
int GetFirstMessage (int sock)
{
    FirstMsgToServer   msg;
    size_t nbytes;

    nbytes = read (sock, (char *)&msg, sizeof(FirstMsgToServer));

    if (nbytes < sizeof(FirstMsgToServer)) {
        /* Read error. */
        exit (EXIT_FAILURE);
    }
    else 
        if (nbytes == 0)
            /* End-of-file. */
            return -1;
        else {
            /* Data read. */
            cout << "Server: got message from client " << sock <<": side=" << (int)msg << endl;            
    
            assert (msg == 0 || msg == 1);
	    
            return msg;
        }
}


void InitServerComm (int *sock, int *normal_sock, int *placer_sock, unsigned short port)
{
    fd_set active_fd_set, read_fd_set;
    int i, clients_connected, messages, sock1 = 0, sock2 = 0;
    int side1, side2;
    struct sockaddr_in clientname;
    int size;
    static bool first_time = true;

    if (!first_time)
        return;

    first_time = false;

    /* Create the socket and set it up to accept connections. */
    *sock = make_socket (port);
    if (listen (*sock, 1) < 0) {
        exit (EXIT_FAILURE);
    }

    /* Initialize the set of active sockets. */
    FD_ZERO (&active_fd_set);
    FD_SET (*sock, &active_fd_set);
    clients_connected = 0;

    /* Initialize the set of active sockets. */
    while (clients_connected < 2) {
        /* Block until input arrives on one or more active sockets. */
        read_fd_set = active_fd_set;
        if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
            exit (EXIT_FAILURE);
        }

        /* Service all the sockets with input pending. */
        for (i = 0; i < FD_SETSIZE; ++i) {
            if (FD_ISSET (i, &read_fd_set)) {
                /* Serve only new clients. Messages from previously       */
                /* connected clients will be handled later.               */
                if (i == *sock) {
                    /* Connection request on original socket. */
                    int new1;
                    size = sizeof (clientname);
                    new1 = accept (*sock,(struct sockaddr *) &clientname,(socklen_t*)&size);
                    if (new1 < 0) {
                        exit (EXIT_FAILURE);
                    }
                    cout << "Server: connect " << new1 << " from host " << inet_ntoa (clientname.sin_addr)<< ", port "<<ntohs (clientname.sin_port)<<"."<<endl; 
                    FD_SET (new1, &active_fd_set);
                    if (clients_connected == 0)
                        sock1 = new1;
                    else
                        sock2 = new1;
                    clients_connected++;
                }
            }
        }
    }

    /* We have two clients - get their first messages */
    messages=0;
    side1 = side2 = -1;
    while (messages < 2) {
        /* Block until input arrives on one or more active sockets. */
        read_fd_set = active_fd_set;
        if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
            exit (EXIT_FAILURE);
        }

        /* Service all the sockets with input pending. */
        for (i = 0; i < FD_SETSIZE; ++i) {
            if (FD_ISSET (i, &read_fd_set)) {
                /* Serve only known clients, ignore the rest. */
                if (i != *sock) {
                    if (i == sock1){
                        if (side1 == -1){
                            side1 = GetFirstMessage(sock1);
                        }else{
                            exit (-1);
                        }
                    }else{
                        if (side2 == -1){
                            side2 = GetFirstMessage(sock2);
                        }else{
                        	cout << "Getting the first message twice" << endl;
                            exit (-1);
                        }
                    }    
                    messages++;
                }
            }
        }
    }

    if (side1 == NORMAL && side2 == PLACER) {
        *normal_sock = sock1;
        *placer_sock = sock2;
    }
    else{
        if (side1 == PLACER && side2 == NORMAL) {
            *placer_sock = sock1;
            *normal_sock = sock2;
        }
        else {
            cerr << "Error, sides do not agree!!" << endl;
            EndServerComm (sock1, sock2);
            exit (-1);
        }
	}
}


int SendEndMessage (int sock, Status status)
{
    MsgFromServer   msg;

    msg.status = status;
    msg.row = -1;           /* Just return something reasonable */
    msg.col = -1;
    
 //    for(int x=0; x<BOARD_SIZE; x++){
 //    	for(int y=0; y<BOARD_SIZE; y++){
 //    		msg.grid[x][y] = board.grid[x][y];
	// 	}
	// }
    
    int nbytes = write (sock, (char *)&msg, sizeof(MsgFromServer));
    if (nbytes < 0) {
        return -1;
    }
    return 0;
}


int DoComm (int sock, MsgFromServer *out_msg, MsgToServer *in_msg)
{
    int nbytes;

    nbytes = write (sock, (char *)out_msg, sizeof(MsgFromServer));
    if (nbytes < 0) {
        return -1;
    }
    else if (nbytes == 0)
        return -2;

    nbytes = read (sock, (char *)in_msg, sizeof(MsgToServer));
    if (nbytes < 0) {
        /* Read error. */
        return -3;
    }
    else if (nbytes == 0)
        return -4;

    return 0;
}


void IllegalMove (int side)
{
    cout << "*** " << ((side == NORMAL) ? "Normal" : "Placer") << " sent an illegal move!! Technical lose ***" << endl;
}


void Crash (int side)
{
    cout << "*** " << ((side == NORMAL) ? "Normal" : "Placer") << " has crashed!! Technical lose ***" << endl;
}


void TimeViolation (int side)
{
    cout << "*** " << ((side == NORMAL) ? "Normal" : "Placer") << " has violated his time limit!! Technical lose ***" << endl;
}

int calculateScore(int tile){
    return tile ? (tile-1)*(1<<tile) : 0;
}

int main (int argc, char *argv[])
{
    srand(time(NULL) % (1 << 24));
    
    int sock, clnt_sock[2];
    double time_left[2], time_limit;
    int row, col, dir, i, games, v;

    MsgFromServer out_msg;
    MsgToServer in_msg;
    bool all_ok, technical_lose;
    unsigned short port;
    int totalNumberOfInitFours = 0;

    all_ok = true;
    technical_lose = false;

    if (argc > 4 || argc == 1) {
    	cout << "Usage: " << argv[0] << " <games> [port] [time]" << endl;
        exit (0);
    }

    games = atoi(argv[1]);
    if (games < 1)
        games = 1;

    port = PORT;
    if (argc > 2)
      port = atoi(argv[2]);

    if (argc < 4)
        time_limit = 300.0;
    else {
        time_limit = atof (argv[3]);
        if (time_limit <= 0.0)
            time_limit = 300.0;
    }


    for (i=0 ; i < games && all_ok; i++) {
    	cout << "Playing game " << i << " out of " << games << endl;
        row = col = dir = v = -1;
        time_left[0] = time_left[1] = time_limit;
		board.clear();
        ptile c = board.placeRandom();
        totalNumberOfInitFours = 0;
        if(c.vlog == 2){
            totalNumberOfInitFours++;
        }
        c = board.placeRandom();
        if(c.vlog == 2){
            totalNumberOfInitFours++;
        } 

        out_msg.board = board;
		cout << board << endl;
		cout << "Normal: " << time_left[0] << " Placer: " << time_left[1] << endl << endl;

        InitServerComm (&sock, &clnt_sock[0], &clnt_sock[1], port);

        while (board.existsNormalMove()) {
			//normal player
            out_msg.status = GIVE_MOVE;
            out_msg.time_left = time_left[NORMAL];
            
            out_msg.row = row;
            out_msg.col = col;
            out_msg.two = v == 1? 1 : 0;
            out_msg.board = board;
            if (DoComm (clnt_sock[NORMAL], &out_msg, &in_msg) < 0) {
                close (clnt_sock[NORMAL]);
                clnt_sock[NORMAL] = -1; /* Invalid socket */
                Crash (NORMAL);
                all_ok = false;
                technical_lose = true;
                break;
            }
			/* Side should move */
			if(!board.tryMove(in_msg.dir)) {
				IllegalMove (NORMAL);
				technical_lose = true;
				break;
			}

			/*if (time_left[NORMAL] < 0) {
				TimeViolation (NORMAL);
				technical_lose = true;
				break;
			}*/
			dir = in_msg.dir;		
	    	/* Decrease time_left[side] */
	    	time_left[NORMAL] = time_left[NORMAL] - 1.0; /* Temporary, will make this better */
            board.score = 0;
            for(unsigned int x = 0; x<BOARD_SIZE; x++){
                for(unsigned int y = 0; y<BOARD_SIZE; y++){
                    board.score += calculateScore(board.getTile(y,x));
                }
            }
            board.score -= totalNumberOfInitFours*4;
			cout << "+-+-+-+-+- Normal -+-+-+-+-+" << endl;
            cout << totalNumberOfInitFours << endl;
            cout << board.getMaxChain() << endl;
			cout << board << endl;
			cout << "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+" << endl;
            
			//Placer player
            out_msg.status = GIVE_MOVE;
			out_msg.time_left = time_left[PLACER];
            
			out_msg.dir = dir;
            out_msg.board = board;

            if (DoComm (clnt_sock[PLACER], &out_msg, &in_msg) < 0) {
                close (clnt_sock[PLACER]);
                clnt_sock[PLACER] = -1; /* Invalid socket */
                Crash (PLACER);
                all_ok = false;
                technical_lose = true;
                break;
            }
			/* Side should move */
			if(board.tryPlace(in_msg.row, in_msg.col, in_msg.two)==false) {
				IllegalMove (PLACER);
				technical_lose = true;
				break;
			}
            if(in_msg.two == false){
                totalNumberOfInitFours++;
            }

			/*if (time_left[PLACER] < 0) {
				TimeViolation(PLACER);
				technical_lose = true;
				break;
			}*/
			row = in_msg.row;
			col = in_msg.col;
			v = in_msg.two==true? 1 : 2;
	    	/* Decrease time_left[side] */
	    	time_left[PLACER] = time_left[PLACER] - 1.0; /* Temporary, will make this better */
	    	
            board.score = 0;
            for(unsigned int x = 0; x<BOARD_SIZE; x++){
                for(unsigned int y = 0; y<BOARD_SIZE; y++){
                    board.score += calculateScore(board.getTile(y,x));
                }
            }
            board.score -= totalNumberOfInitFours*4;

			cout << "+-+-+-+-+- Placer -+-+-+-+-+" << endl;
            cout << totalNumberOfInitFours << endl;
			cout << board << endl;
			cout << "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+" << endl;
        }

        if (!technical_lose){
            cout << "Final score: " << board.score << endl;
        }

        SendEndMessage (clnt_sock[0], GAME_ENDED);
        SendEndMessage (clnt_sock[1], GAME_ENDED);
    }

    EndServerComm (clnt_sock[0], clnt_sock[1]);
    exit (EXIT_SUCCESS);
}

