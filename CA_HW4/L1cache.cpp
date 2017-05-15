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
        int tag = address / 4; // tag�]�i�H������@�s�^memory��address
        // �p�Gvalid bit = 0, �qmemŪ��
        if (cache[block][0] == 0) {
            int* index = mem->getfromMem(address/4); // �qmemŪ����
            for (int i = 0 ; i < 4 ; i++)
                cache[block][i+4] = index[i];
            cache[block][0] = 1; // �]valid bit =1
            cache[block][1] = 0; // �]dirty bit =0
            cache[block][3] = tag; // ��stag
            L1readmiss++;
            return cache[block][word];
        }
        // �p�Gtag�@�ˡAhit
        if (cache[block][3] == tag) {
            L1readhit++;
            return cache[block][word];
        }
        // tag���@�ˡA�N�쥻�s�b�̭����ȶǨ�memory
        else {
            if (cache[block][1] == 1) // �p�G��dirty bit =1,�N�s�b�̭����ƭȶǦ^mem��s
                mem->writetoMem(cache[block][3], cache[block]+4);
            int* index = mem->getfromMem(address/4);
            for (int i = 0 ; i < 4 ; i++)
                cache[block][i+4] = index[i];
            cache[block][0] = 1; // �]valid bit =1
            cache[block][1] = 0; // �]dirty bit =0
            cache[block][3] = tag; // ��stag
            L1readmiss++;
            return cache[block][word];
        }
    } // ����direct-map

    //two-way
    if (way_number == 2) {
        int block = 2*((address / 8) % 4);
        int word = address % 4 + 4;
        int tag = address / 4;
        // �p�G���valid bit���O0�A�����qmem���ȼg��Ĥ@�Ӹ�
        if (cache[block][0] == 0 && cache[block+1][0] == 0) {
            int* index = mem->getfromMem(address/4); // �qmem����
            for (int i = 0 ; i < 4 ; i++)
                cache[block][i+4] = index[i];
            cache[block][0] = 1; // �]valid=1
            cache[block][1] = 0; // �]dirty=0
            //�P�@block���F�Ϊ��A��Lrelease bit��--�A�u���Ϊ��O0,�G���LRU��ɭԷ|��release bit�̤p��
            for (int i = 0 ; i < 2 ; i++)
               	cache[block+i][2]--;
            cache[block][2] = 0;
            cache[block][3] = tag; // ��stag
            L1readmiss++;
            return cache[block][word];
        }
        //two-way�䤤�@��valid���O0�A�ˬd��tag�A�p�G�ŦX����read�A���ŦX�qmem��Ȧs��t�~�@�Ӹ̭�
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
        //�P�W�A�u�O�����t�~�@��valid���O0
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
        //��Ӹ̭�valid���O1�A�ˬdtag�A�p�G�����ŦX�A�N�ھ�LRU�qmem��Ȧ^��cache�s
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
            //���tag�����ŦX�A����write back
            else {
                int replaced_block;
                if (cache[block][2] >= cache[block+1][2]) //��X�̤[�S�Ϊ��A�]�N�O��Xrelease bit�̤p��
                    replaced_block = block+1;
                else
                    replaced_block = block;
            	//�p�G����mdirty=1�Awrite back �^mem
                if (cache[replaced_block][1] == 1)
                    mem->writetoMem(cache[replaced_block][3], cache[replaced_block]+4);
                int* index = mem->getfromMem(address/4); // �qmem���
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
    } // ����two-way

	//four-way
    if (way_number == 4) {
        int block = (address / 16) % 2;
        int word = address % 4 + 4;
        int tag = address / 4;

        for (int j = 0 ; j < 4 ; j++) {
            //�p�G��valid bit=0�A�Ө�L�Htag�S���R���N�g�L
			if (cache[block+j][0] == 0) {
                for (int i = 0 ; i < 4 ; i++) {
                	//�ݨ�L3�Ӧ��S��tag�O�ŦX���A�S�����ܴN��ȼg�bblock+j�̭�
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
            //valid bit=1�A��tag���S���ŦX
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

        //4�ӥ���valid�ҬO1�A�Btag���S�ŦX
        int replaced_block = block;
        //��X4�Ӹ̭��̤[�S�Ψ쪺
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
    } // ����four-way

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
    } // ����full associative
}

void L1cache::writetoCache(const int address,const int indata) {
	//direct-map
    if (way_number == 1) {
        int block = (address / 4) % 8;
        int word = address % 4 + 4;
        int tag = address / 4;
        // �p�Gvalid bit =0,���Nmem�������ȶǶicache�A�A���g���ʧ@
        if (cache[block][0] == 0) {
            int* index = mem->getfromMem(address/4); // �qmem����
            for (int i = 0 ; i < 4 ; i++)
                cache[block][i+4] = index[i];
            cache[block][0] = 1; // �]valid bit =1
            cache[block][1] = 1; // �]dirty bit=1
            cache[block][3] = tag; // ��stag
            cache[block][word] = indata; //�Ndata�g�i�h
            L1writemiss++;
            return;
        }
        // valid bit=1, �p�Gtag�@�˪���write�i�h�A�p�G���@�ˡA�ھ�dirty bit�ݭn���nwrite back
        else {
            // tag�@�ˡA�����g�i�h
            if (cache[block][3] == tag) {
                cache[block][word] = indata;
                cache[block][1] = 1; //�]dirty bit=1
                L1writehit++;
                return;
            }
            // tag���@�ˡA����write back
            else {
                // �p�Gdirty bit =1, �N�쥻�s�b�̭����ƭȶǦ^mem
                if (cache[block][1] == 1)
                    mem->writetoMem(cache[block][3], cache[block]+4);
                int* index = mem->getfromMem(address/4); //�N�n��data�Ǧ^cache
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
        // �p�G���valid���O0�A�qmem��Ȧ^�ӡA�Awrite
        if (cache[block][0] == 0 && cache[block+1][0] == 0) {
            int* index = mem->getfromMem(address/4);
            for (int i = 0 ; i < 4 ; i++)
                cache[block][i+4] = index[i];
            cache[block][0] = 1;
            cache[block][1] = 1;
            for (int i = 0 ; i < 2 ; i++) //�p��release bit
              	cache[block+i][2]--;
            cache[block][2] = 0;
            cache[block][3] = tag;
            cache[block][word] = indata;
            L1writemiss++;
            return;
        }
        //�䤤�@��valid=1�A�ˬd��tag�A�ŦX�N����write,���ŦX�A�qmem��Ȧs��t�~�@�Ӹ̭�
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
        //�P�W�A�����t�~�@��valid=1
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
        //���valid���O1�A�ˬd����tag�ŦX�A�����ŦX���ܥ�LRU��X�̤[�S�Ϊ����N��
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
                if (cache[block][2] >= cache[block+1][2]) //��X�̤[�S�Ϊ�
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
        	//valid=1�Btag�ŦX�A����write
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
        //���䤤���@��valid=0�A�qmem��ȩ�쨺�Ӹ̭�
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
        //����valid�ҬO1�Btag�����ŦX
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
        	//valid=1�Btag�ŦX
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
        //�ݭ���valid�O0�A���Ʊqmem���X�ӦA�g�i�h
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
        //����valid�ҬO1�B�S��tag�ŦX
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
