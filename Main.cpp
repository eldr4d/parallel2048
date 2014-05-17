// #include "Board.hpp"
#include "BitBoard.hpp"
#include <iostream>
#include <time.h>
#include <cstdlib>
using namespace std;
int main(int argc, char **argv){
	// Board board;
	// //cout << board.IsLegalNormalMove(2) << endl;
	// //board.PrettyPrint();
	// //cout << board.DoPlacerMove(1,2,2) << endl;
	// //board.PrettyPrint();
	// srand (time(NULL));
	
	// while(board.CanNormalMove()){
	// 	board.PrettyPrint();
	// 	int r;
	// 	do{
	// 		r = rand()%4;
	// 	}while(board.IsLegalNormalMove(r) == 0);
	// 	board.DoNormalMove(r);
	// 	cout << "Normal Move: " << r << endl;
	// 	board.AddRandom();
	// 	board.PrettyPrint();
	// 	char ch;
	// 	cin >> ch;
	// }

	BitBoard<> board1;
	BitBoard<> board2 = board1;
	BitBoard<> board3 = board1;
	BitBoard<> board4 = board1;
	
	cout << board1;

	board1.move<d4::left>();
	cout << board1;

	board2.move<d4::right>();
	cout << board2;

	board3.move<d4::up>();
	cout << board3;

	board4.move<d4::down>();
	cout << board4;
	return 0;
}
