#include "Search.hpp"
#include "MoveIterator.hpp"
#include <limits>

using namespace std;

template<player pl>
int32_t negaScout(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta){
    ++totalNodes;
    player other = getOtherPlayer(pl);

    //An ftasame sta katw katw fyla tote gyrname thn katastash
    if(depth == 0){
        ++horizonNodes;
        int32_t temp = veryVeryGreedyAndStupidEvaluationFunction(board);
        temp = (pl == NORMAL) ? temp : -temp;
        return temp;
    }

    uint64 hash = board.getHash();
    //alpha beta are passed by reference!!!
    int kiMove = tt.retrieveTTEntry(hash, depth, alpha, beta, pl == PLACER);
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
        if (kiMove >= 0){
            sr = mIt.searchKiller(board, kiMove, depth-1, alph, bet,firstChild);
            kiMove = -1;
        } else {
            sr = mIt.searchNextChild(board, depth - 1, alph, bet, firstChild);
        }
        if (sr.move < 0) break;
        //move played!
        if (sr.score >= bet){
            tt.addTTEntry(hash, depth, sr.move, sr.score, pl==PLACER, Cut_Node);
            return bet;                         //fail-hard beta cut-off
        }

        firstChild = false;
        if (sr.score > alph){                   //better move found
            alph  = sr.score;
            bmove = sr.move;
        }
    }

    if (firstChild){ //no move was available, this is a leaf node, return score
        assert(pl == NORMAL);
        ++horizonNodes;
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
}

template<player other>
int32_t search_deeper(BitBoard_t &board, int32_t depth, int32_t alpha, 
                                                int32_t beta, bool firstChild){
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
