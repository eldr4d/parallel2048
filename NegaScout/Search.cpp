#include "Search.hpp"
#include "MoveIterator.hpp"
#include <limits>

using namespace std;

ThreadPool<maskedArguments> myThreadPool(NUM_OF_THREADS,&negaScoutWrapper);

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
    int kiMove = tt.retrieveTTEntry(hash, depth, alpha, beta, pl == PLACER);
    if (alpha >= beta) {
        // statistics(++hashHitCutOff);
        return alpha;
    }
    // kiMove = -1;
    int bmove = -1;

    int32_t alph = alpha, bet = beta;
    bool firstChild = true;

    MoveIterator_t<BitBoard_t, pl, mainThread> mIt(board);
    bool threadsSpawned=false;
    int iter = 0;
    while(true) {
        mIt.searchNextChild(board, kiMove, depth-1, alph, bet, firstChild);
        kiMove = -1;
        //mIt.allResults[mIt.resIter-1].move == -1 signals that no more moves exists, no move was played
        //if (mIt.allResults[mIt.resIter-1].move == -1) break;                 //equivalent
        int tmp = mIt.allResults[mIt.resIter-1].move;
        if (tmp < 0) break;                     //and rt.score is invalid
        firstChild = false; //set this here
        if(mainThread){//Template if
            if(mIt.allResults[mIt.resIter-1].threadSpawned) {
                threadsSpawned = true;
                continue;//7576 16780
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
        // if(amIfirst && depth == 9){
        //     cout << " ------ -- -- - -- - -- - - " << endl << board << endl;
        //     cout << "f " << iter << " " << alpha << " " << beta << " " << depth << " " << pl << " " <<  mIt.allResults[iter-1].score << " " << hex << mIt.allResults[iter-1].move << dec << endl;

        // }
        //mIt.allResults[mIt.resIter-1].move is always non-negative if a valid move was played

        //Setting firstChild here permits skipping the rest of current iteration
        //if score must be ignored for now

        //possible set a continue here such that if a new thread is used for
        //subtree search, to skip score checking ? 
        //if (mIt.allResults[mIt.resIter-1].move < -1) continue;

        /*if(amIfirst && depth == 9 && mIt.resIter >= 2){  
            cout << iter-1 << " " << alpha << " " << beta << " " << depth << " " << pl << " " <<  mIt.allResults[iter-1].score << " " << hex << mIt.allResults[iter-1].move << dec << endl;
        }*/
        //move played!
        if (mIt.allResults[mIt.resIter-1].score >= bet){
            tt.addTTEntry(hash, depth, mIt.allResults[mIt.resIter-1].move, mIt.allResults[mIt.resIter-1].score, pl==PLACER, Cut_Node);
            return bet;                         //fail-hard beta cut-off
        }//never with killer!

        if (mIt.allResults[mIt.resIter-1].score > alph){                   //better move found
            alph  = mIt.allResults[mIt.resIter-1].score;
            bmove = mIt.allResults[mIt.resIter-1].move;
        }
    }
    if(mainThread && threadsSpawned){
        int failHardCuttoff = 0;
        iter = 0;
        while(mIt.allResults[iter].move != -1){
            //if(pl==NORMAL)
            while(mIt.allResults[iter].score == 0){
                usleep(0.01);
            }

            if(!failHardCuttoff){
                // if(depth == 9)
                //     cout << iter << " " << alpha << " " << beta << " " << depth << " " << pl << " " <<  mIt.allResults[iter].score << " " << hex << mIt.allResults[iter].move << dec << endl;
                if (mIt.allResults[iter].score >= bet){
                    tt.addTTEntry(hash, depth, mIt.allResults[iter].move, mIt.allResults[iter].score, pl==PLACER, Cut_Node);
                    failHardCuttoff = iter+1;//fail-hard beta cut-off
                } else if (mIt.allResults[iter].score > alph){                   //better move found
                    alph  = mIt.allResults[iter].score;
                    bmove = mIt.allResults[iter].move;
                    assert(bmove >= 0);
                }
            }
            iter++;
        }
        if(failHardCuttoff){
            // iter = failHardCuttoff - 1;
            // tt.addTTEntry(hash, depth, mIt.allResults[iter].move, mIt.allResults[iter].score, pl==PLACER, Cut_Node);
            return bet;
        }
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
