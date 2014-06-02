/**
 * @file BitUtils.cpp
 * 
 * @brief Definition useful utilities to be used with bitsets
 */
#include "BitUtils.hpp"

void printLBoard(uint64 x, ostream& out, string del, string pre){
    out << pre;
    for (int k = 0x8000 ; k > 0 ; k >>= 1) 
        out << ((k & x) ? 1 : 0) << ((k & 0x1111) ? del : "");
}

void printLRow(uint64 x, ostream& out, string del, string pre){
    out << pre;
    for (int k = 0x8 ; k > 0 ; k >>= 1) 
        out << ((k & x) ? 1 : 0) << ((k & 0x1111) ? del : "");
}

uint64 pop_lsb(uint64 &x){
    uint64 t = x & -x;
    x ^= t;
    return t;
}