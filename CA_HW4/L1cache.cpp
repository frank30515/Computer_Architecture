#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "L1cache.h"
#include "Mem.h"
using namespace std;

L1cache::L1cache (Mem* memory,int way = 1) {
	L1readhit = 0;
	L1readmiss = 0;
	L1writehit = 0;
	L1writemiss = 0;
	mem = memory;
	way_number = way;
	for(int i=0; i < L1size; i++) {
		for (int j=0; j < 4; j++) {
			cache[i][j] = 0;
		}
	}
}

int L1cache::getfromCache(const int address) {
	//direct-map
    if (way_number == 1) {
        int block = (address / 4) % 8;
        int word = address % 4 + 4;
        int tag = address / 4; // tag也可以直接當作存回memory的address
        // 如果valid bit = 0, 從mem讀值
        if (cache[block][0] == 0) {
            int* index = mem->getfromMem(address/4); // 從mem讀取值
            for (int i = 0 ; i < 4 ; i++)
                cache[block][i+4] = index[i];
            cache[block][0] = 1; // 設valid bit =1
            cache[block][1] = 0; // 設dirty bit =0
            cache[block][3] = tag; // 更新tag
            L1readmiss++;
            return cache[block][word];
        }
        // 如果tag一樣，hit
        if (cache[block][3] == tag) {
            L1readhit++;
            return cache[block][word];
        }
        // tag不一樣，將原本存在裡面的值傳到memory
        else {
            if (cache[block][1] == 1) // 如果此dirty bit =1,將存在裡面的數值傳回mem更新
                mem->writetoMem(cache[block][3], cache[block]+4);
            int* index = mem->getfromMem(address/4);
            for (int i = 0 ; i < 4 ; i++)
                cache[block][i+4] = index[i];
            cache[block][0] = 1; // 設valid bit =1
            cache[block][1] = 0; // 設dirty bit =0
            cache[block][3] = tag; // 更新tag
            L1readmiss++;
            return cache[block][word];
        }
    } // 結束direct-map

    //two-way
    if (way_number == 2) {
        int block = 2*((address / 8) % 4);
        int word = address % 4 + 4;
        int tag = address / 4;
        // 如果兩個valid bit都是0，直接從mem拿值寫到第一個裡
        if (cache[block][0] == 0 && cache[block+1][0] == 0) {
            int* index = mem->getfromMem(address/4); // 從mem拿值
            for (int i = 0 ; i < 4 ; i++)
                cache[block][i+4] = index[i];
            cache[block][0] = 1; // 設valid=1
            cache[block][1] = 0; // 設dirty=0
            //同一block除了用的，其他release bit都--，只有用的是0,故基於LRU到時候會取release bit最小的
            for (int i = 0 ; i < 2 ; i++)
               	cache[block+i][2]--;
            cache[block][2] = 0;
            cache[block][3] = tag; // 更新tag
            L1readmiss++;
            return cache[block][word];
        }
        //two-way其中一個valid不是0，檢查其tag，如果符合直接read，不符合從mem抓值存到另外一個裡面
        if (cache[block][0] != 0 && cache[block+1][0] == 0) {
            if (cache[block][3] == tag) {
                for (int i = 0 ; i < 2 ; i++)
                	cache[block+i][2]--;
                cache[block][2] = 0;
                L1readhit++;
                return cache[block][word];
            }
            if (cache[block][3] != tag) {
                int* index = mem->getfromMem(address/4);
                for (int i = 0 ; i < 4 ; i++)
                    cache[block+1][i+4] = index[i];
                cache[block+1][0] = 1;
                cache[block+1][1] = 0;
                for (int i = 0 ; i < 2 ; i++)
                	cache[block+i][2]--;
                cache[block+1][2] = 0;
                cache[block+1][3] = tag;
                L1readmiss++;
                return cache[block+1][word];
            }
        }
        //同上，只是換成另外一個valid不是0
        if (cache[block][0] == 0 && cache[block+1][0] != 0) {
            if (cache[block+1][3] == tag) {
                for (int i = 0 ; i < 2 ; i++)
                	cache[block+i][2]--;
                cache[block+1][2] = 0;
                L1readhit++;
                return cache[block+1][word];
            }
            if (cache[block+1][3] != tag) {
                int* index = mem->getfromMem(address/4);
                for (int i = 0 ; i < 4 ; i++)
                    cache[block][i+4] = index[i];
                cache[block][0] = 1;
                cache[block][1] = 0;
                for (int i = 0 ; i < 2 ; i++)
                	cache[block+i][2]--;
                cache[block][2] = 0;
                cache[block][3] = tag;
                L1readmiss++;
                return cache[block][word];
            }
        }
        //兩個裡面valid都是1，檢查tag，如果都不符合，就根據LRU從mem抓值回來cache存
        else {
            if (cache[block][3] == tag) {
                for (int i = 0 ; i < 2 ; i++)
                	cache[block+i][2]--;
                cache[block][2] = 0;
                L1readhit++;
                return cache[block][word];
            }
            if (cache[block+1][3] == tag) {
                for (int i = 0 ; i < 2 ; i++)
                	cache[block+i][2]--;
                cache[block+1][2] = 0;
                L1readhit++;
                return cache[block+1][word];
            }
            //兩個tag都不符合，執行write back
            else {
                int replaced_block;
                if (cache[block][2] >= cache[block+1][2]) //找出最久沒用的，也就是找出release bit最小的
                    replaced_block = block+1;
                else
                    replaced_block = block;
            	//如果此位置dirty=1，write back 回mem
                if (cache[replaced_block][1] == 1)
                    mem->writetoMem(cache[replaced_block][3], cache[replaced_block]+4);
                int* index = mem->getfromMem(address/4); // 從mem抓值
                for (int i = 0 ; i < 4 ; i++)
                    cache[replaced_block][i+4] = index[i];
                cache[replaced_block][0] = 1;
                cache[replaced_block][1] = 0;
                for (int i = 0 ; i < 2 ; i++)
                	cache[block+i][2]--;
                cache[replaced_block][2] = 0;
                cache[replaced_block][3] = tag;
                L1readmiss++;
                return cache[replaced_block][word];
            }
        }
    } // 結束two-way

	//four-way
    if (way_number == 4) {
        int block = (address / 16) % 2;
        int word = address % 4 + 4;
        int tag = address / 4;

        for (int j = 0 ; j < 4 ; j++) {
            //如果此valid bit=0，而其他人tag沒有命中就寫他
			if (cache[block+j][0] == 0) {
                for (int i = 0 ; i < 4 ; i++) {
                	//看其他3個有沒有tag是符合的，沒有的話就把值寫在block+j裡面
                    if (cache[block+i][0] != 0 && cache[block+i][3] == tag && i != j) {
                        for (int k = 0 ; k < 4 ; i++)
                	    	cache[block+k][2]--;
                        cache[block+i][2] = 0;
                        L1readhit++;
                        return cache[block+i][word];
                    }
                }
                int* index = mem->getfromMem(address/4);
                for (int i = 0 ; i < 4 ; i++)
                    cache[block+j][i+4] = index[i];
                cache[block+j][0] = 1;
                cache[block+j][1] = 0;
                for (int i = 0 ; i < 4 ; i++)
                	cache[block+i][2]--;
                cache[block+j][2] = 0;
                cache[block+j][3] = tag;
                L1readmiss++;
                return cache[block+j][word];
            }
            //valid bit=1，看tag有沒有符合
            else {
                if (cache[block+j][3] == tag) {
                    for (int i = 0 ; i < 4 ; i++)
                		cache[block+i][2]--;
                    cache[block+j][2] = 0;
                    L1readhit++;
                    return cache[block+j][word];
                }
            }
        } // end for loop

        //4個全部valid皆是1，且tag都沒符合
        int replaced_block = block;
        //找出4個裡面最久沒用到的
        for (int i = 0 ; i < 4 ; i++){
        	if (cache[replaced_block][2] >= cache[block+i][2])
                replaced_block = block+i;
		}
        if (cache[replaced_block][1] == 1)
            mem->writetoMem(cache[replaced_block][3], cache[replaced_block]+4);
        int* index = mem->getfromMem(address/4);
        for (int i = 0 ; i < 4 ; i++)
            cache[replaced_block][i+4] = index[i];
        cache[replaced_block][0] = 1;
        cache[replaced_block][1] = 0;
        for (int i = 0 ; i < 4 ; i++)
        	cache[block+i][2]--;
        cache[replaced_block][2] = 0;
        cache[replaced_block][3] = tag;
        L1readmiss++;
        return cache[replaced_block][word];
    } // 結束four-way

	//full associative
    if (way_number == 8) {
        int block = 0;
        int word = address % 4 + 4;
        int tag = address / 4;
        for (int i = 0 ; i < 8 ; i++) {
            if (cache[block+i][0] == 0) {
                for (int j = 0 ; j < 8 ; j++) {
                    if (cache[block+j][0] != 0 && cache[block+j][3] == tag && i != j) {
                        for (int k = 0 ; k < 8 ; k++)
                		cache[block+k][2]--;
                    	cache[block+j][2] = 0;
                        L1readhit++;
                        return cache[block+j][word];
                    }
                }
                int* index = mem->getfromMem(address/4);
                for (int j = 0 ; j < 4 ; j++)
                    cache[block+i][j+4] = index[j];
                cache[block+i][0] = 1;
                cache[block+i][1] = 0;
                for (int j = 0 ; j < 8 ; j++)
                	cache[block+j][2]--;
                cache[block+i][2] = 0;
                cache[block+i][3] = tag;
                L1readmiss++;
                return cache[block+i][word];
            }
            else {
                if (cache[block+i][3] == tag) {
                    for (int j = 0 ; j < 8 ; j++)
                		cache[block+j][2]--;
                    cache[block+i][2] = 0;
                    L1readhit++;
                    return cache[block+i][word];
                }
            }
        } // end for loop
        int replaced_block = block;
        for (int i = 0 ; i < 8 ; i++)
            if (cache[replaced_block][2] >= cache[block+i][2])
                replaced_block = block+i;
        if (cache[replaced_block][1] == 1)
            mem->writetoMem(cache[replaced_block][3], cache[replaced_block]+4);
        int* index = mem->getfromMem(address/4);
        for (int i = 0 ; i < 4 ; i++)
            cache[replaced_block][i+4] = index[i];
        cache[replaced_block][0] = 1;
        cache[replaced_block][1] = 0;
        for (int i = 0 ; i < 8 ; i++)
        	cache[i][2]--;
        cache[replaced_block][2] = 0;
        cache[replaced_block][3] = tag;
        L1readmiss++;
        return cache[replaced_block][word];
    } // 結束full associative
}

