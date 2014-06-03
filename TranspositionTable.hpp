#ifndef TRANSPOSITIONTABLE_H_
#define TRANSPOSITIONTABLE_H_
#include "Communication/Protocol.hpp"
#include "BitUtils.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

typedef uint64 tthash;
typedef uint64 ttdata;
typedef uint32_t hdata;


struct ttEntry{
    tthash hashXORdata;
    ttdata data;
};

#define tte_sz          (sizeof(hdata)*8)

#define tte_bits_sstate (2)
#define tte_bits_sdepth (6)
#define tte_bits_killer (5)
#define tte_bits_sscore (tte_sz-tte_bits_killer-tte_bits_sstate-tte_bits_sdepth)

#define tte_shft_sstate (0)
#define tte_shft_sdepth (tte_bits_sstate)
#define tte_shft_killer (tte_bits_sstate + tte_bits_sdepth)
#define tte_shft_sscore (tte_bits_sstate + tte_bits_sdepth + tte_bits_killer)

#define tte_bits_prmask (tte_bits_sdepth + tte_bits_sstate)
#define tte_shft_prmask (0)
#define tte_mask_prmask ((1 << tte_bits_prmask) - 1)

#define tte_mask_sdepth ((1 << tte_bits_sdepth) - 1)
#define tte_mask_sstate ((1 << tte_bits_sstate) - 1)
#define tte_mask_killer ((1 << tte_bits_killer) - 1)
#define tte_mask_sscore ((1 << tte_bits_sscore) - 1)

#define ss_mask         (3 << tte_shft_sstate)

enum NodeType{
    All_Node          = 0,                      //No score > alpha
    Cut_Node          = (1 << tte_shft_sstate), //Beta Cut-Off
    PV__Node          = (2 << tte_shft_sstate)  //Exact Score
};

#define TRANSPOSITION_TABLE_SIZE (0x7fffff)
#define getTTIndex(x) ((x) % TRANSPOSITION_TABLE_SIZE)

#define NULL_MOVE   (-1)
#define EXACT_SCORE (-2)

class TranspositionTable{
private:
    ttEntry mem[TRANSPOSITION_TABLE_SIZE];

public:
    TranspositionTable(){
        assert(sizeof(ttdata) == 8);
        assert(sizeof(hdata)  == 4);
    }

    inline int retrieveTTEntry(tthash hash, unsigned int depth, 
                            int32_t &alpha, int32_t &beta, bool placer) const{
        int index = getTTIndex(hash);

        const ttEntry * entr = mem + index;

        tthash hashXD   = entr->hashXORdata;
        ttdata data     = entr->data;

        if ((hashXD ^ data) != hash)                        return NULL_MOVE;

        hdata ndata = (placer) ? (data) : (data >> tte_sz);

        if (depth <= ((ndata >> tte_shft_sdepth) & tte_mask_sdepth)){
            NodeType bs = (NodeType) ((ndata >> tte_shft_sstate) & tte_mask_sstate);
            int32_t  sc = (ndata >> tte_shft_sscore) & tte_mask_sscore;
            if (sc & (1 << (tte_bits_sscore-1))) sc |= ~tte_mask_sscore;

            // entry's depth is greater than current, update bounds
            if (bs == Cut_Node){        //entry contains a Cut-Node, alpha...
                if (sc > alpha) alpha   = sc;
            } else if (bs == All_Node){ //entry contains an All-Node, beta...
                if (sc < beta)  beta    = sc;
                return NULL_MOVE;       //All-Nodes do not produce a killer move
            } else {                    //entry contains a Pv-Node, replace...
                alpha   = sc;
                beta    = sc;
            }
        }
        return (ndata >> tte_shft_killer) & tte_mask_killer;
    }

    inline void addTTEntry(tthash hash, unsigned int depth, 
                            int move, int32_t score, bool placer, NodeType ss){
        // if (placer) score = -score;

        assert(!((depth >> tte_shft_sdepth) & ~tte_mask_sdepth));
        assert(( move & tte_mask_killer) ==  move);
        int32_t sc = ((score << tte_shft_sscore) >> tte_shft_sscore) & tte_mask_sscore;
        if (sc & (1 << (tte_bits_sscore-1))) sc |= ~tte_mask_sscore;
        assert(sc == score);
        assert((depth & tte_mask_sdepth) == depth);
        assert((   ss & tte_mask_sstate) == ss);

        int index = getTTIndex(hash);

        ttEntry * entry = mem + index;
        ttdata data = entry->data;
        
        //replace rule
        hdata old_data = (placer) ? (data) : (data >> tte_sz);
        hdata priotity = (old_data >> tte_shft_prmask) & tte_mask_prmask;

        hdata newData    = depth << tte_shft_sdepth;
        newData         |=    ss << tte_shft_sstate;

        if (priotity >= newData){ //priority check
            return;
        }

        newData         |=  move << tte_shft_killer;
        newData         |= score << tte_shft_sscore;


        ttdata nFData = newData ^ old_data;

        nFData  = (placer) ? (nFData) : (nFData << tte_sz);
        nFData ^= data;

        tthash newHXD = hash ^ nFData;
        
        entry->hashXORdata = newHXD;
        entry->data = nFData;

        assert((newHXD ^ nFData) == hash);
        hdata ndata = (placer) ? (nFData) : (nFData >> tte_sz);
        assert(depth == ((ndata >> tte_shft_sdepth) & tte_mask_sdepth));
        assert(ss == (NodeType) ((ndata >> tte_shft_sstate) & tte_mask_sstate));
        sc = (ndata >> tte_shft_sscore) & tte_mask_sscore;
        if (sc & (1 << (tte_bits_sscore-1))) sc |= ~tte_mask_sscore;
        assert(sc == score);
        assert(move == ((ndata >> tte_shft_killer) & tte_mask_killer));
    }

    bool extractBest(BitBoard_t board, player pl, Move * m){
        tthash hash = board.getHash();
        int index = getTTIndex(hash);

        const ttEntry * entr = mem + index;

        tthash hashXD   = entr->hashXORdata;
        ttdata data     = entr->data;

        if ((hashXD ^ data) != hash) {
            assert(false);
            return false;
        }

        hdata nd = (pl == PLACER) ? (data) : (data >> tte_sz);
        int km = (nd >> tte_shft_killer) & tte_mask_killer;

        BitBoard_t::intToMove(m, km, pl);
        return true;
    }

    string extractPV(BitBoard_t board, player pl){
        stringstream pv;
        Move m;
        while (1){
            tthash hash = board.getHash();
            int index = getTTIndex(hash);

            const ttEntry * entr = mem + index;

            tthash hashXD   = entr->hashXORdata;
            ttdata data     = entr->data;

            if ((hashXD ^ data) != hash)                        return pv.str();

            hdata nd = (pl == PLACER) ? (data) : (data >> tte_sz);

            NodeType bs =(NodeType) ((nd >> tte_shft_sstate) & tte_mask_sstate);

            if (bs == All_Node)                                 return pv.str();

            int km = (nd >> tte_shft_killer) & tte_mask_killer;

            //board's hash is unique per board!!! no collisions...
            //so killer move must be valid 
            if (pl == PLACER){
                board.makePlace(1 << km);
            } else {
                board.move(km);
            }

            BitBoard_t::intToMove(&m, km, pl);
            pv << hex << setw(2) << m << " ";
            pl = getOtherPlayer(pl);
        }
    }
};

#endif /* TRANSPOSITIONTABLE_H_ */