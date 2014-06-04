/**
 * @file BitUtils.hpp
 * 
 * @brief Useful utilities to be used with bitsets
 */
#ifndef _BITUTILS_HPP
#define _BITUTILS_HPP

#include <iomanip>
#include <iostream>
#include <cassert>
#include <cstdint>

#ifdef _MSC_VER
#include <intrin.h>
#endif

using namespace std;

/**
 * @brief 64bit unsigned int
 */
typedef uint64_t uint64;
#define uint64c(x) UINT64_C(x)

/**
 * @brief Prints lower board of @p x, mirrored 
 * (left is right and right is left, up is down, down is up!)
 *
 * Prints in a simple format the lower board of @p x, in stream @p out.
 * Row change is signal by string @p del. Before appending board,
 * string @p pre is appended.
 *
 * @param[in]       x       bitset containing the board
 * @param[out]      out     output stream to print the board
 * @param[in]       del     string to print after each row change
 * @param[in]       pre     string to print before board
 */
void printLBoard(uint64 x, ostream& out=cout, string del=" ", string pre="");

/**
 * @brief Prints lower row of lower board of @p x, mirrored 
 * (left is right and right is left, up is down, down is up!)
 *
 * Prints in a simple format the lower row of lower board of @p x, 
 * in stream @p out.
 * String @p pre is outputed before row while @p del is appended to the end.
 *
 * @param[in]       x       bitset containing the row
 * @param[out]      out     output stream to print row to
 * @param[in]       del     string to print after row
 * @param[in]       pre     string to print before row
 */
void printLRow(uint64 x, ostream& out=cout, string del=" ", string pre="");

/**
 * @brief Computes index of bit on 64bit integer
 * 
 * Computes index of active bit in @p bb
 *
 * @pre                     @p bb has exactly one active bit
 * 
 * @param[in]       bb      bitset containing bit whose index is to be computed
 * 
 * @return                  index of bit in @p bb
 */
constexpr unsigned long int square(uint64 bb){
#ifdef _MSC_VER
    unsigned long int index;
    _BitScanForward64(&index, bb);
    return index;
#else
    return __builtin_ctzll(bb);
#endif
}

uint64 pop_lsb(uint64 &x);


template <typename T>
constexpr T rotateLeft(T x, unsigned int shftamnt){
    return (x << shftamnt) | (x >> ((sizeof(T) << 3) - shftamnt));
}

template <typename T, unsigned int shftamnt>
constexpr T rotateLeft(T x){
    return (x << shftamnt) | (x >> ((sizeof(T) << 3) - shftamnt));
}

template <typename T>
constexpr T rotateRight(T x, unsigned int shftamnt){
    return (x >> shftamnt) | (x << ((sizeof(T) << 3) - shftamnt));
}

template <typename T, unsigned int shftamnt>
constexpr T rotateRight(T x){
    return (x >> shftamnt) | (x << ((sizeof(T) << 3) - shftamnt));
}
#endif /* _BITUTILS_HPP */
