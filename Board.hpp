#ifndef _BOARD_HPP
#define _BOARD_HPP

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>

/* ---------------------------------   SAFETY   ------------------------------*/

/* If the SAFETY flag is on the code performs more sanity checks. This way it */
/* easier to debug, but the code is less efficient. The flag should be turned */
/* on during development, and turned off in the final version.                */
#define SAFETY
#ifdef SAFETY
    #include <assert.h>
#else
    #define assert(p) {}
#endif

#define BOARD_SIZE 4

class Board{
public:
	int higherTile;
	int grid[BOARD_SIZE][BOARD_SIZE];
	int score;


	/* Allocate a new Board, and initialize it to the standard opening position   */
	Board(){
		higherTile = 0;
		srand(time(NULL));
	    InitializeBoard ();
	}

	/* Calculate 2^n															  */
	static int myPow(int n){
		int two = 2;
		return 2<<(n-1);
	}

	/* Re-initialize an existing board to the opening position.                   */
	void InitializeBoard ();

	/* A simple way to print to board in ASCII (mainly for debug).                */
	void PrettyPrint ();
	

	/* Does the side have any legal moves? */
	bool CanNormalMove();
	//This must always be true :P
	bool CanPlacerMove();

	/* Can the side put a piece on a given square */
	bool IsLegalNormalMove(int direction);
	bool IsLegalPlacerMove(int row, int col, int value);


	/* Perform the move. If allocate_new_board is TRUE a new board is allocated,  */
	/* and the move is performed there. Otherwise, the procedure will return the  */
	/* pointer it received to the board argument (after performing the move).     */
	/* In case of an illegal move, NULL is returned.                              */
	bool DoNormalMove(int direction);
	bool DoPlacerMove(int row, int col, int value);
	
	
	//TODO rewrite these functions when bitboard is ready
	bool biggestTileIsInCorner(){
		if(grid[0][0] == higherTile || grid[0][BOARD_SIZE-1] == higherTile || grid[BOARD_SIZE-1][0] == higherTile || grid[BOARD_SIZE-1][BOARD_SIZE-1] == higherTile){
			return true;
		}else{
			return false;
		}
	}
	
	//Add random number to the board
	void AddRandom() {
		int x, y;
		int r,len=0;
		int n,list[BOARD_SIZE*BOARD_SIZE][2];

		for(int x=0; x<BOARD_SIZE; x++) {
			for(int y=0; y<BOARD_SIZE; y++) {
				if(grid[x][y]==0) {
					list[len][0]=x;
					list[len][1]=y;
					len++;
				}
			}
		}
		if (len>0) {
			r = rand()%len;
			x = list[r][0];
			y = list[r][1];
			n = ((rand()%10)/9+1);
			grid[x][y]=n;
		}
	}
	
	bool MoveUp(bool check);
	bool MoveLeft(bool check);	
	bool MoveDown(bool check);
	bool MoveRight(bool check);
	bool SlideArray(bool check);
	void RotateBoard();
	int FindTarget(int x, int y, int stop);
	
};

#endif /*_BOARD_HPP*/

