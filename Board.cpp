#include "Board.hpp"
#include <limits.h>
#include <assert.h>
#include "Communication/Protocol.hpp"

using namespace std;

void Board::InitializeBoard ()
{
	for(int i=0; i< BOARD_SIZE; i++){
		for(int j=0; j <BOARD_SIZE; j++){
			grid[i][j] = 0;
		}
	}
    //AddRandom();
    //AddRandom();
    grid[2][2] = 1;
    grid[2][3] = 1;
    higherTile = 1;
    score = 0;
}

void Board::PrettyPrint ()
{
    int     i, j;

    /* Print the upper boarder */
    cout << "   ";
    for (i=0 ; i < BOARD_SIZE ; i++){
		cout << i << " ";
    }

    cout << endl << " +";
    for (i=0 ; i < 2*BOARD_SIZE+1 ; i++){
		cout << "-";
	} 
    cout << "+" << endl;

    for (i=0 ; i < BOARD_SIZE ; i++) {
    	cout << i << "| ";
        for (j=0 ; j < BOARD_SIZE ; j++){
            if(grid[i][j] == 0) {
                cout << 0 << " ";
            }else{
            	cout << myPow(grid[i][j]) << " ";
            }
        }
        cout << "|" << i << endl;
    }



    /* Print the lower boarder */
	cout << " +";
    for (i=0 ; i < 2*BOARD_SIZE+1 ; i++){
    	cout << "-";
    }
    cout << "+" << endl;
    cout << "   ";
    
    for (i=0 ; i < BOARD_SIZE ; i++){
		cout << i << " ";
	}
	cout << endl;
	cout << "Da score is: " << score << endl << endl;
}

/* --------------------------------   Moves   ------------------------------- */

/* Probably not the most efficient implementation ... */
bool Board::CanNormalMove ()
{
	for(int direction = 0; direction < DIR_SIZE; direction++){
		if(IsLegalNormalMove(direction)){
			return true;
		}
	}
    return false;
}

//This must be always true :P
bool Board::CanPlacerMove()
{
	for(int x=0; x<BOARD_SIZE; x++){
		for(int y=0; y<BOARD_SIZE; y++){
			if(grid[x][y] == 0){
				return true;
			}
		}
	}	
    return false;
}



bool Board::IsLegalNormalMove(int direction)
{
	if(direction == UP){
		return MoveUp(true);
	}else if(direction == RIGHT){
		return MoveRight(true);
	}else if(direction == DOWN){
		return MoveDown(true);
	}else if(direction == LEFT){
		return MoveLeft(true);
	}
	return false;
}


bool Board::IsLegalPlacerMove(int row, int col, int value)
{
	if(grid[row][col] == 0 && (value == 1 || value == 2)){
		return true;
	}
	return false;
}

bool Board::DoNormalMove(int direction)
{
	if(direction == UP){
		return MoveUp(false);
	}else if(direction == RIGHT){
		return MoveRight(false);
	}else if(direction == DOWN){
		return MoveDown(false);
	}else if(direction == LEFT){
		return MoveLeft(false);
	}
	return false;
}

bool Board::DoPlacerMove(int row, int col, int value)
{
	if(grid[row][col] == 0 && (value == 1 || value == 2)){
		grid[row][col] = value;
		return true;
	}
	return false;
}

bool Board::MoveUp(bool check)
{
	bool success = false;
	int tempGrid[BOARD_SIZE][BOARD_SIZE];
	if(check){
		for(int x = 0; x<BOARD_SIZE; x++){
			for(int y = 0; y<BOARD_SIZE; y++){
				tempGrid[x][y] = grid[x][y];
			}
		}
	}

	success |= SlideArray(check);
	if(check){
		for(int x = 0; x<BOARD_SIZE; x++){
			for(int y = 0; y<BOARD_SIZE; y++){
				grid[x][y] = tempGrid[x][y];
			}
		}
	}
	return success;
}

bool Board::MoveRight(bool check)
{
	bool success;
	RotateBoard();
	success = MoveUp(check);
	RotateBoard();
	RotateBoard();
	RotateBoard();
	return success;
}

bool Board::MoveDown(bool check)
{
	bool success;
	RotateBoard();
	RotateBoard();
	success = MoveUp(check);
	RotateBoard();
	RotateBoard();
	return success;
}

bool Board::MoveLeft(bool check){
	bool success;
	RotateBoard();
	RotateBoard();
	RotateBoard();
	success = MoveUp(check);
	RotateBoard();
	return success;
}


bool Board::SlideArray(bool check)
{
	bool success = false;
	int x,t,stop=0;
	for(int y=0; y<BOARD_SIZE; y++){
		stop = 0;
		for(int x=0; x<BOARD_SIZE; x++) {
			if (grid[x][y]!=0) {
				t = FindTarget(x,y,stop);
				// if target is not original position, then move or merge
				if (t!=x) {
					// if target is not zero, set stop to avoid double merge
					if (grid[t][y]!=0) {
						if(!check){
							score+=2*myPow(grid[t][y]);
						}
						stop = t+1;
					}
					if(grid[t][y] == 0){
						grid[t][y] = grid[x][y];
					}else{
						grid[t][y]++;
						if(grid[t][y] > higherTile){
							higherTile = grid[t][y];
						}
					}
					grid[x][y]=0;
					success = true;
				}
			}
		}
	}
	return success;
}

int Board::FindTarget(int x, int y, int stop)
{
	int t;
	// if the position is already on the first, don't evaluate
	if (x==0) {
		return x;
	}
	for(t=x-1;t>=0;t--) {
		if (grid[t][y]!=0) {
			if (grid[t][y]!=grid[x][y]) {
				// merge is not possible, take next position
				return t+1;
			}
			return t;
		} else {
			// we should not slide further, return this one
			if (t==stop) {
				return t;
			}
		}
	}
	//we cannot move
	return x;
}

void Board::RotateBoard()
{
	int n = BOARD_SIZE;
	int tmp;
	for(int i=0; i<n/2; i++){
		for(int j=i; j<n-i-1; j++){
			tmp = grid[i][j];
			grid[i][j] = grid[j][n-i-1];
			grid[j][n-i-1] = grid[n-i-1][n-j-1];
			grid[n-i-1][n-j-1] = grid[n-j-1][i];
			grid[n-j-1][i] = tmp;
		}
	}
}
