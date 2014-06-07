#include "Search.hpp"
#include "MoveIterator.hpp"
#include <limits>

using namespace std;

ThreadPool<maskedArguments> myThreadPool(NUM_OF_THREADS,&negaScoutWrapper);

search_result allResults[64][16*2-2];

template<player pl, bool mainThread>
int32_t negaScout(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool amIfirst){
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
    int32_t alph = alpha, bet = beta;
    int kiMove = tt.retrieveTTEntry(hash, depth, alph, bet, pl == PLACER);
    if (alph >= bet) {
        // statistics(++hashHitCutOff);
        if (bet  >= beta ) return beta;
        if (alph <= alpha) return alpha;
        return alph;
    }
    // kiMove = -1;
    int bmove = -1;

    bool firstChild = true;

    MoveIterator_t<BitBoard_t, pl, mainThread> mIt(board);
    // bool threadsSpawned=false;
    int iter = 0;
    while(true) {
        tlocal_search_result tmp_r = mIt.searchNextChild(board, kiMove, 
            depth-1, alph, bet, firstChild, &(allResults[depth][iter].score));
        kiMove = -1;
        //allResults[mIt.resIter-1].move == -1 signals that no more moves exists, no move was played
        //if (allResults[mIt.resIter-1].move == -1) break;                 //equivalent
        //int tmp = allResults[depth-1][mIt.resIter-1].move;
        if (tmp_r.move == -1) break;                     //and rt.score is invalid
        firstChild = false; //set this here
        if(mainThread){//Template if
            if(tmp_r.move < 0) {
                // threadsSpawned = true;
                // while(allResults[depth][iter].score == 0) usleep(0.01);
                // tmp_r.score = allResults[depth][iter].score;
                allResults[depth][iter].move  = -(2+tmp_r.move);
                // allResults[depth][iter].move  = -(2+tmp_r.move); //-2-move
                ++iter;
                continue;
            }
        }
        // assert(!threadsSpawned);
        //allResults[depth-1][mIt.resIter-1].move is always non-negative if a valid move was played

        //Setting firstChild here permits skipping the rest of current iteration
        //if score must be ignored for now

        //possible set a continue here such that if a new thread is used for
        //subtree search, to skip score checking ? 
        //if (allResults[depth-1][mIt.resIter-1].move < -1) continue;

        //move played!
        // assert(tmp_r.score <= beta);
        // assert(tmp_r.score >= alpha);
        // assert(tmp_r.score <= bet);
        // assert(tmp_r.score >= alph);

        if (tmp_r.score >= bet){
            tt.addTTEntry(hash, depth, tmp_r.move, tmp_r.score, pl==PLACER, Cut_Node);
            return bet;                                 //fail-hard beta cut-off
        }

        if (tmp_r.score > alph){                        //better move found
            alph  = tmp_r.score;
            bmove = tmp_r.move;
            assert(bmove >= 0);
        }
    }
    if(mainThread){
        // int failHardCuttoff = 0;
        for (int i = 0; i < iter; ++i){
        // while(allResults[depth][iter].move != -1){
            //if(pl==NORMAL)
            while(allResults[depth][i].score == 0){
                usleep(0.01);
            }
            tlocal_search_result tmp_r;

            tmp_r.score = allResults[depth][i].score;
            tmp_r.move  = allResults[depth][i].move;

            // if(!failHardCuttoff){
                // if(depth == 9)
                //     cout << iter << " " << alpha << " " << beta << " " << depth << " " << pl << " " <<  allResults[depth-1][iter].score << " " << hex << allResults[depth-1][iter].move << dec << endl;
                // if (allResults[depth][i].score >= bet){
                //     for (int j = i+1 ; j < iter ; ++j){
                //         while(allResults[depth][j].score == 0){
                //             usleep(0.01);
                //         }
                //     }
                //     tt.addTTEntry(hash, depth, tmp_r.move, tmp_r.score, pl==PLACER, Cut_Node);
                //     return bet;
                //     // tt.addTTEntry(hash, depth, tmp_r.move, tmp_r.score, pl==PLACER, Cut_Node);
                //     // failHardCuttoff = i+1;//fail-hard beta cut-off
                // } else if (tmp_r.score > alph){                   //better move found
                //     alph  = tmp_r.score;
                //     bmove = tmp_r.move;
                //     assert(bmove >= 0);
                // }
            if (tmp_r.score >= bet){
                for (int j = i+1 ; j < iter ; ++j){
                    while(allResults[depth][j].score == 0){
                        usleep(0.01);
                    }
                }
                tt.addTTEntry(hash, depth, tmp_r.move, tmp_r.score, pl==PLACER, Cut_Node);
                return bet;                                 //fail-hard beta cut-off
            }

            if (tmp_r.score > alph){                        //better move found
                alph  = tmp_r.score;
                bmove = tmp_r.move;
                assert(bmove >= 0);
            }
            // }
        }
        // if(failHardCuttoff){
        //     iter = failHardCuttoff - 1;
        //     tt.addTTEntry(hash, depth, allResults[depth][iter].move, allResults[depth][iter].score, pl==PLACER, Cut_Node);
        //     return bet;
        // }
    }

    //Wait for threads to finish and merge scores here (?)

    if (firstChild){ //no move was available, this is a leaf node, return score
        board.assert_state();
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

template<player other, bool mainThread>
int32_t search_deeper(BitBoard_t &board, int32_t depth, int32_t alpha, 
                                                int32_t beta, bool firstChild){
    int32_t score;
    if(firstChild == true){
        score = -negaScout<other, mainThread>(board, depth, -beta, -alpha, firstChild);
    } else {
        score = -negaScout<other, mainThread>(board, depth, -alpha-1, -alpha, firstChild);
        if(alpha < score){
            score = -negaScout<other, mainThread>(board, depth, -beta, -alpha,firstChild);
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
    //cout << "Data = " << data << endl;
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
    //if(other == player::PLACER){
    //    cout << args.board << endl;
    //}
    // negaScoutWrapper(args);
    myThreadPool.useNewThread(args);
}

template int32_t negaScout<player::PLACER, true>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool amIfirst);
template int32_t negaScout<player::NORMAL, true>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool amIfirst);
template int32_t negaScout<player::PLACER, false>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool amIfirst);
template int32_t negaScout<player::NORMAL, false>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool amIfirst);
template int32_t search_deeper<player::PLACER, true>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild);
template int32_t search_deeper<player::NORMAL, true>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild);
template int32_t search_deeper<player::PLACER, false>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild);
template int32_t search_deeper<player::NORMAL, false>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild);
