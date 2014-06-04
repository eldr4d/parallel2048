#ifndef _DEFINITIONS_HPP
#define _DEFINITIONS_HPP

#include <thread>

using namespace std;


#define NUM_OF_THREADS 4

typedef struct{
    int dir;
    int row, col, v;
}Move;

enum player{
    NORMAL=0,
    PLACER,
};

extern uint64_t horizonNodes;
extern uint64_t totalNodes;

constexpr player getOtherPlayer(player pl) {
    return (player) (pl ^ 1); //== PLACER ? NORMAL : PLACER;
}

#endif /* _DEFINITIONS_HPP */