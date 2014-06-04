#ifndef _SEARCH_HPP
#define _SEARCH_HPP

#include "../Communication/Client-comm.hpp"
#include "TranspositionTable.hpp"
#include "../Board/BitBoard.hpp"
using namespace std;

extern TranspositionTable tt;

int32_t veryVeryGreedyAndStupidEvaluationFunction(BitBoard_t boardForEv);

template<player pl>
int32_t negaScout(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta);

template<player other>
int32_t search_deeper(BitBoard_t &board, int32_t depth, int32_t alpha, int32_t beta, bool firstChild);

#endif /* _SEARCH_HPP */