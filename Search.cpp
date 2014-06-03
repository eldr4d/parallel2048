#include "Search.hpp"
#include "MoveIterator.hpp"
#include <limits>

using namespace std;

template<player pl>
int32_t negaScout(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta){
    
    player other = getOtherPlayer(pl);

    //An ftasame sta katw katw fyla tote gyrname thn katastash
    if(depth == 0){
        int32_t temp = veryVeryGreedyAndStupidEvaluationFunction(board);
        temp = (pl == NORMAL) ? temp : -temp;
        return temp;
    }

    uint64 hash = board.getHash();
    int killerMove = -1;
    //alpha beta are passed by reference!!!
    killerMove = tt.retrieveTTEntry(hash, depth, alpha, beta, pl == PLACER);
    //if (ttResult == ttExactScoreReturned) return alpha;
    if (alpha >= beta) {
        // statistics(++hashHitCutOff);
        return alpha;
    }
    int bmove = -1;

    int32_t alph = alpha, bet = beta;
    bool firstChild = true;

    MoveIterator_t<BitBoard_t, pl> mIt(board);

    search_result sr;
    while(true) {
        sr = mIt.searchNextChild(board, depth - 1, alph, bet, firstChild);
        if (sr.move < 0) break;

        //move played!
        if (sr.score >= bet){
            tt.addTTEntry(hash, depth, sr.move, sr.score, 
                            pl == PLACER, Cut_Node);
            return bet;                         //fail-hard beta cut-off
        }

        firstChild = false;
        if (sr.score > alph){                   //better move found
            alph  = sr.score;
            bmove = sr.move;
        }
    }

    if (firstChild){
        assert(pl == NORMAL);
        bool a;
        return board.getHigherTile(&a) << 2;
    }

    NodeType nt = PV__Node;
    if (bmove < 0){ //All-Node
        nt    = All_Node;
        bmove = 0;
    }
    tt.addTTEntry(hash, depth, bmove, alph, pl == PLACER, nt);
    return alph;

    // if(pl == PLACER){
    //     uint64 empty = board.getEmptyTiles();
    //     while (empty){
    //         uint64 p = pop_lsb(empty);
    //         for (unsigned int is2 = 0; is2 < 2 ; ++is2){ // 2 or 4
    //             board.makePlace(p, is2);
    //             int32_t score = search_deeper(board, other, depth - 1, alph, bet, firstChild);
    //             board.undoPlace(p, is2);

    //             if (score >= bet){
    //                 assert(move == NULL);
    //                 tt.addTTEntry(hash, depth, square(p), score, pl == PLACER, Cut_Node);
    //                 return bet;                         //fail-hard beta cut-off
    //             }
    //             firstChild = false;
    //             if (score > alph){
    //                 alph = score;
    //                 bmove = square(p);
    //                 bis2 = is2;
    //             }
    //         }
    //     }
    // }else{
    //     bool noMove = true;
    //  for(unsigned int d=LEFT; d<DIR_SIZE; d++){
    //         BitBoard_t board2 = board;
       //   if(!board2.tryMove(d)){
       //       continue;
       //   }
    //         noMove = false;
    //         int32_t score = search_deeper(board2, other, depth - 1, alph, bet, firstChild);

    //         if (score >= bet){
    //             assert(move == NULL);
    //             tt.addTTEntry(hash, depth, 1 << d, score, pl == PLACER, Cut_Node);
    //             return bet;                             //fail-hard beta cut-off
    //         }
    //         firstChild = false;
    //         if (score > alph){
    //             alph = score;
    //             bmove = 1 << d;
    //         }
    //  }
    // }
}

template<player other>
int32_t search_deeper(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild){
    int32_t score;
    if(firstChild == true){
        score = -negaScout<other>(board, depth, -beta, -alpha);
    } else {
        score = -negaScout<other>(board, depth, -alpha-1, -alpha);
        if(alpha < score){
            score = -negaScout<other>(board, depth, -beta, -score);
        }
    }
    return score;
}

template int32_t negaScout<player::PLACER>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta);
template int32_t negaScout<player::NORMAL>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta);
template int32_t search_deeper<player::PLACER>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild);
template int32_t search_deeper<player::NORMAL>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild);
