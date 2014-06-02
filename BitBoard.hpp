/**
 * @file BitBoard.hpp
 * 
 * @brief Declaration of bitboards for game representation
 */
#ifndef _BITBOARD_HPP
#define _BITBOARD_HPP

#include <stdexcept>
#include <iomanip>
#include <iostream>
#include <cassert>
#include <cmath>
#include "BitUtils.hpp"

using namespace std;

/**
 * @brief Coordinate and value of tile
 *
 * Value is in logarithmic scale
 */
struct ptile{
    /**
     * @brief Row in which tile belongs
     */
    unsigned int row;
    /**
     * @brief Column in which tile belongs
     */
    unsigned int col;
    /**
     * @brief Value of tile, in logarithmic scale
     */
    unsigned int vlog;
};

/**
 * @brief Values for each of the 2 directions
 *
 * Horizontal or vertical direction
 */
enum dir{
    /**
     * @brief Horizontal
     */
    h = 1,
    /**
     * @brief Vertical
     */
    v = 4
};

/**
 * @brief Values for each of the 4 directions
 *
 * Left, right, down, up direction
 */
enum d4{
    /**
     * @brief Left
     */
    left  =  1,
    /**
     * @brief Up
     */
    up    =  4,
    /**
     * @brief Right
     */
    right = -1,
    /**
     * @brief Down
     */
    down  = -4
};

/**
 * @brief Constants and functions which depend on direction (2)
 *
 * Line masks
 * 
 * @tparam          d   direction (vertical, horizontal)
 */
template<dir d> class dconsts{
public:
    /**
     * @brief distance in bits of two adjacent positions in current direction
     */
    static const int step   = (d == h) ? 1 : 4;
    /**
     * @brief bits to shift to move 1 position in current direction
     */
    static const int shft1  =  step      ;
    /**
     * @brief bits to shift to move 2 positions in current direction
     */
    static const int shft2  = (step << 1);

    /**
     * @brief Masks masking out line 0 in current direction
     *
     * Masks out column 0 if direction is horizontal, 
     * row 0 if direction is vertical
     */
    static const uint64 l0  =(d==h)?
                    uint64c(0xEEEEEEEEEEEEEEEE) : uint64c(0xFFF0FFF0FFF0FFF0);
    /**
     * @brief Masks masking out lines 0, 1 in current direction
     *
     * Masks out columns 0 and 1 if direction is horizontal, 
     * row 0 and 1 if direction is vertical
     */
    static const uint64 l01 = (d == h) ?
                    uint64c(0xCCCCCCCCCCCCCCCC) : uint64c(0xFF00FF00FF00FF00);
    /**
     * @brief Masks masking out line 3 in current direction
     *
     * Masks out column 3 if direction is horizontal, 
     * row 3 if direction is vertical
     */
    static const uint64 l3  = (d == h) ?
                    uint64c(0x7777777777777777) : uint64c(0x0FFF0FFF0FFF0FFF);
    /**
     * @brief Masks masking out lines 2, 3 in current direction
     *
     * Masks out columns 2 and 3 if direction is horizontal, 
     * row 2 and 3 if direction is vertical
     */
    static const uint64 l23 = (d == h) ?
                    uint64c(0x3333333333333333) : uint64c(0x00FF00FF00FF00FF);
};

/**
 * @brief Constants and functions which depend on direction (4)
 *
 * Line masks and shift operations
 * 
 * @tparam          d   direction (left, right, up, down)
 */
