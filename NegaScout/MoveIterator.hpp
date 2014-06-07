#ifndef _MOVEITERATOR_HPP
#define _MOVEITERATOR_HPP

#include "../Board/BitUtils.hpp"
#include "../Communication/Protocol.hpp"
#include "../Board/BitBoard.hpp"
#include "Search.hpp"
#include <atomic>

using namespace std;

struct search_result{
    std::atomic<int32_t> score;
    std::atomic<int> move;
    // std::atomic<bool> threadSpawned;
};

struct tlocal_search_result{
    int32_t score;
    int     move;
};

extern search_result allResults[64][16*2-2];


template<typename T, player pl, bool mainThread>
class MoveIterator_t{
    uint64 moves;
public:
    //total squares * two possible moves for placer in the same square -2 for the always oqupied square
    // int32_t resIter;

public:
    MoveIterator_t(const T &board);

    tlocal_search_result searchNextChild(T &board, int killer, int32_t depth, 
            int32_t alpha, int32_t beta, bool fChild, atomic<int32_t> *thr_score){
        tlocal_search_result ret;
        if (pl == player::PLACER){ //if resolved by compiler based on template
            uint64 p;
            if (killer >= 0){
                ret.move = killer;

                p = uint64c(1) << killer;
                assert(moves & p);
                moves ^= p; //killer must be one in moves!
            } else {
                if (!moves) {
                    ret.move = -1;
                    return ret;
                }

                p = pop_lsb(moves);
                //in case of placer move format is as follows:
                // - | - - - - 
                // 4 | square
                // square   : 0-15 board position tile was placed
                // 4        : 0-1  1 when a 4-tile was placed, 0 otherwise
                ret.move = square(p);
            }
            assert((board.getEmptyTiles() | (board.getEmptyTiles() << SQR_POP)) & (uint64c(1) <<  ret.move));
            assert(p == (uint64c(1) <<  ret.move));

            //move is always valid! play it and return score
            board.makePlace(p);
            if(mainThread){
                if(true && !fChild && depth > 5){ //!firstChild
                    *thr_score = 0;
                    ret.move = -2 - ret.move;
                    spawnThread<player::NORMAL>(board, depth, alpha, beta, fChild, thr_score);
                }else{
                    ret.score = search_deeper<player::NORMAL, mainThread>(board, depth, alpha, beta, fChild);
                }
            }else{
                ret.score = search_deeper<player::NORMAL, mainThread>(board, depth, alpha, beta, fChild);
            }
            board.undoPlace(p);
        } else {
            T bd2 = board;
            if (killer >  0){
                ret.move = killer;

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
                    ret.move = pop_lsb(moves);
                    if (!ret.move){                 //no move available, return
                        ret.move = -1;
                        return ret;
                    }
                } while(!bd2.tryMove(ret.move));
            }
            //move was valid! play it and return score
            // if(mainThread){
            //     if(true && !firstChild && depth > 7){
            //         allResults[depth][resIter].score = 0; //Thread spawned
            //         allResults[depth][resIter].threadSpawned = true;
            //         spawnThread<player::PLACER>(bd2, depth, alpha, beta, firstChild, &allResults[depth][resIter].score);
            //     }else{
            //         allResults[depth][resIter].score = search_deeper<player::PLACER, mainThread>(bd2, depth, alpha, beta, firstChild);
            //         allResults[depth][resIter].threadSpawned = false;
            //     }
            //     resIter++;
            // }else{
                ret.score = search_deeper<player::PLACER, mainThread>(bd2, depth, alpha, beta, fChild);
                // resIter++;
            // }
        }
        return ret;
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
    // resIter = 0;
}

#endif /* _MOVEITERATOR_HPP */