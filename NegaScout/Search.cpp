#include "Search.hpp"
#include "MoveIterator.hpp"
#include <limits>

using namespace std;

ThreadPool<maskedArguments> myThreadPool(NUM_OF_THREADS,&negaScoutWrapper);

search_result allResults[16*2-2];

template<player pl, bool mainThread>
int32_t negaScout(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool amIfirst){
    ++totalNodes;
    player other = getOtherPlayer(pl);
    uint64 hash = board.getHash();

    //An ftasame sta katw katw fyla tote gyrname thn katastash
    if(depth == 0){
        ++horizonNodes;
        int32_t temp = veryVeryGreedyAndStupidEvaluationFunction(board);
        temp = (pl == NORMAL) ? temp : -temp;
        // cout << hex << hash << dec << temp << endl;
        return max(min(temp, beta), alpha);
    }

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
    bool threadsSpawned=false;
    int iter = 0;
    while(true) {
        mIt.searchNextChild(board, kiMove, depth-1, alph, bet, firstChild);
        kiMove = -1;
        //allResults[mIt.resIter-1].move == -1 signals that no more moves exists, no move was played
        //if (allResults[mIt.resIter-1].move == -1) break;                 //equivalent
        int tmp = allResults[mIt.resIter-1].move;
        if (tmp < 0) break;                     //and rt.score is invalid
        firstChild = false; //set this here
        if(mainThread){//Template if
            if(allResults[mIt.resIter-1].threadSpawned) {
                threadsSpawned = true;
                //continue;//7576 16780
            }
        //     // }else{
        //     //    iter++;
        //     // }
        // }else{
            // if (pl == PLACER && mIt.resIter >= 2) continue;
        //     // if(mIt.resIter >= 2)
        //         continue;
        //     // iter++;
        }


        while(allResults[mIt.resIter-1].score == 0){
            usleep(0.01);
        }
        // if(amIfirst && depth == 9){
        //     cout << " ------ -- -- - -- - -- - - " << endl << board << endl;
        //     cout << "f " << iter << " " << alpha << " " << beta << " " << depth << " " << pl << " " <<  allResults[iter-1].score << " " << hex << allResults[iter-1].move << dec << endl;

        // }
        //allResults[mIt.resIter-1].move is always non-negative if a valid move was played

        //Setting firstChild here permits skipping the rest of current iteration
        //if score must be ignored for now

        //possible set a continue here such that if a new thread is used for
        //subtree search, to skip score checking ? 
        //if (allResults[mIt.resIter-1].move < -1) continue;

        /*if(amIfirst && depth == 9 && mIt.resIter >= 2){  
            cout << iter-1 << " " << alpha << " " << beta << " " << depth << " " << pl << " " <<  allResults[iter-1].score << " " << hex << allResults[iter-1].move << dec << endl;
        }*/
        //move played!
        assert(allResults[mIt.resIter-1].score <= beta);
        assert(allResults[mIt.resIter-1].score >= alpha);
        assert(allResults[mIt.resIter-1].score <= bet);
        assert(allResults[mIt.resIter-1].score >= alph);
        // assert(allResults[mIt.resIter-1].move == m);
        if (allResults[mIt.resIter-1].score >= bet){
            tt.addTTEntry(hash, depth, allResults[mIt.resIter-1].move, allResults[mIt.resIter-1].score, pl==PLACER, Cut_Node);
            return bet;                         //fail-hard beta cut-off
        }

        if (allResults[mIt.resIter-1].score > alph){                   //better move found
            alph  = allResults[mIt.resIter-1].score;
            bmove = allResults[mIt.resIter-1].move;
            assert(bmove >= 0);
        }
    }
    if(mainThread && threadsSpawned){
        // int failHardCuttoff = 0;
        // iter = 0;
        // while(allResults[iter].move != -1){
        //     //if(pl==NORMAL)
        //     while(allResults[iter].score == 0){
        //         usleep(0.01);
        //     }

        //     assert(allResults[iter].score <= beta);
        //     assert(allResults[iter].score >= alpha);
        //     assert(allResults[iter].score <= bet);
        //     // assert(allResults[iter].score >= alph);
        //     if(!failHardCuttoff){
        //         // if(depth == 9)
        //         //     cout << iter << " " << alpha << " " << beta << " " << depth << " " << pl << " " <<  allResults[iter].score << " " << hex << allResults[iter].move << dec << endl;
        //         if (allResults[iter].score >= bet){
        //             // tt.addTTEntry(hash, depth, allResults[iter].move, allResults[iter].score, pl==PLACER, Cut_Node);
        //             failHardCuttoff = iter+1;//fail-hard beta cut-off
        //         } else if (allResults[iter].score > alph){                   //better move found
        //             alph  = allResults[iter].score;
        //             bmove = allResults[iter].move;
        //             assert(bmove >= 0);
        //         }
        //     }
        //     iter++;
        // }
        // if(failHardCuttoff){
        //     iter = failHardCuttoff - 1;
        //     tt.addTTEntry(hash, depth, allResults[iter].move, allResults[iter].score, pl==PLACER, Cut_Node);
        //     return bet;
        // }
    }
    //Wait for threads to finish and merge scores here (?)

    if (firstChild){ //no move was available, this is a leaf node, return score
        board.assert_state();
        assert(pl == NORMAL);
        ++horizonNodes;
        bool a;
        return max(min((int32_t) (board.getHigherTile(&a) << 2), beta), alpha);
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
        //score = -negaScout<other, mainThread>(board, depth, -alpha-1, -alpha);
        //if(alpha < score){
        score = -negaScout<other, mainThread>(board, depth, -beta, -alpha,firstChild);
            //score = -negaScout<other, mainThread>(board, depth, -beta, -score);
        //}
    }
    return score;
}


template<player other>
void spawnThread(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild, int32_t *score){
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
    myThreadPool.useNewThread(args);
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

template int32_t negaScout<player::PLACER, true>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool amIfirst);
template int32_t negaScout<player::NORMAL, true>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool amIfirst);
template int32_t negaScout<player::PLACER, false>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool amIfirst);
template int32_t negaScout<player::NORMAL, false>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool amIfirst);
template int32_t search_deeper<player::PLACER, true>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild);
template int32_t search_deeper<player::NORMAL, true>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild);
template int32_t search_deeper<player::PLACER, false>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild);
template int32_t search_deeper<player::NORMAL, false>(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild);