template<d4 d> class d4c: public dconsts<(dir) abs((int) d)>{
public:
    /**
     * @brief Reverse direction
     */
    static const d4  rev    = (d4) -d;
    /**
     * @brief Vertical if current direction is up or down, horizontal otherwise
     */
    static const dir base   = (dir) abs((int) d);

    /**
     * @brief Mask without line 0 vertical to current direction
     * 
     * For example, if current direction is left, this will be the mask 
     * excluding column 3.
     * If direction is up, this will be set to mask out row 0.
     */
    static const uint64 r0  = (d > 0) ? dconsts<base>::l0  : dconsts<base>::l3 ;
    /**
     * @brief Mask without line 0 and 1 vertical to current direction
     * 
     * For example, if current direction is left, this will be the mask 
     * excluding column 3 and column 2.
     * If direction is up, this will be set to mask out row 0 and 1.
     */
    static const uint64 r01 = (d > 0) ? dconsts<base>::l01 : dconsts<base>::l23;
    /**
     * @brief Mask without line 3 vertical to current direction
     * 
     * For example, if current direction is left, this will be the mask 
     * excluding column 0.
     * If direction is up, this will be set to mask out row 3.
     */
    static const uint64 r3  = (d > 0) ? dconsts<base>::l3  : dconsts<base>::l0 ;
    /**
     * @brief Mask without line 2 and 3 vertical to current direction
     * 
     * For example, if current direction is left, this will be the mask 
     * excluding column 0 and column 1.
     * If direction is up, this will be set to mask out row 2 and 3.
     */
    static const uint64 r23 = (d > 0) ? dconsts<base>::l23 : dconsts<base>::l01;

    /**
     * @brief Simulate a left shift
     *
     * Performs a shift equivalent to left shift, if directions was 'left'.
     * Shift is by an amount of @p shftamnt positions.
     *
     * @param[in]   x           bitset to shift
     * @param[in]   shftamnt    number of positions to shift
     *
     * @return                  result of shifting
     */
    static uint64 left_shift(uint64 x, unsigned int shftamnt);
    /**
     * @brief Simulate a right shift
     *
     * Performs a shift equivalent to right shift, if directions was 'left'.
     * Shift is by an amount of @p shftamnt positions.
     *
     * @param[in]   x           bitset to shift
     * @param[in]   shftamnt    number of positions to shift
     *
     * @return                  result of shifting
     */
    static uint64 right_shift(uint64 x, unsigned int shftamnt);
    /**
     * @brief Simulate a left shift by 1 position
     *
     * Performs a shift equivalent to left shift, if directions was 'left'.
     * Shifts by one position.
     *
     * @param[in]   x           bitset to shift
     *
     * @return                  result of shifting
     */
    static uint64 left_shift1(uint64 x);
    /**
     * @brief Simulate a right shift by 1 position
     *
     * Performs a shift equivalent to right shift, if directions was 'left'.
     * Shifts by one position.
     *
     * @param[in]   x           bitset to shift
     *
     * @return                  result of shifting
     */
    static uint64 right_shift1(uint64 x);
    /**
     * @brief Simulate a left shift by 2 position
     *
     * Performs a shift equivalent to left shift, if directions was 'left'.
     * Shifts by two positions.
     *
     * @param[in]   x           bitset to shift
     *
     * @return                  result of shifting
     */
    static uint64 left_shift2(uint64 x);
    /**
     * @brief Simulate a right shift by 2 position
     *
     * Performs a shift equivalent to right shift, if directions was 'left'.
     * Shifts by two positions.
     *
     * @param[in]   x           bitset to shift
     *
     * @return                  result of shifting
     */
    static uint64 right_shift2(uint64 x);
};

/**
 * @brief tile's value type
 */
typedef unsigned int tile_value;

/**
 * @brief one full board
 */
#define FBOARD uint64c(0xFFFF)

/**
 * @brief Board's corners
 */
#define CORNERS uint64c(0x9009)

/**
 * @brief board's height (and width) size
 *
 * CHANGING IT WILL PROBABLY PRODUCE MANY BUGS!!! CHANGE AT YOUR OWN RISK
 */
#define BOARD_SIZE (4)

/**
 * @brief Count of board's squares
 */
#define SQR_POP (BOARD_SIZE*BOARD_SIZE)


/**
 * @brief Bitboard to represent game state
 *
 * board bits:
 *
 * 12 13 14 15 \n
 * 08 09 10 11 \n
 * 04 05 06 07 \n
 * 00 01 02 03 \n
 */
