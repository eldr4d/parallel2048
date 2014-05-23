/**
 * @file BitBoard.cpp
 * 
 * @brief Definition of bitboards for game representation
 */
#include "BitBoard.hpp"
#include <cstring>

template<d4 d>
uint64 d4c<d>::left_shift(uint64 x, unsigned int shftamnt){
    if (d > 0) return x << (shftamnt * d4c<d>::step);
    return x >> (shftamnt * d4c<d>::step);
}

template<d4 d>
uint64 d4c<d>::right_shift(uint64 x, unsigned int shftamnt){
    return d4c<d4c<d>::rev>::left_shift(x, d4c<d>::shftamnt);
}

template<d4 d>
uint64 d4c<d>::left_shift1(uint64 x){
    if (d > 0) return x << d4c<d>::shft1;
    return x >> d4c<d>::shft1;
}

template<d4 d>
uint64 d4c<d>::right_shift1(uint64 x){
    return d4c<d4c<d>::rev>::left_shift1(x);
}

template<d4 d>
uint64 d4c<d>::left_shift2(uint64 x){
    if (d > 0) return x << d4c<d>::shft2;
    return x >> d4c<d>::shft2;
}

template<d4 d>
uint64 d4c<d>::right_shift2(uint64 x){
    return d4c<d4c<d>::rev>::left_shift2(x);
}

template<unsigned int state_size>
BitBoard<state_size>::BitBoard(){
    initialize(2, 2, true, 3, 2, true);
}

template<unsigned int state_size>
BitBoard<state_size>::BitBoard(unsigned int ya, unsigned int xa, bool a_is_2,
                    unsigned int yb, unsigned int xb, bool b_is_2){
    initialize(ya, xa, a_is_2, yb, xb, b_is_2);
}

template<unsigned int state_size>
void BitBoard<state_size>::initialize(
                                unsigned int ya, unsigned int xa, bool a_is_2,
                                unsigned int yb, unsigned int xb, bool b_is_2){
    if (xa >= BOARD_SIZE) throw std::invalid_argument("invalid xa coord");
    if (ya >= BOARD_SIZE) throw std::invalid_argument("invalid ya coord");
    if (xb >= BOARD_SIZE) throw std::invalid_argument("invalid xb coord");
    if (yb >= BOARD_SIZE) throw std::invalid_argument("invalid yb coord");
    memset(state, 0, state_size*sizeof(uint64));
    state[0]  = xy2mask(ya, xa, (a_is_2 ? 1 : 2));
    state[0] |= xy2mask(yb, xb, (b_is_2 ? 1 : 2));
    state[0] |= ((state[0]>>SQR_POP) & FBOARD) | (state[0]>>(SQR_POP << 1));
    score = 0;
}

template<unsigned int state_size>
void BitBoard<state_size>::assert_state() const{
#ifndef NDEBUG
    uint64 all = state[0] & ~FBOARD;
    for (int i = 1 ; i < state_size ; ++i) all |= state[i];
    all = moccu(all);
    assert(all == (state[0] & FBOARD));
#endif
}

template<unsigned int state_size>
tile_value BitBoard<state_size>::getTile(unsigned int y, unsigned int x) const{
    //tile mask
    uint64 mask  = xy2mask(y, x);
    if (!(state[0] & mask)) return 0;
    uint64 fmask = fill(mask);
    //reverse search to not get fooled by occupancy board
    for (int i = state_size-1 ; i >= 0 ; --i){
        uint64 t = state[i] & fmask;
        if (t){
            tile_value v = 0;
            //we only need to check 3 of 4 boards, as t is true...
            //skip 0 to not get fooled by occupancy board in case of i == 0
            t >>= SQR_POP;
            for (int j = 1; j < 4 ; ++j, t >>= SQR_POP)
                if (t & FBOARD) v = j;
            return v + (i << 2);
        }
    }
    assert(false); //will not reach this point
    //if tile is empty, return should had happen at first return
    //else it should had been detected inside one of the loops and exited.
    return -1;
}

template<unsigned int state_size>
bool BitBoard<state_size>::valid_xy(unsigned int y, unsigned int x){
    return (y < BOARD_SIZE) && (x < BOARD_SIZE);
}

