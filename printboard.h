#ifndef PRINTBOARD_H
#define PRINTBOARD_H

#include <cstdio>
#include <cstdint>
#include <iostream>
using namespace std;
void PrintBoard(std::uint64_t black,std::uint64_t white){
    constexpr const char * table[] = {"　","●","〇","　"};
    constexpr const char * lftable[] = {"\n-------------------------\n|","","","","","","",""};
    std::printf("-------------------------\n|");
    for(int i=63;0 <= i;--i){
        std::printf("%s|%s",table[(((black >> i) & 1) | (((white >> i) & 1) << 1))],lftable[i % 8]);
    }
}

inline void PrintBoard_single(std::uint64_t board){
    PrintBoard(board,0);
}

#endif