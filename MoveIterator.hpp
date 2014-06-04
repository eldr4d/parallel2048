#ifndef _MOVEITERATOR_HPP
#define _MOVEITERATOR_HPP

#include "BitUtils.hpp"
#include "Communication/Protocol.hpp"
#include "BitBoard.hpp"
#include "Search.hpp"

using namespace std;

struct search_result{
    int32_t score;
    int32_t move;
};

template<typename T, player pl>
class MoveIterator_t{
    uint64 moves;

public:
    MoveIterator_t(const T &board);

    search_result searchKiller(T &board, int killer, int32_t depth, 
                                int32_t alpha, int32_t beta, bool firstChild){
        search_result ret;
        ret.move = killer;
        if (killer < 0) return ret;

        uint64 p = killer;
        if (pl == player::PLACER) p = uint64c(1) << p;

        assert(moves & p);
        moves ^= p; //killer must be one in moves!

        if (pl == player::PLACER){ //if resolved by compiler based on template
            board.makePlace(p);
            ret.score = search_deeper<player::NORMAL>(board, depth, alpha, beta, firstChild);
            board.undoPlace(p);
        } else {
            T bd2 = board;
            bd2.move(p);
            ret.score = search_deeper<player::PLACER>(bd2, depth, alpha, beta, firstChild);
        }
        return ret;
    }

    search_result searchNextChild(T &board, int32_t depth, 
                                int32_t alpha, int32_t beta, bool firstChild){

        search_result ret;

        if (pl == player::PLACER){ //if resolved by compiler based on template
            if (!moves) {
                ret.move = -1;
                return ret;
            }

            uint64 p = pop_lsb(moves);
            //in case of placer move format is as follows:
            // - | - - - - 
            // 4 | square
            // square   : 0-15 board position tile was placed
            // 4        : 0-1  1 when a 4-tile was placed, 0 otherwise
            ret.move  = square(p);
            assert((board.getEmptyTiles() | (board.getEmptyTiles() << SQR_POP)) & (uint64c(1) << ret.move));
            assert(p == (uint64c(1) << ret.move));

            //move is always valid! play it and return score
            board.makePlace(p);
            ret.score = search_deeper<player::NORMAL>(board, depth, alpha, beta, firstChild);
            board.undoPlace(p);
        } else {
            T bd2 = board;
            do {
                //in case of normal move format is as follows:
                // 0 | - - - - 
                //   | direction
                // direction : 0001->left, 0010->up, 0100->right, 1000->down
                ret.move = pop_lsb(moves);
                if (!ret.move) {                //no move available, return
                    ret.move = -1;
                    return ret;
                }
            } while(!bd2.tryMove(ret.move));

            //move was valid! play it and return score
            ret.score = search_deeper<player::PLACER>(bd2, depth, alpha, beta, firstChild);
        }
        return ret;
    }

};


template<typename T, player pl> 
MoveIterator_t<T, pl>::MoveIterator_t(const T &board){
    if (pl == player::PLACER){ //if resolved by compiler based on template
        moves  = board.getEmptyTiles();
        moves |= moves << SQR_POP;
    } else {
        moves = 0xF;
    }
}

#endif /* _MOVEITERATOR_HPP */