void L1cache::writetoCache(const int address,const int indata) {
	//direct-map
    if (way_number == 1) {
        int block = (address / 4) % 8;
        int word = address % 4 + 4;
        int tag = address / 4;
        // 如果valid bit =0,須將mem對應的值傳進cache，再做寫的動作
        if (cache[block][0] == 0) {
            int* index = mem->getfromMem(address/4); // 從mem拿值
            for (int i = 0 ; i < 4 ; i++)
                cache[block][i+4] = index[i];
            cache[block][0] = 1; // 設valid bit =1
            cache[block][1] = 1; // 設dirty bit=1
            cache[block][3] = tag; // 更新tag
            cache[block][word] = indata; //將data寫進去
            L1writemiss++;
            return;
        }
        // valid bit=1, 如果tag一樣直接write進去，如果不一樣，根據dirty bit看要不要write back
        else {
            // tag一樣，直接寫進去
            if (cache[block][3] == tag) {
                cache[block][word] = indata;
                cache[block][1] = 1; //設dirty bit=1
                L1writehit++;
                return;
            }
            // tag不一樣，執行write back
            else {
                // 如果dirty bit =1, 將原本存在裡面的數值傳回mem
                if (cache[block][1] == 1)
                    mem->writetoMem(cache[block][3], cache[block]+4);
                int* index = mem->getfromMem(address/4); //將要的data傳回cache
                for (int i = 0 ; i < 4 ; i++)
                    cache[block][i+4] = index[i];
                cache[block][0] = 1;
                cache[block][1] = 1;
                cache[block][3] = tag;
                cache[block][word] = indata;
                L1writemiss++;
                return;
            }
        }
    }

    if (way_number == 2) {
        int block = 2*((address / 8) % 4);
        int word = (address % 4) + 4;
        int tag = address / 4;
        // 如果兩個valid都是0，從mem抓值回來，再write
        if (cache[block][0] == 0 && cache[block+1][0] == 0) {
            int* index = mem->getfromMem(address/4);
            for (int i = 0 ; i < 4 ; i++)
                cache[block][i+4] = index[i];
            cache[block][0] = 1;
            cache[block][1] = 1;
            for (int i = 0 ; i < 2 ; i++) //計算release bit
              	cache[block+i][2]--;
            cache[block][2] = 0;
            cache[block][3] = tag;
            cache[block][word] = indata;
            L1writemiss++;
            return;
        }
        //其中一個valid=1，檢查其tag，符合就直接write,不符合，從mem抓值存到另外一個裡面
        if (cache[block][0] == 0 && cache[block+1][0] != 0) {
            if (cache[block+1][3] == tag) {
                cache[block+1][1] = 1;
                for (int i = 0 ; i < 2 ; i++)
                	cache[block+i][2]--;
                cache[block+1][2] = 0;
                cache[block+1][word] = indata;
                L1writehit++;
                return;
            }
            if (cache[block+1][3] != tag) {
                int* index = mem->getfromMem(address/4);
                for (int i = 0 ; i < 4 ; i++)
                    cache[block][i+4] = index[i];
                cache[block][0] = 1;
                cache[block][1] = 1;
                for (int i = 0 ; i < 2 ; i++)
                	cache[block+i][2]--;
                cache[block][2] = 0;
                cache[block][3] = tag;
                cache[block][word] = indata;
                L1writemiss++;
                return;
            }
        }
        //同上，換成另外一個valid=1
        if (cache[block][0] != 0 && cache[block+1][0] == 0) {
            if (cache[block][3] == tag) {
                cache[block][1] = 1;
                for (int i = 0 ; i < 2 ; i++)
                	cache[block+i][2]--;
                cache[block][2] = 0;
                cache[block][word] = indata;
                L1writehit++;
                return;
            }
            if (cache[block][3] != tag) {
                int* index = mem->getfromMem(address/4);
                for (int i = 0 ; i < 4 ; i++)
                    cache[block+1][i+4] = index[i];
                cache[block+1][0] = 1;
                cache[block+1][1] = 1;
                for (int i = 0 ; i < 2 ; i++)
                	cache[block+i][2]--;
                cache[block+1][2] = 0;
                cache[block+1][3] = tag;
                cache[block+1][word] = indata;
                L1writemiss++;
                return;
            }
        }
        //兩個valid都是1，檢查哪個tag符合，都不符合的話用LRU找出最久沒用的取代它
        else{
            if (cache[block][3] == tag) {
                cache[block][1] = 1;
                for (int i = 0 ; i < 2 ; i++)
                	cache[block+i][2]--;
                cache[block][2] = 0;
                cache[block][word] = indata;
                L1writehit++;
                return;
            }
            if (cache[block+1][3] == tag) {
                cache[block+1][1] = 1;
                for (int i = 0 ; i < 2 ; i++)
                	cache[block+i][2]--;
                cache[block+1][2] = 0;
                cache[block+1][word] = indata;
                L1writehit++;
                return;
            }
            else {
                int replaced_block;
                if (cache[block][2] >= cache[block+1][2]) //選出最久沒用的
                    replaced_block = block+1;
                else
                    replaced_block = block;
                if (cache[replaced_block][1] == 1)
                    mem->writetoMem(cache[replaced_block][3], cache[replaced_block]+4);
                int* index = mem->getfromMem(address/4);
                for (int i = 0 ; i < 4 ; i++)
                    cache[replaced_block][i+4] = index[i];
                cache[replaced_block][0] = 1;
                cache[replaced_block][1] = 1;
                for (int i = 0 ; i < 2 ; i++)
                	cache[block+i][2]--;
                cache[replaced_block][2] = 0;
                cache[replaced_block][3] = tag;
                cache[replaced_block][word] = indata;
                L1writemiss++;
                return;
           	}
        }
    }

    if (way_number == 4) {
        int block = (address / 16) % 2;
        int word = address % 4 + 4;
        int tag = address / 4;

        for (int i = 0 ; i < 4 ; i++) {
        	//valid=1且tag符合，直接write
            if (cache[block+i][0] == 1 && cache[block+i][3] == tag) {
                cache[block+i][1] = 1;
                for (int k = 0 ; k < 4 ; k++)
                	cache[block+k][2]--;
                cache[block+i][2] = 0;
                cache[block+i][word] = indata;
                L1writehit++;
                return;
            }
        }
        //找到其中有一個valid=0，從mem抓值放到那個裡面
        for (int i = 0 ; i < 4 ; i++) {
            if (cache[block+i][0] == 0) {
                int* index = mem->getfromMem(address/4);
                for (int j = 0 ; j < 4 ; j++)
                    cache[block+i][j+4] = index[j];
                cache[block+i][0] = 1;
                cache[block+i][1] = 1;
                for (int k = 0 ; k < 4 ; k++)
                	cache[block+k][2]--;
                cache[block+i][2] = 0;
                cache[block+i][3] = tag;
                cache[block+i][word] = indata;
                L1writemiss++;
                return;
            }
        }
        //全部valid皆是1且tag都不符合
        int replaced_block = block;
        for (int i = 0 ; i < 4 ; i++)
            if (cache[replaced_block][2] >= cache[block+i][2])
                replaced_block = block+i;
        if (cache[replaced_block][1] == 1)
            mem->writetoMem(cache[replaced_block][3], cache[replaced_block]+4);
        int* index = mem->getfromMem(address/4);
        for (int j = 0 ; j < 4 ; j++)
            cache[replaced_block][j+4] = index[j];
        cache[replaced_block][0] = 1;
        cache[replaced_block][1] = 1;
        for (int i = 0 ; i < 4 ; i++)
        	cache[block+i][2]--;
        cache[replaced_block][2] = 0;
        cache[replaced_block][3] = tag;
        cache[replaced_block][word] = indata;
        L1writemiss++;
        return;
    }

    if (way_number == 8) {
        int block = 0;
        int word = address % 4 + 4;
        int tag = address / 4;
        for (int i = 0 ; i < 8 ; i++) {
        	//valid=1且tag符合
            if (cache[block+i][0] == 1 && cache[block+i][3] == tag) {
                cache[block+i][1] = 1;
                for (int j = 0 ; j < 8 ; j++)
                	cache[block+j][2]--;
                cache[block+i][2] = 0;
                cache[block+i][word] = indata;
                L1writehit++;
                return;
            }
        }
        //看哪個valid是0，把資料從mem拿出來再寫進去
        for (int i = 0 ; i < 8 ; i++) {
            if (cache[block+i][0] == 0) {
                int* index = mem->getfromMem(address/4);
                for (int j = 0 ; j < 4 ; j++)
                    cache[block+i][j+4] = index[j];
                cache[block+i][0] = 1;
                cache[block+i][1] = 1;
                for (int j = 0 ; j < 8 ; j++)
                	cache[block+j][2]--;
                cache[block+i][2] = 0;
                cache[block+i][3] = tag;
                cache[block+i][word] = indata;
                L1writemiss++;
                return;
            }
        }
        //全部valid皆是1且沒有tag符合
        int replaced_block = block;
        for (int i = 0 ; i < 8 ; i++)
            if (cache[replaced_block][2] >= cache[block+i][2])
                replaced_block = block+i;
        if (cache[replaced_block][1] == 1)
            mem->writetoMem(cache[replaced_block][3], cache[replaced_block]+4);
        int* index = mem->getfromMem(address/4);
        for (int j = 0 ; j < 4 ; j++)
            cache[replaced_block][j+4] = index[j];
        cache[replaced_block][0] = 1;
        cache[replaced_block][1] = 1;
        for (int i = 0 ; i < 8 ; i++)
        	cache[block+i][2]--;
        cache[replaced_block][2] = 0;
        cache[replaced_block][3] = tag;
        cache[replaced_block][word] = indata;
        L1writemiss++;
        return;
    }
}

int L1cache::getReadHit(void){
	int temp = L1readhit;
	return temp;
}
int L1cache::getReadMiss(void){
	int temp = L1readmiss;
	return temp;
}
int L1cache::getWriteHit(void){
	int temp = L1writehit;
	return temp;
}
int L1cache::getWriteMiss(void){
	int temp = L1writemiss;
	return temp;
}
int L1cache::getHit(void){
	int temp = L1readhit + L1writehit;
	return temp;
}
int L1cache::getMiss(void){
	int temp = L1readmiss + L1writemiss;
	return temp;
}
