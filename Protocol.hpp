#include "Board.hpp"

/* Messages flowing between the server and the clients */

#ifndef _PROTOCOL_H
#define _PROTOCOL_H

enum Status {
    GIVE_MOVE=0,
    GAME_ENDED,
    ABORT,
};

enum player{
	NORMAL=0,
	PLACER,
};


/* Original Socket Interface */


typedef struct {
    Status          status;
    double          time_left; /* In seconds */
    int             row, col; /* Opponent's last move (if placer)*/
    bool            two; /* Opponent's last move (if placer)*/
    int 			dir; /* Opponent's last move (if normal player); 0 up 1 right 2 down 3 left */
    int				grid[BOARD_SIZE][BOARD_SIZE];
	player			pl;
} MsgFromServer;



typedef struct {
	player			pl;
    bool            two; /* Opponent's last move (if placer)*/
    int				row, col; /* Opponent's last move (if placer)*/
    int				dir; /* The player's move; 0 left 1 up 2 right 3 down */
} MsgToServer;

typedef char FirstMsgToServer;  /* 0-Normal, 1-Placer */
#endif //_PROTOCOL_H