template<unsigned int state_size>
uint64 BitBoard<state_size>::xy2mask(unsigned int y, unsigned int x, 
                                        unsigned int pw){
    assert(valid_xy(y, x));
    return uint64(1) << (xy2ind(y, x) + SQR_POP * (pw & 3));
}

template<unsigned int state_size>
ptile BitBoard<state_size>::mask2xy(uint64 x){
    ptile t;
    unsigned long int sq = square(x);
    t.row = (sq >> 2) & 0x3;
    t.col = sq & 0x3;
    t.vlog = sq >> 4;
    return t;
}

template<unsigned int state_size>
unsigned int BitBoard<state_size>::xy2ind(unsigned int y, unsigned int x){
    assert(valid_xy(y, x));
    return (x + (y*BOARD_SIZE));
}

template<unsigned int state_size>
uint64 BitBoard<state_size>::mfill(uint64 t){
    return fill(t & FBOARD);
}

template<unsigned int state_size>
uint64 BitBoard<state_size>::fill(uint64 t){
    t |= t << SQR_POP;
    return t | (t << (SQR_POP << 1));
}

template<unsigned int state_size>
uint64 BitBoard<state_size>::occu(uint64 t){
    t |= (t >> (SQR_POP << 1));
    return t | (t >> SQR_POP);
}

template<unsigned int state_size>
uint64 BitBoard<state_size>::moccu(uint64 t){
    return occu(t) & FBOARD;
}

template<unsigned int state_size>
void BitBoard<state_size>::prettyPrint(ostream& out) const{
    //upper border
    out << " ";
    for (int i = 0 ; i < BOARD_SIZE ; ++i) out << "   " << i << "  ";
    out << "   \n +";
    for (int i = 0 ; i < BOARD_SIZE ; ++i) out << "------";
    out << "+\n";

    //tiles
    for (int i = 0 ; i < BOARD_SIZE ; ++i){
        out << i << "|";
        for (int j = 0 ; j < BOARD_SIZE ; ++j){
            tile_value v = getTile(i, j);
            if (getTile(i, j) == 0){
                out << "_____ ";
            } else {
                out << setw(5) << (1 << v) << ' ';
            }
        }
        out << "|" << i;
        if (i == (BOARD_SIZE-1)/2  ) out << "            Score";
        if (i == (BOARD_SIZE-1)/2+1) out << "            " << setw(5) << score;
        out << '\n';
    }
    out << setw(0);

    // lower border
    out << " +";
    for (int i = 0 ; i < BOARD_SIZE ; ++i) out << "------";
    out << "+\n ";
    for (int i = 0 ; i < BOARD_SIZE ; ++i) out << "   " << i << "  ";
    out << "   \n";
}

template<unsigned int state_size> template<dir d>
void BitBoard<state_size>::combine(){
    uint64 pre = 0;
    uint64 all = 0;
    for (int j = 0 ; j < state_size ; ++j){
        uint64 x, m, c, o;

        //compute position of new tiles (left one or upper one)
        x           = state[j];
        c           = m = x &  (x >> dconsts<d>::shft1) & dconsts<d>::l3  ;
        c           =     c ^ ((c << dconsts<d>::shft1) & dconsts<d>::l0 );
        c           =     c ^ ((c << dconsts<d>::shft2) & dconsts<d>::l01);
        c           =     c & m;

        //clear old tiles
        o           =     c | ((c << dconsts<d>::shft1) & dconsts<d>::l0 );

        if (j){
            all |= state[j] = (x ^ o) | (c << SQR_POP) | (pre >> (3*SQR_POP));
        } else {
            //protect 2's and occupancy
            all |= state[j] = ((x ^ o) | ((c & ~FBOARD) << SQR_POP)) & ~FBOARD;
        }
        pre         = c;
    }
    state[0] |= moccu(all);
}

