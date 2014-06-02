#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include "../BitBoard.hpp"

/* Messages flowing between the server and the clients */
enum Status {
    GIVE_MOVE=0,
    GAME_ENDED,
    ABORT,
};

enum Directions { //DO NOT CHANGE VALUES!!!
    LEFT = 0,
    DOWN = 1,
    RIGHT = 2,
    UP = 3,
    DIR_SIZE,
};

enum player{
	NORMAL=0,
	PLACER,
};

typedef struct {
    Status          status;
    double          time_left; /* In seconds */
    int             row, col; /* Opponent's last move (if placer)*/
    bool            two; /* Opponent's last move (if placer)*/
    int 			dir; /* Opponent's last move (if normal player); 0 up 1 right 2 down 3 left */
	player			pl;
    BitBoard<4u>    board;
} MsgFromServer;



typedef struct {
	player			pl;
    bool            two; /* Opponent's last move (if placer)*/
    int				row, col; /* Opponent's last move (if placer)*/
    int				dir; /* The player's move; 0 left 1 up 2 right 3 down */
} MsgToServer;

typedef char FirstMsgToServer;  /* 0-Normal, 1-Placer */
#endif //_PROTOCOL_H



