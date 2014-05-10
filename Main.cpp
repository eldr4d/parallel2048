#include "Board.hpp"
#include <iostream>
#include <time.h>
#include <cstdlib>
using namespace std;
int main(int argc, char **argv){
	Board board;
	//cout << board.IsLegalNormalMove(2) << endl;
	//board.PrettyPrint();
	//cout << board.DoPlacerMove(1,2,2) << endl;
	//board.PrettyPrint();
	srand (time(NULL));
	
	while(board.CanNormalMove()){
		board.PrettyPrint();
		int r;
		do{
			r = rand()%4;
		}while(board.IsLegalNormalMove(r) == 0);
		board.DoNormalMove(r);
		cout << "Normal Move: " << r << endl;
		board.AddRandom();
		board.PrettyPrint();
		char ch;
		cin >> ch;
	}

}