template<unsigned int state_size = 4>
class BitBoard{
private:
    /**
     * bits to represent board's state
     */
    uint64 state[state_size]; //(64*4)/16 = 16
    //state[0] will contain 'occupied' squares as the lower 2 bytes
    //state[0], bytes 3..2:     '2' squares
    //state[0], bytes 5..4:     '4' squares
    //state[0], bytes 7..6:     '8' squares
    //state[1], bytes 1..0:    '16' squares
    //state[1], bytes 3..2:    '32'
    //state[1], bytes 5..4:    '64'
    //state[1], bytes 7..6:   '128'
    //state[2], bytes 1..0:   '256' squares
    //state[2], bytes 3..2:   '512'
    //state[2], bytes 5..4:  '1024'
    //state[2], bytes 7..6:  '2048'
    //state[3], bytes 1..0:  '4096' squares
    //state[3], bytes 3..2:  '8192'
    //state[3], bytes 5..4: '16384'
    //state[3], bytes 7..6: '32768' squares

public:
    /**
     * Score of current board
     */
    int score;

    /**
     * @brief Creates a new empty board with two tiles
     *
     * New tiles will be of number '2' on coordinates (2, 2) and (2, 3)
     */
    BitBoard();

    /**
     * @brief Sets initial pieces.
     *
     * Creates a board such that it contains (at most) two tiles, on coordinates
     * (@p xa, @p ya) (tile is '2' if @p a_is_2 else '4')
     * (@p xb, @p yb) (tile is '2' if @p b_is_2 else '4')
     * 
     * @param[in]  ya       y coordinate of tile 'a' in {0, 1,..., BOARD_SIZE-1}
     * @param[in]  xa       x coordinate of tile 'a' in {0, 1,..., BOARD_SIZE-1}
     * @param[in]  a_is_2   true if tile 'a' should be 2, false will produce '4'
     * @param[in]  yb       y coordinate of tile 'b' in {0, 1,..., BOARD_SIZE-1}
     * @param[in]  xb       x coordinate of tile 'b' in {0, 1,..., BOARD_SIZE-1}
     * @param[in]  b_is_2   true if tile 'b' should be 2, false will produce '4'
     * 
     * @exception  std::invalid_argument        an argument is out of range
     */
    BitBoard(unsigned int ya, unsigned int xa, bool a_is_2,
                unsigned int yb, unsigned int xb, bool b_is_2);

    /**
     * @brief Asserts bitboard is in an expected state (only for debugging)
     *
     * If @p NDEBUG is undefined, asserts that bitboard is in a normal state,
     * otherwise, it does nothing.
     *
     * @post    If @p NDEBUG is undefined and assertion does not fail, the
     *          board will be on a valid state.
     */
    void assert_state() const;

    /**
     * @brief Sets initial pieces.
     *
     * Resets board such that it contains (at most) two tiles, on coordinates
     * (@p xa, @p ya) (tile is '2' if @p a_is_2 else '4')
     * (@p xb, @p yb) (tile is '2' if @p b_is_2 else '4')
     * 
     * @param[in]  ya       y coordinate of tile 'a' in {0, 1,..., BOARD_SIZE-1}
     * @param[in]  xa       x coordinate of tile 'a' in {0, 1,..., BOARD_SIZE-1}
     * @param[in]  a_is_2   true if tile 'a' should be 2, false will produce '4'
     * @param[in]  yb       y coordinate of tile 'b' in {0, 1,..., BOARD_SIZE-1}
     * @param[in]  xb       x coordinate of tile 'b' in {0, 1,..., BOARD_SIZE-1}
     * @param[in]  b_is_2   true if tile 'b' should be 2, false will produce '4'
     * 
     * @exception  std::invalid_argument        an argument is out of range
     */
    void initialize(unsigned int ya = 2, unsigned int xa = 2, bool a_is_2=true,
                    unsigned int yb = 3, unsigned int xb = 2, bool b_is_2=true);

    /**
     * @brief delete all the contents of the board
     **/
    void clear();
private:

