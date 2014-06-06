#ifndef _DEFINITIONS_HPP
#define _DEFINITIONS_HPP

#include <thread>
#include <atomic>

using namespace std;


#define NUM_OF_THREADS 1

typedef struct{
    int dir;
    int row, col, v;
}Move;

enum player{
    NORMAL=0,
    PLACER,
};

extern std::atomic<uint64_t> horizonNodes;
extern std::atomic<uint64_t> totalNodes;

constexpr player getOtherPlayer(player pl) {
    return (player) (pl ^ 1); //== PLACER ? NORMAL : PLACER;
}

#endif /* _DEFINITIONS_HPP */