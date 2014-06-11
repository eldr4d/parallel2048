#include "Communication/Protocol.hpp"
#include "Board/BitBoard.hpp"

using namespace std;

ostream& operator<<(ostream& os, const Move& obj){
    switch (obj.dir){
        case ((int) bitNormalMove::b_left ): os << "L"; break;
        case ((int) bitNormalMove::b_right): os << "R"; break;
        case ((int) bitNormalMove::b_up   ): os << "U"; break;
        case ((int) bitNormalMove::b_down ): os << "D"; break;
        case (-1)                          :
                os << (v ? 2 : 4) << "@r" << obj.row << "c" << obj.col;
                break;

        default       : assert(false); os << "UNKNOWN"; break;
    }
    return os;
}

bool validMove(const Move &obj){
    switch (obj.dir){
        case ((int) bitNormalMove::b_left ): return true;
        case ((int) bitNormalMove::b_right): return true;
        case ((int) bitNormalMove::b_up   ): return true;
        case ((int) bitNormalMove::b_down ): return true;
        case (-1)                          : return true;
        default                            : return false;
    }
    return false;
}