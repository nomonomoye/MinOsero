#include <cstdlib>
#include <ctime>
#include <cstdint>

#include <iostream>
#include <vector>
#include <thread>

#include <intrin.h>
#include "printboard.h"

constexpr int THREAD_CNT = 4;

#define searchLegal(shift,self,enemy,blank,ret) {\
    std::uint64_t sself = ((self shift) & enemy) shift; \
    ret |= sself & blank;sself = ((sself & enemy) shift); \
    ret |= sself & blank;sself = ((sself & enemy) shift); \
    ret |= sself & blank;sself = ((sself & enemy) shift); \
    ret |= sself & blank;sself = ((sself & enemy) shift); \
    ret |= sself & blank;sself = ((sself & enemy) shift); \
    ret |= sself & blank; \
}\

inline std::uint64_t getLegalMove(const std::uint64_t self,const std::uint64_t enemy){
    const std::uint64_t blank = ~(self | enemy);
    const std::uint64_t mask_enemy = enemy & 0x7E7E7E7E7E7E7E7E;
    std::uint64_t ret = 0;

    searchLegal(>> 8,self,enemy,blank,ret);
    searchLegal(>> 1,self,mask_enemy,blank,ret);
    searchLegal(>> 9,self,mask_enemy,blank,ret);
    searchLegal(>> 7,self,mask_enemy,blank,ret);
    searchLegal(<< 8,self,enemy,blank,ret);
    searchLegal(<< 1,self,mask_enemy,blank,ret);
    searchLegal(<< 9,self,mask_enemy,blank,ret);
    searchLegal(<< 7,self,mask_enemy,blank,ret);

    return ret;
}

#define searchRev(shift,move,self,enemy,blank,ret){\
    std::uint64_t sself = (move shift) & enemy; \
    std::uint64_t result = sself; \
    while(sself & enemy){ \
        sself = sself shift; \
        result |= sself & enemy; \
    }\
    ret |= sself & self ? result : 0; \
}\

inline std::uint64_t getRevPos(const std::uint64_t move,const std::uint64_t self,std::uint64_t enemy){
    const std::uint64_t blank = ~(self | enemy);
    enemy &= ~self;
    const std::uint64_t mask_enemy = enemy & 0x7E7E7E7E7E7E7E7E;
    std::uint64_t ret = 0;

    searchRev(>> 8,move,self,enemy,blank,ret);
    searchRev(>> 1,move,self,mask_enemy,blank,ret);
    searchRev(>> 9,move,self,mask_enemy,blank,ret);
    searchRev(>> 7,move,self,mask_enemy,blank,ret);
    searchRev(<< 8,move,self,enemy,blank,ret);
    searchRev(<< 1,move,self,mask_enemy,blank,ret);
    searchRev(<< 9,move,self,mask_enemy,blank,ret);
    searchRev(<< 7,move,self,mask_enemy,blank,ret);

    return ret;
}

inline void turnMove(std::uint64_t n,std::uint64_t &self,std::uint64_t &enemy){
    std::uint64_t rev = getRevPos(n,self,enemy);
    self |= rev | n;
    enemy &= ~rev;
}

inline int checkResult(std::uint64_t black,std::uint64_t white){
    int wcnt = __popcnt64(white);
    int bcnt = __popcnt64(black);

    return wcnt < bcnt ? 1 : wcnt > bcnt ? -1 : 0;
}

int randexec(std::uint8_t nowturn,std::uint64_t b1,std::uint64_t b2){
    std::uint64_t mylegal = getLegalMove(b1,b2);
    if(!mylegal){
        if(!getLegalMove(b2,b1)){
            if(!nowturn){
                return checkResult(b1,b2);
            }
            return checkResult(b2,b1);
        }else{
            return randexec(nowturn ^ 1,b2,b1);
        }
    }

    int cnt = (rand() % __popcnt64(mylegal)) + 1;
    std::uint64_t n;
    while(cnt){
        n = mylegal & -mylegal;
        mylegal ^= n;
        --cnt;
    }

    turnMove(n,b1,b2);

    return randexec(nowturn ^ 1,b2,b1);
}

void mineMain(std::uint64_t minelevel,const std::uint64_t self,const std::uint64_t enemy,std::vector<int64_t> & result,std::uint64_t legal){
    srand((unsigned)time(NULL));
    int32_t cnt = 0;
    while(legal){
        std::uint64_t b1 = self,b2 = enemy;
        std::uint64_t n = legal & -legal;
        turnMove(n,b1,b2);
        for(std::uint64_t i=0;i<minelevel;++i){
            result[cnt] += randexec(1,b2,b1);
        }
        
        legal ^= n;
        ++cnt;
    }
}

std::uint64_t mineMain_thread(std::uint64_t minelevel,std::uint64_t black,std::uint64_t white){
    std::uint64_t legal = getLegalMove(black,white);
    if(legal <= 0){return 0;}

    std::size_t cnt = __popcnt64(legal);
    std::vector<std::vector<int64_t>> resultList(THREAD_CNT,std::vector<int64_t>(cnt,0));

    std::vector<std::thread> threadList;
    for(int i=0;i<THREAD_CNT;++i){
        threadList.push_back(std::thread([&,i]{mineMain(minelevel,black,white,resultList[i],legal);}));
    }

    for(auto & i : threadList){
        i.join();
    }

    int64_t maxval = LLONG_MIN;
    int maxcnt = 0;
    for(int i=0;i<cnt;++i){
        int64_t now = 0;
        for(auto & j : resultList){
            now += j[i];
        }
        cout << now << endl;
        if(maxval < now){
            maxval = now;
            maxcnt = i;
        }
    }

    std::uint64_t ret = legal & -legal;
    for(int i=0;i<maxcnt;++i){
        legal ^= ret;
        ret = legal & -legal;
    }

    if(ret == 0){
        cerr << "error" << endl;
        exit(1);
    }
    return ret;
}

void testcode1(){
    std::uint64_t black_b = 0x8A62819732468732;
    std::uint64_t white_b = 0x83CADFE9837F7346;

    std::uint64_t black = black_b & ~(black_b & white_b);
    std::uint64_t white = white_b & ~(black_b & white_b);

    PrintBoard(black,white);
    PrintBoard_single(getLegalMove(black,white));
    PrintBoard_single(getRevPos(0x80,black,white));
}

int main(){
    std::uint64_t black = 0x0000001008000000;
    std::uint64_t white = 0x0000000810000000;
    constexpr int minelevel = 10000;

    int turn = 0;
    
    while(getLegalMove(black,white) || getLegalMove(white,black)){
        if(turn){
            turnMove(mineMain_thread(minelevel,white,black),white,black);
        }else{
            turnMove(mineMain_thread(minelevel,black,white),black,white);
        }

        turn ^= 1;
        PrintBoard(black,white);
    }
}

