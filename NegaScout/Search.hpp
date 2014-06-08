#ifndef _SEARCH_HPP
#define _SEARCH_HPP

#include "../Communication/Client-comm.hpp"
#include "TranspositionTable.hpp"
#include "../Board/BitBoard.hpp"
#include "../ThreadPool/ThreadPool.hpp"

using namespace std;

extern TranspositionTable tt;

extern std::atomic<int32_t> alpha,beta;
typedef struct{
    player pl;
    int32_t depth;
    BitBoard_t board;
    bool firstChild;
    std::atomic<int32_t> *writeResult;
}maskedArguments;




template<player other>
void spawnThread(BitBoard_t &board, int32_t depth, bool firstChild, std::atomic<int32_t> *score);


int32_t veryVeryGreedyAndStupidEvaluationFunction(BitBoard_t boardForEv);


template<player pl, bool mainThread>

int32_t negaScout(BitBoard_t &board, int32_t depth);


template<player other, bool mainThread>
int32_t search_deeper(BitBoard_t &board, int32_t depth, bool firstChild);

void negaScoutWrapper(maskedArguments args);


#endif /* _SEARCH_HPP */