    /**
     * @brief Checks coordinates
     *
     * Checks that coordinates are inside board. Board is defined by the square
     * having its corners at (@p 0, @p 0), (@p BOARD_SIZE-1, @p 0), 
     * (@p 0, @p BOARD_SIZE-1), (@p BOARD_SIZE-1, @p BOARD_SIZE-1)
     *
     * @param[in]  y        y coordinate of tile, in {0, 1,..., BOARD_SIZE-1}
     * @param[in]  x        x coordinate of tile, in {0, 1,..., BOARD_SIZE-1}
     *
     * @return              true if coordinates are inside board's borders, 
     *                      false otherwise.
     */
    static bool valid_xy(unsigned int y, unsigned int x);

    /**
     * @brief Computes mask for bit at (@p y, @p x) in bitboard specified by
     *          @p pw, inside a @p uint64 state.
     *
     * Computes a mask having only one active bit at coordinate (@p y, @p x)
     * of the bitboard specified using the @p pw argument in one @p uint64 
     * state variable. Bitboard will be selected by peeking the board
     * containing tiles being a @p pw power of 2.
     *
     * @pre                 valid coordinates
     *
     * @param[in]  y        y coordinate of tile, in {0, 1,..., BOARD_SIZE-1}
     * @param[in]  x        x coordinate of tile, in {0, 1,..., BOARD_SIZE-1}
     * @param[in]  pw       power of 2 of targeted bitboard's tiles
     *
     * @post                mask with exactly one active bit.
     * 
     * @return              Computed mask
     */
    static uint64 xy2mask(unsigned int y, unsigned int x, unsigned int pw=0);

    /**
     * @brief Transforms coordinates to board's index
     *
     * Computes a index of coordinate (@p y, @p x) in a board.
     *
     * @pre                 valid coordinates
     *
     * @param[in]  y        y coordinate of tile, in {0, 1,..., BOARD_SIZE-1}
     * @param[in]  x        x coordinate of tile, in {0, 1,..., BOARD_SIZE-1}
     *
     * @post                index in {0, 1, ..., @p SQR_POP-1}
     * 
     * @return              computed index
     */
    static unsigned int xy2ind(unsigned int y, unsigned int x);

    /**
     * @brief Fills all boards of a @p uint64 state variable with the board 
     * located in @p t 's lower @p SQR_POP bits.
     *
     * Copies lower board of @p t in every board of result.
     *
     * @pre                 only lower board of @p t contains active bits
     *
     * @param[in]  t        variable containing only one board in its lower
     *                      @p SQR_POP bits
     *
     * @post                all boards in result have the same state
     * @post                lower board in result is the same with @p t 's
     * 
     * @return              state variable containing multiple replicates of
     *                      a board
     */
    static uint64 fill(uint64 t);

    /**
     * @brief Fills all boards of a @p uint64 state variable with the board 
     * located in @p t 's lower @p SQR_POP bits.
     *
     * Copies lower board of @p t in every board of result.
     *
     * @param[in]  t        variable containing a board in its lower
     *                      @p SQR_POP bits
     *
     * @post                all boards in result have the same state
     * @post                lower board in result is the same with @p t 's
     * 
     * @return              state variable containing multiple replicates of
     *                      a board
     */
    static uint64 mfill(uint64 t);

    /**
     * @brief Computes the occupancy of boards in state @p t
     *
     * In every result's board the occupancy of corresponding 's board and its 
     * left ones (located in state @p t).
     *
     * Total of occupancy of @p t will be located in lower board (Board 0).
     * Board 1 will contain occupancy of boards 1, 2, 3.
     * Board 2 will contain occupancy of boards 2, 3.
     * Board 3 will contain higher board of @p t.
     *
     * @param[in]  t        state variable whose occupancy is to be found
     * 
     * @return              state variable containing multiple occupancy boards
     */
    static uint64 occu(uint64 t);

    /**
     * @brief Computes the occupancy of board in state @p t
     *
     * Computes occupancy of board @p t, having every result's bit deactivated 
     * if it does not exists to the lower board.
     *
     * @param[in]  t        state variable whose occupancy is to be found
     *
     * @post                only lower board of result contains active bits
     * 
     * @return              state variable containing active bits on its lower
     *                      board, representing occupancy of @p t
     */
    static uint64 moccu(uint64 t);

