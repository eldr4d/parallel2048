#include "Search.hpp"
#include "MoveIterator.hpp"
#include <limits>

using namespace std;

ThreadPool<maskedArguments> myThreadPool(NUM_OF_THREADS,&negaScoutWrapper);

search_result allResults[64][16*2-2];

atomic<int32_t> galpha_pl;
atomic<player>  gpl;
atomic<bool>    ch_search;

template<player pl, bool mainThread>
int32_t negaScout(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta){
#ifndef NDEBUG
    ++totalNodes;
#endif

    //An ftasame sta katw katw fyla tote gyrname thn katastash
    if(depth == 0){
#ifndef NDEBUG
        ++horizonNodes;
#endif
        int32_t temp = veryVeryGreedyAndStupidEvaluationFunction(board);
        temp = (pl == NORMAL) ? temp : -temp;
        return temp;
    }

    uint64 hash = board.getHash();

    int32_t alph = alpha, bet = beta;
    //alpha beta are passed by reference!!!
    int kiMove = tt.retrieveTTEntry(hash, depth, alph, bet, pl == PLACER);
    if (alph >= bet) {
        return alph;
    }

    int bmove = -1;

    bool firstChild = true;

    MoveIterator_t<BitBoard_t, pl, mainThread> mIt(board);

    if (mainThread){
        galpha_pl = alph;
        gpl       = pl;
        ch_search = true;
    }

    int iter = 0;
    bool cont_cond;
    while( (cont_cond = (firstChild || ch_search)) ) { //assignment!!!
        tlocal_search_result tmp_r = mIt.searchNextChild(board, kiMove, 
            depth-1, alph, bet, firstChild, &(allResults[depth][iter].score));
        kiMove = -1;
        if (tmp_r.move == -1) break;                //and rt.score is invalid

        firstChild = false; //set this here
        if(mainThread){//Template if
            if(tmp_r.move < 0) {
                allResults[depth][iter].move  = -(2+tmp_r.move);
                ++iter;
                continue;
            }
        }
        //tmp_r.move is always non-negative if a valid move was played

        //Setting firstChild here permits skipping the rest of current iteration
        //if score must be ignored for now

        //possible set a continue here such that if a new thread is used for
        //subtree search, to skip score checking ? 
        //if (tmp_r.move < -1) continue;

        //move played!

        if (tmp_r.score >= bet){
            tt.addTTEntry(hash, depth, tmp_r.move, tmp_r.score, pl==PLACER, Cut_Node);
            return bet;                                 //fail-hard beta cut-off
        }
        if ((!mainThread) && pl != gpl && tmp_r.score >= -galpha_pl){
            if (bmove >= 0){
                tt.addTTEntry(hash, depth, alph, bmove, pl==PLACER, Cut_Node);
            }
            return -galpha_pl;//((pl == NORMAL) ? (int32_t) gbeta__pl : -galpha_pl);//MAX_TT_SCORE;
        }

        if (tmp_r.score > alph){                        //better move found
            if (mainThread){
                galpha_pl = +tmp_r.score;
            }
            alph  = tmp_r.score;
            bmove = tmp_r.move;
            assert(bmove >= 0);
        }
    }

    //Wait for threads to finish and merge scores here
    if(mainThread){
        bool unfinished;
        do {
            unfinished = false;
            for (int i = 0 ; i < iter ; ++i){
                if (allResults[depth][i].score == 0) {
                    unfinished = true;
                } else if (allResults[depth][i].move >= 0){
                    tlocal_search_result tmp_r;

                    tmp_r.score = allResults[depth][i].score;
                    tmp_r.move  = allResults[depth][i].move;
                    allResults[depth][i].move = -5;

                    if (tmp_r.score >= bet){
                        ch_search = false;
                        for (int j = 0 ; j < iter ; ++j){
                            while(allResults[depth][j].score == 0 && allResults[depth][j].move >= 0){
                                this_thread::yield();
                            }
                            if (allResults[depth][j].score > tmp_r.score){
                                tmp_r.score = allResults[depth][j].score;
                                tmp_r.move  = allResults[depth][j].move;
                            }
                        }
                        tt.addTTEntry(hash, depth, tmp_r.move, tmp_r.score, pl==PLACER, Cut_Node);
                        return bet;                             //fail-hard beta cut-off
                    }

                    if (tmp_r.score > alph){                    //better move found
                        galpha_pl = tmp_r.score;
                        alph  = tmp_r.score;
                        bmove = tmp_r.move;
                        assert(bmove >= 0);
                    }
                }
            }
            if (unfinished){
                this_thread::yield();
            }
        } while (unfinished);
    }

    if (firstChild){ //no move was available, this is a leaf node, return score
        board.assert_state();
        assert(pl == NORMAL);
#ifndef NDEBUG
        ++horizonNodes;
#endif
        bool a;
        return board.getHigherTile(&a) << 2;
    }

    NodeType nt = PV__Node;
    if (bmove < 0){                                     //All-Node
        nt    = All_Node;
        bmove = 0;
    } else if (!cont_cond){
        nt = Cut_Node;
    }
    tt.addTTEntry(hash, depth, bmove, alph, pl == PLACER, nt);
    return alph;
}

template<player other, bool mainThread>
int32_t search_deeper(BitBoard_t &board, int32_t depth, int32_t alpha, 
                                                int32_t beta, bool firstChild){
    int32_t score;
    if(firstChild == true){
        score = -negaScout<other, mainThread>(board, depth, -beta, -alpha);
    } else {
        score = -negaScout<other, mainThread>(board, depth, -alpha-1, -alpha);
        if(alpha < score){
            score = -negaScout<other, mainThread>(board, depth, -beta, -alpha);
        }
    }
    return score;
}

void negaScoutWrapper(maskedArguments args){
    int32_t data;
    if (args.pl == player::PLACER){
        data = search_deeper<player::PLACER,false>(args.board, args.depth, args.alpha, args.beta, args.firstChild);
    } else {
        data = search_deeper<player::NORMAL,false>(args.board, args.depth, args.alpha, args.beta, args.firstChild);
    }
    assert(data!=0);
    *(args.writeResult) = data;
}


template<player other>
void spawnThread(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild, atomic<int32_t> *score){
    maskedArguments args;
    args.pl = other;
    args.alpha = alpha;
    args.beta = beta;
    args.depth = depth;
    args.board = board;
    args.firstChild = firstChild;
    args.writeResult = score;

    myThreadPool.useNewThread(args);
}

int32_t veryVeryGreedyAndStupidEvaluationFunction(BitBoard_t boardForEv){
    bool inCorner = false;
    int32_t v2 = boardForEv.getHigherTile(&inCorner);
    int32_t score = (v2 << 7) + 1;
    if(inCorner){
        score += boardForEv.getMaxCornerChain() << 7; // +2 = 9!!!
        score <<= 2;
    }
    score += boardForEv.getMaxChain() << 4;
    int tmp = boardForEv.countFreeTiles() - 7;
    tmp = (tmp < 0) ? -tmp : tmp;
    score += (7-tmp) << 2;
    score += boardForEv.countTileTypes() << 2;
    assert(score >= 0);
    return score;
}

template int32_t negaScout<player::PLACER, true>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta);
template int32_t negaScout<player::NORMAL, true>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta);
template int32_t negaScout<player::PLACER, false>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta);
template int32_t negaScout<player::NORMAL, false>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta);
template int32_t search_deeper<player::PLACER, true>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild);
template int32_t search_deeper<player::NORMAL, true>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild);
template int32_t search_deeper<player::PLACER, false>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild);
template int32_t search_deeper<player::NORMAL, false>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild);
