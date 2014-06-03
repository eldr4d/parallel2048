#ifndef _DEFINITIONS_HPP
#define _DEFINITIONS_HPP

typedef struct{
    int dir;
    int row, col, v;
}Move;

enum player{
    NORMAL=0,
    PLACER,
};

constexpr player getOtherPlayer(player pl) {
    return pl == PLACER ? NORMAL : PLACER;
}

#endif /* _DEFINITIONS_HPP */