    /**
     * @brief Performs merging on a specific direction
     *
     * Performs vertical or horizontal merging, based on @p d.
     * Only adjacent tiles are merged, no sliding happens.
     *
     * New tile will replace the upper one (if horizontal merge) or the left one
     * (if vertical merge) of the pair of merging tiles.
     *
     * @tparam     d       merging direction
     */
    template<dir d> void combine();

    /**
     * @brief Performs slide on a specific direction
     *
     * Performs slide in direction specified by @p d.
     * No merging happens.
     *
     * @tparam     d       slide direction
     */
    template<d4  d> void compress();

public:

    uint64 getHash() const;
    
    static ptile mask2xy(uint64 x);

    void makePlace(unsigned int y, unsigned int x, bool is2);
    void makePlace(uint64 m, bool is2);
    void undoPlace(unsigned int y, unsigned int x, bool is2);
    void undoPlace(uint64 m, bool is2);

    /**
     * @brief Returns true if board contains the given tile
     *
     * @return              true if the given tile exist else false
     */
     bool tileExist(tile_value tile);

	/**
     * @brief Returns the Highest tile of the board
     *
     * @param[out] inCorner set to true, if there is a tile having the highest
     *                      value in corner
   	 *
     * @return              logarithm (base 2) of value of the largest tile
     */
    tile_value getHigherTile(bool &inCorner);
    
    /**
     * @brief Gets value (log) of tile on coordinates (@p y, @p x)
     *
     * Checks bitboard for the existence of a tile at (@p y, @p x) and returns 
     * its log_2(value).
     *
     * @param[in]  y        y coordinate of tile, in {0, 1,..., BOARD_SIZE-1}
     * @param[in]  x        x coordinate of tile, in {0, 1,..., BOARD_SIZE-1}
     *
     * @return              logarithm (base 2) of value of tile at (@p y, @p x),
     *                      0 if square is empty.
     */
    tile_value getTile(unsigned int y, unsigned int x) const;
    
    unsigned int countOccupiedTiles() const;
    unsigned int countFreeTiles() const;

    /**
     * @brief Checks if a tile can be placed
     *
     * Checks if there is any empty slot
     *
     * @return      true if a tile can be placed, false otherwise
     */
    bool existsPlacerMove() const;
    
    /**
     * @brief Tries to place a tile.
     *
     * Tries to place a tile on coordinates (@p x, @p y).
     * If position is occupied, board does not change and false is returned.
     * Otherwise a tile of value '2' if @p is2 else '4' is placed.
     * 
     * @param[in]  y        y coordinate of tile in {0, 1,..., BOARD_SIZE-1}
     * @param[in]  x        x coordinate of tile in {0, 1,..., BOARD_SIZE-1}
     * @param[in]  is2      true if tile to be placed should be 2, false if '4'
     * 
     * @return              true is tile successfully placed, false if 
     *                      position was occupied
     */
    bool tryPlace(unsigned int y, unsigned int x, bool is2);
    /**
     * @brief Checks is it is possible to place a tile at (@p x, @p y).
     *
     * Checks if position (@p x, @p y) is empty.
     * 
     * @param[in]  y        y coordinate of tile in {0, 1,..., BOARD_SIZE-1}
     * @param[in]  x        x coordinate of tile in {0, 1,..., BOARD_SIZE-1}
     * 
     * @return              true if position is empty, false otherwise
     */
    bool canPlace(unsigned int y, unsigned int x) const;
    /**
     * @brief Tries to place a tile.
     *
     * Tries to place a tile on position specified by @p m.
     * If position is occupied, board does not change and false is returned.
     * Otherwise a tile of value '2' if @p is2 else '4' is placed.
     *
     * @pre                 @p m contains exactly one active bit and it is 
     *                      located in its lower @p SQR_POP bits
     * 
     * @param[in]  m        board having an active bit on position of new tile
     * @param[in]  is2      true if tile to be placed should be 2, false if '4'
     * 
     * @return              true is tile successfully placed, false if 
     *                      position was occupied
     */
    bool tryPlace(uint64 m, bool is2);
    /**
     * @brief Checks is it is possible to place a tile at @p m.
     *
     * Checks if position specified by @p m is empty.
     * 
     * @pre                 @p contains exactly one active bit and it is located
     *                      in its lower @p SQR_POP bits
     * 
     * @param[in]  m         board having an active bit on position to check
     * 
     * @return              true if position is empty, false otherwise
     */
    bool canPlace(uint64 m) const;

