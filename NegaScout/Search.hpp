#ifndef _SEARCH_HPP
#define _SEARCH_HPP

#include "../Communication/Client-comm.hpp"
#include "TranspositionTable.hpp"
#include "../Board/BitBoard.hpp"
#include "../ThreadPool/ThreadPool.hpp"
#include <atomic>

using namespace std;

extern TranspositionTable tt;

extern atomic<int32_t> galpha_pl;
extern atomic<int32_t> gbeta__pl;


typedef struct{
    player pl;
    int32_t alpha, beta, depth;
    BitBoard_t board;
    bool firstChild;
    atomic<int32_t> *writeResult;
}maskedArguments;

template<player other>
void spawnThread(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild, int32_t *score);


int32_t veryVeryGreedyAndStupidEvaluationFunction(BitBoard_t boardForEv);


template<player pl, bool mainThread>
int32_t negaScout(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool amIfirst);


template<player other, bool mainThread>
int32_t search_deeper(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild);

void negaScoutWrapper(maskedArguments args);


#endif /* _SEARCH_HPP */