template<unsigned int state_size> template<d4 d>
void BitBoard<state_size>::compress(){
    uint64 m = mfill(state[0]);
    //based on compress function (p. 153) of Hacker's Delight (second edition)

    //compute shifts

    //occupancy is the mask, compress active bits of mask to the right
    //all bits are relevant, as occupancy is is superset of any board
    uint64 mk = d4c<d>::left_shift1(~m) & d4c<d>::r0;
    uint64 mp, mv0, mv1;
    //for i = 0, 1

    //i = 0
    mp = mk ^ (d4c<d>::left_shift1(mk) & d4c<d>::r0);
    mp = mp ^ (d4c<d>::left_shift2(mp) & d4c<d>::r01);

    mv0 = mp & m;                   //bits to move 1 step

    //no need to mask after shift, two XORs will have eliminate first column
    m  = (m ^ mv0) | d4c<d>::right_shift1(mv0); //so... no overflow

    mk  = mk & ~mp;
    //end loop

    //i = 1
    mp = mk ^ (d4c<d>::left_shift1(mk) & d4c<d>::r0);
    mp = mp ^ (d4c<d>::left_shift2(mp) & d4c<d>::r01);

    mv1 = mp & m;                   //bits to move 2 steps (after moving by 1)

    // m  = (m ^ mv1) | (mv1 >> 2); //no need, m will not be used again...

    // mk  = mk & ~mp; //no need, mk will not be used again...
    //end loop

    //end of compute shifts

    //shift actual boards
    for (int j = 0; j < state_size ; ++j){
        uint64 x = state[j];
        uint64 t;

        t =  x & mv0;
        x = (x ^ t) | d4c<d>::right_shift1(t);

        t =  x & mv1;
        x = (x ^ t) | d4c<d>::right_shift2(t);

        state[j] = x;
    }
}

// template<unsigned int state_size>
// void BitBoard<state_size>::compressLeft(){
//     compress<d4::left>();
// }

// template<unsigned int state_size>
// void BitBoard<state_size>::compressUp(){
//     compress<d4::up>();
// }

// template<unsigned int state_size>
// void BitBoard<state_size>::compressDown(){
//     compress<d4::down>();
// }

// template<unsigned int state_size>
// void BitBoard<state_size>::compressRight(){
//     compress<d4::right>();
// }

template<unsigned int state_size> template<d4 d>
void BitBoard<state_size>::move(){
    compress<d>();
    combine<d4c<d>::base>();
    compress<d>();
}

template<unsigned int state_size> template<d4 d>
bool BitBoard<state_size>::existsNMove() const{
    if (d4c<d>::r0 & state[0]&d4c<d>::left_shift1(FBOARD^state[0])) return true;
    for (int i = 1 ; i < state_size ; ++i)
        if (d4c<d>::r0 & state[i] & d4c<d>::left_shift1(state[i])) return true;
    return false;
}

template<unsigned int state_size>
bool BitBoard<state_size>::existsNormalMove() const{
    if (existsNMove<d4::left>() ) return true;
    if (existsNMove<d4::right>()) return true;
    if (existsNMove<d4::down>() ) return true;
    if (existsNMove<d4::up>()   ) return true;
    return false;
}

template<unsigned int state_size>
bool BitBoard<state_size>::existsPlacerMove() const{
    return getEmptyTiles();
}

template<unsigned int state_size>
uint64 BitBoard<state_size>::getEmptyTiles() const{
    return (FBOARD & ~state[0]);
}

template<unsigned int state_size> template<d4 d>
bool BitBoard<state_size>::tryMove(){
    uint64 old = state[0];
    move<d>();
    return (FBOARD & (old ^ state[0]));
}

template<unsigned int state_size>
bool BitBoard<state_size>::tryMove(unsigned int d){
    switch(d & 3){
        case  0: return tryMove<d4::left> ();
        case  1: return tryMove<d4::down> ();
        case  2: return tryMove<d4::right>();
        default: return tryMove<d4::up>   ();
    }
}

template<unsigned int state_size>
void BitBoard<state_size>::move(unsigned int d){
    switch(d & 3){
        case  0: move<d4::left> (); break;
        case  1: move<d4::down> (); break;
        case  2: move<d4::right>(); break;
        default: move<d4::up>   (); break;
    }
}

