#ifndef _MOVEITERATOR_HPP
#define _MOVEITERATOR_HPP

#include "../Board/BitUtils.hpp"
#include "../Communication/Protocol.hpp"
#include "../Board/BitBoard.hpp"
#include "Search.hpp"

using namespace std;

struct search_result{
    int32_t score;
    int32_t move;
    bool threadSpawned;
};

template<typename T, player pl, bool mainThread>
class MoveIterator_t{
    uint64 moves;
public:
    //total squares * two possible moves for placer in the same square -2 for the always oqupied square
    struct search_result allResults[16*2-2];
    int32_t resIter;

public:
    MoveIterator_t(const T &board);

    void searchNextChild(T &board, int killer, int32_t depth, 
                                int32_t alpha, int32_t beta, bool firstChild){
        if (pl == player::PLACER){ //if resolved by compiler based on template
            uint64 p;
            if (killer >= 0){
                allResults[resIter].move = killer;

                p = uint64c(1) << killer;
                assert(moves & p);
                moves ^= p; //killer must be one in moves!
            } else {
                if (!moves) {
                    allResults[resIter].move = -1;
                    resIter++;
                    return;
                }

                p = pop_lsb(moves);
                //in case of placer move format is as follows:
                // - | - - - - 
                // 4 | square
                // square   : 0-15 board position tile was placed
                // 4        : 0-1  1 when a 4-tile was placed, 0 otherwise
                allResults[resIter].move = square(p);
            }
            assert((board.getEmptyTiles() | (board.getEmptyTiles() << SQR_POP)) & (uint64c(1) <<  allResults[resIter].move));
            assert(p == (uint64c(1) <<  allResults[resIter].move));

            //move is always valid! play it and return score
            board.makePlace(p);
            if(mainThread){
                if(true && !firstChild && depth > 5){
                    allResults[resIter].score = 0; //Thread spawned
                    allResults[resIter].threadSpawned = true;
                    spawnThread<player::NORMAL>(board, depth, alpha, beta, firstChild, &allResults[resIter].score);
                }else{
                    allResults[resIter].score = search_deeper<player::NORMAL, mainThread>(board, depth, alpha, beta, firstChild);
                    allResults[resIter].threadSpawned = false;
                }
                resIter++;
            }else{
                allResults[resIter].score = search_deeper<player::NORMAL, mainThread>(board, depth, alpha, beta, firstChild);
                resIter++;
            }
            board.undoPlace(p);
            assert(resIter != 16*2-2);
        } else {
            T bd2 = board;
            if (killer >= 0){
                allResults[resIter].move = killer;

                uint64 p = killer;
                assert(moves & p);
                moves ^= p; //killer must be one in moves!
                bd2.move(p);
            } else {
                do {
                    //in case of normal move format is as follows:
                    // 0 | - - - - 
                    //   | direction
                    // direction : 0001->left, 0010->up, 0100->right, 1000->down
                    allResults[resIter].move = pop_lsb(moves);
                    if (!allResults[resIter].move) {                //no move available, return
                        allResults[resIter].move = -1;
                        resIter++;
                        return;
                    }
                } while(!bd2.tryMove(allResults[resIter].move));
            }
            //move was valid! play it and return score
            if(mainThread){
                if(true && !firstChild && depth > 5){
                    allResults[resIter].score = 0; //Thread spawned
                    allResults[resIter].threadSpawned = true;
                    spawnThread<player::PLACER>(bd2, depth, alpha, beta, firstChild, &allResults[resIter].score);
                }else{
                    allResults[resIter].score = search_deeper<player::PLACER, mainThread>(bd2, depth, alpha, beta, firstChild);
                    allResults[resIter].threadSpawned = false;
                }
                resIter++;
            }else{
                allResults[resIter].score = search_deeper<player::PLACER, mainThread>(bd2, depth, alpha, beta, firstChild);
                resIter++;
            }
            assert(resIter != 16*2-2);
        }
    }

};


template<typename T, player pl, bool mainThread> 
MoveIterator_t<T, pl, mainThread>::MoveIterator_t(const T &board){
    if (pl == player::PLACER){ //if resolved by compiler based on template
        moves  = board.getEmptyTiles();
        moves |= moves << SQR_POP;
    } else {
        moves = 0xF;
    }
    resIter = 0;
}

#endif /* _MOVEITERATOR_HPP */