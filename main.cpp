#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <dirent.h>
#include <openssl/md5.h>
#include "bitset.h"
#include "error.h"

#define SIM_HASH_HOP 1
#define SIM_HASH_LEN 8

void add_to_sim_hash(char * schingle,int hash_len, int sh_state[])
{
    char myHash[128/8];
    if(*((unsigned long*)schingle) == 0)
    {
        return;
    }
    MD5((const unsigned char*)schingle,(size_t)hash_len,(unsigned char*)myHash);
    int byte_idx = 0;
    int my_mask = 0x80;
    char byte = myHash[byte_idx];
    for(int i = 0; i < 128; i++)
    {
        sh_state[i] += (byte & my_mask) ? +1 : -1;
        my_mask >>= 1;
        if(!my_mask)
        {
            my_mask = 0x80;
            byte_idx++;
            byte = myHash[byte_idx];
        }
    }
}

bool process_file(char*fileName)
{
    int sh_state[128] = {0};
    std::ifstream inputFile;
    inputFile.open(fileName,std::ifstream::binary);
    if (inputFile.is_open())
    {
        inputFile.seekg(0,inputFile.end);
        int fileSize = inputFile.tellg();
        inputFile.seekg(0,inputFile.beg);
        char * myBuf = new (std::nothrow) char [fileSize];
        if(myBuf == nullptr)
        {
            printf("Error: memory could not be allocated.\n");
            exit(0);
        }
        inputFile.read(myBuf,fileSize);
        if (!inputFile)
        {
            std::cout<<"Error: loading file "<<fileName<<std::endl;
            exit(0);
        }
        int myPtr = 0;
        while(myPtr <= (fileSize - SIM_HASH_LEN))
        {
            add_to_sim_hash((myBuf+myPtr),SIM_HASH_LEN,sh_state);
            myPtr += SIM_HASH_HOP;
        }
        bitset_alloc(myRes,128);
        for(int i = 0; i < 128; i++)
        {
            if(sh_state[i] > 0)
            {
                bitset_setbit(myRes,i,1);
            }
        }
        for(int i=0; i<128/8;i++)
        {
            printf("%02x",*(((unsigned char*)(myRes+1))+i));
        }
        printf(": ");
        std::cout<<fileName<<std::endl;
        inputFile.close();
        delete[] myBuf;
    }else{
        std::cout<<"Error occurred while opening file"<<" "<<fileName<<"."<<std::endl;
        return false;
    }
    return true;
}

bool processDir(const char*dirName)
{
    DIR* myDir = opendir(dirName);
    struct dirent* directory;
    if(!myDir)
    {
        printf("Error: could not open directory: %s\n",dirName);
        return false;
    }
    while((directory = readdir(myDir)))
    {
        switch (directory->d_type)
        {
            case DT_DIR:
                if(strcmp(".",directory->d_name) && (strcmp("..",directory->d_name)))
                {
                    char* tempDirName = new (std::nothrow) char [PATH_MAX+1];
                    if(tempDirName == nullptr)
                    {
                        printf("Error: memory could not be allocated.\n");
                        exit(0);
                    }
                    strcpy(tempDirName,dirName);
                    strcat(tempDirName,"/");
                    strcat(tempDirName,directory->d_name);
                    processDir(tempDirName);
                    delete[] tempDirName;
                }
                break;
            case DT_REG:
                process_file(directory->d_name);
                break;
            default:
                break;
        }
    }
    closedir(myDir);
    return true;
}

int main()
{
    processDir("/home/viktor/huhu");
    return 0;
}