    /**
     * @brief Tries to perform a sliding move on a specific direction
     *
     * Based on lower two bits of @p d, tries to make a sliding/merging move
     * on a specific direction. If move is not possible no changes will be made,
     * otherwise move is done.
     *
     * Direction based on lower bits of @p d, will be:
     * 0 -> left, 1 -> down, 2 -> right, 3 -> up
     *
     * @param[in]           bitset specifying direction
     *
     * @return              true if move is legal, false otherwise
     */
    bool tryMove(unsigned int d);
    /**
     * @brief Checks if a sliding move on a specific direction is legal
     *
     * Based on lower two bits of @p d, checks if a sliding/merging move
     * on specified direction is legal.
     *
     * Direction based on lower bits of @p d, will be:
     * 0 -> left, 1 -> down, 2 -> right, 3 -> up
     *
     * @param[in]           bitset specifying direction
     *
     * @return              true if move is legal, false otherwise
     */
    bool existsMove(unsigned int d) const;
    /**
     * @brief Checks if a sliding move is legal in any direction
     *
     * Checks if there exists a direction, that sliding based on it, is legal
     *
     * @return              true iff there exists a legal sliding/merging move
     */
    bool existsNormalMove() const;
    /**
     * @brief Performs a sliding move on a specific direction
     *
     * Based on lower two bits of @p d, performs a sliding/merging move
     * on specified direction.
     *
     * Direction based on lower bits of @p d, will be:
     * 0 -> left, 1 -> down, 2 -> right, 3 -> up
     * 
     * @pre                 sliding/merging move in direction @p d is legal
     * 
     * @param[in]           bitset specifying direction
     */
    void move(unsigned int d);

    /**
     * @brief Randomly places a tile
     *
     * Places a tile in a random empty position. Tile will be with 
     * 10% probability of value '4' and with 90% of value '2'.
     *
     * @pre                 there exists an empty position on board
     *
     * @return              coordinate of new tile and its value, in log
     */
    ptile placeRandom();

    /**
     * @brief Computes empty tiles
     *
     * Computes empty tiles on board
     * 
     * @return      a @p uint64 containing a board with every empty square 
     *              active, on its lower @p SQR_POP bits
     */
    uint64 getEmptyTiles() const;

    /**
     * @brief Tries to perform slide on a specific direction
     *
     * Tries to perform a move in the direction specified by @p d, doing 
     * tile merging and sliding as specified by rules.
     *
     * @tparam      d       slide direction
     *
     * @return      true if move was possible, false otherwise
     */
    template<d4 d> bool tryMove();

    /**
     * @brief Checks if a move in direction @p d can be made
     *
     * Checks if a move is possible in direction @p d.
     *
     * @tparam      d       direction to check for move
     *
     * @return      true if a move in direction @p d is possible, 
     *              false otherwise
     */
    template<d4 d> bool existsNMove() const;

    /**
     * @brief Performs slide on a specific direction
     *
     * Performs a move in the direction specified by @p d, doing 
     * tile merging and sliding as specified by rules.
     *
     * @tparam      d       slide direction
     */
    template<d4 d> void move();

    /**
     * @brief Prints board in @p out using a nice format
     *
     * Result printed contains multiple lines and has a human friendly format
     *
     * @param[out]  out     stream to print board
     */
    void prettyPrint(ostream& out=cout) const;

    bool operator==(const BitBoard<state_size> &other) const;
};

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
template<unsigned int state_size>
ostream& operator<<(ostream& os, const BitBoard<state_size>& obj);

#endif /* _BITBOARD_HPP */