template<unsigned int state_size>
bool BitBoard<state_size>::existsMove(unsigned int d) const{
    switch(d & 3){
        case  0: return existsNMove<d4::left> ();
        case  1: return existsNMove<d4::down> ();
        case  2: return existsNMove<d4::right>();
        default: return existsNMove<d4::up>   ();
    }
}

template<unsigned int state_size>
bool BitBoard<state_size>::canPlace(unsigned int y, unsigned int x) const{
    return canPlace(xy2mask(y, x));
}

template<unsigned int state_size>
bool BitBoard<state_size>::canPlace(uint64 m) const{
    assert(!(m & ~FBOARD));
    return !(state[0] & m);
}

template<unsigned int state_size>
bool BitBoard<state_size>::tryPlace(unsigned int y, unsigned int x, bool is2){
    uint64 m = xy2mask(y, x);
    return tryPlace(m, is2);
}

template<unsigned int state_size>
bool BitBoard<state_size>::tryPlace(uint64 m, bool is2){
    assert(!(m & ~FBOARD));
    assert(m);
    assert(!(m & (m - 1)));
    uint64 t = state[0];
    if (t & m) return false;
    state[0] = t | m | (m << ( SQR_POP << (is2 ? 0 : 1)));
    return true;
}

template<unsigned int state_size>
void BitBoard<state_size>::makePlace(unsigned int y, unsigned int x, bool is2){
    makePlace(xy2mask(y, x), is2);
}

template<unsigned int state_size>
void BitBoard<state_size>::makePlace(uint64 m, bool is2){
    assert(!(m & ~FBOARD));
    assert(m);
    assert(!(m & (m - 1)));
    assert(!(state[0] & m));
    state[0] |= m | (m << ( SQR_POP << (is2 ? 0 : 1)));
}

template<unsigned int state_size>
void BitBoard<state_size>::undoPlace(unsigned int y, unsigned int x, bool is2){
    undoPlace(xy2mask(y, x), is2);
}

template<unsigned int state_size>
void BitBoard<state_size>::undoPlace(uint64 m, bool is2){
    assert(!(m & ~FBOARD));
    assert(m);
    assert(!(m & (m - 1)));
    uint64 t = state[0];
    assert(t & m);
    assert(t & (m << ( SQR_POP << (is2 ? 0 : 1))));
    state[0] = t & ~(m | (m << ( SQR_POP << (is2 ? 0 : 1))));
}

template<unsigned int state_size>
ptile BitBoard<state_size>::placeRandom(){
    uint64 s = FBOARD & ~state[0];
    assert(s);
    uint64 t = s;
    for (int i = rand()&0xF ; i > 0 ; --i){
        s = s & (s - 1);
        if (!s) s = t;
    }
    t = s & -s;
    unsigned int r = rand()%10;
    tryPlace(t, r);
    ptile ret = mask2xy(t);
    ret.vlog = (r) ? 1 : 2;
    return ret;
}

template<unsigned int state_size>
bool BitBoard<state_size>::operator==(const BitBoard<state_size> &other) const {
    for (int i=0;i < state_size;++i) if (state[i]^other.state[i]) return false;
    return true;
}

template<unsigned int state_size>
ostream& operator<<(ostream& os, const BitBoard<state_size>& obj)
{
    obj.prettyPrint(os);
    return os;
}

template class BitBoard<4u>;
template void BitBoard<4u>::move<d4::left>();
template void BitBoard<4u>::move<d4::right>();
template void BitBoard<4u>::move<d4::down>();
template void BitBoard<4u>::move<d4::up>();
template bool BitBoard<4u>::existsNMove<d4::left> () const;
template bool BitBoard<4u>::existsNMove<d4::right>() const;
template bool BitBoard<4u>::existsNMove<d4::down> () const;
template bool BitBoard<4u>::existsNMove<d4::up>   () const;
/**
 * @brief Appends to @p os stream board @p obj in a nice format
 *
 * Output will be in a human friendly multi-line format
 *
 * @param[out]      os      stream to append board to
 * @param[in]       obj     board to append
 *
 * @return                  stream used for output
 */
template ostream& operator<<(ostream& os, const BitBoard<4u>& obj);