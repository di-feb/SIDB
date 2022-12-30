#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "sht_table.h"
#include "ht_table.h"
#include "record.h"

#define UNITIALLIZED -1
#define MAX_RECORDS_PER_BLOCK (BF_BLOCK_SIZE - sizeof(SHT_block_info)) / (sizeof(Record))
#define BYTES_UNTIL_NUM_OF_RECORDS BF_BLOCK_SIZE - sizeof(SHT_block_info) + sizeof(int) + sizeof(int)
#define BYTES_UNTIL_NEXT BF_BLOCK_SIZE - sizeof(SHT_block_info) + sizeof(int)
#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

// Mallocs and initiallizes a struct SHT_info
SHT_info* createSHT_info(char* fileName, int fileDescriptor, int numOfBuckets, size_t size){
    // Allocate the struct SHT_info
    SHT_info* info = malloc(sizeof(*info)); 
    // Initiallize it
    info->isSecondaryHashTable = true;
    info->fileName = malloc(size);
    strcpy(info->fileName, fileName);
    info->fileDesc = fileDescriptor;
    info->numOfBuckets = numOfBuckets;

    return info;
}

// Mallocs and initiallizes a struct HT_block_info
SHT_block_info* createSHT_block_info(int index, int next){
    // Allocate the struct with the data
    SHT_block_info* blockInfo = malloc(sizeof(*blockInfo)); 
    // Initiallize it
    blockInfo->blockIndex = index;
    blockInfo->next = next;
    blockInfo->numOfRecords = 0;

    return blockInfo;
}

// Allocated a new block and returns its ID.
static int createBlock(SHT_info* info){
    BF_Block* block;
	BF_Block_Init(&block);
		
    // Allocate a new block
	CALL_OR_DIE(BF_AllocateBlock(info->fileDesc, block));

    // Malloc a pointer where we will write the total blocks number
    int *blocksNum = malloc(sizeof(int)); 

    // Get the total numbers of the blocks
	CALL_OR_DIE(BF_GetBlockCounter(info->fileDesc, blocksNum));

    // Get the index of the last block we just allocated
	int index = *blocksNum - 1;
    // Get the block with this specific index
	CALL_OR_DIE(BF_GetBlock(info->fileDesc, index, block));

    // Get the data of this block
	char *data = BF_Block_GetData(block);

    // Allocate the SHT_block_info struct and initiallize it.
    SHT_block_info* blockInfo = createSHT_block_info(index, -1);

    // Move to the end of the block to store the struct BlockInfo
    data += BF_BLOCK_SIZE - sizeof(*blockInfo);

	memcpy(data, blockInfo, sizeof(*blockInfo));

	BF_Block_SetDirty(block);
    CALL_OR_DIE(BF_UnpinBlock(block));
	BF_Block_Destroy(&block);

    // Store the blockId so we can return it
	int blockId = blockInfo->blockIndex;

    // Memory managment
	free(blockInfo);
	free(blocksNum);

    return blockId;
}

// Creates the buckets of the hashTable
static void createBuckets(SHT_info* info){
    
    BF_Block* block;
    BF_Block_Init(&block);

    CALL_OR_DIE(BF_AllocateBlock(info->fileDesc, block));   // Allocate a new block

    int* array_of_buckets = malloc(info->numOfBuckets * sizeof(int));
    for (int i = 0; i < info->numOfBuckets; i++) 
        array_of_buckets[i] = -1;

    // Get the data of the block we just allocated.
    char* data = BF_Block_GetData(block); 
    // Pass the array to block data
    memcpy(data, array_of_buckets, info->numOfBuckets * sizeof(int));

    SHT_block_info* blockInfo = createSHT_block_info(1, 2);
    data += BF_BLOCK_SIZE - sizeof(*blockInfo);
    // Pass the array to block data
    memcpy(data, blockInfo, sizeof(*blockInfo));

    // Write the block back to the disk.
    BF_Block_SetDirty(block);

    free(array_of_buckets);
    free(blockInfo);
    BF_Block_Destroy(&block);
}

// Check if a specific bucket is unitiallized.
// If it is allocate a new block and let bucket point to that block.
// If it is NOT just return and do nothing.
static void checkBucket(SHT_info* info, int* buckets, int hashedId){
    // If the bucket is initiallized return
    if(buckets[hashedId] != UNITIALLIZED)
        return;

    BF_Block* block;
    BF_Block_Init(&block); // Initiallize the struct BF_Block.
    // If it isn't:
    // Add the new block inside the bucket at the position of the hashed id.
    buckets[hashedId] = createBlock(info);

    // Get the block that holds all the buckets
    CALL_OR_DIE(BF_GetBlock(info->fileDesc, 1, block));
    // Get the data of this block
	char* data = BF_Block_GetData(block);
    // Go to the right data's position
    data += hashedId * sizeof(int); 
    // Store the block into the bucket position
    memcpy(data, &buckets[hashedId], sizeof(int));

    // Write the block back to the disk.
    BF_Block_SetDirty(block);
    CALL_OR_DIE(BF_UnpinBlock(block));
    // Memory managment
    BF_Block_Destroy(&block);
}

// Frees the memory of the structs SHT_info, SHT_block_info.
static void infoDestroy(SHT_info* info, SHT_block_info* block_info){
    free(info->fileName);
    free(info);
    free(block_info);
}

// Checks if a specific file is a secondaryhashTable file
bool isSecondaryHashTable(SHT_info* info){
    if(info->isSecondaryHashTable)
        return false;
    return true;
}

int SHT_CreateSecondaryIndex(char *sfileName,  int buckets, char* fileName){
    BF_Block* block;

    BF_Block_Init(&block); // Initiallize the struct BF_Block.
	CALL_OR_DIE(BF_CreateFile(sfileName)); // Create a file which consists of blocks. 

    int fileDescriptor; // The file descriptor of the file we are going to open.
	CALL_OR_DIE(BF_OpenFile(sfileName, &fileDescriptor)); // Open the file

    CALL_OR_DIE(BF_AllocateBlock(fileDescriptor, block));

    char* data = BF_Block_GetData(block);

    // Size of fileName
    size_t sizeOfFilename = sizeof(fileName);
    // Create struct SHT_info
	SHT_info* info = createSHT_info(fileName, fileDescriptor, buckets, sizeOfFilename); 
    // Store the info into the data
    memcpy(data, info, sizeof(*info));

    // Create struct SHT_block_info
    SHT_block_info* blockInfo = createSHT_block_info(0, 1);

    // Move to the end of the block to store the struct BlockInfo
    data += BF_BLOCK_SIZE - sizeof(*blockInfo);
    // Store the info
    memcpy(data, blockInfo, sizeof(*blockInfo));

    // create the buckets
    createBuckets(info);

    // Write the block back to the disk.
    BF_Block_SetDirty(block);

    // Memory managment
    BF_Block_Destroy(&block);
    free(info);
    free(blockInfo);


}

SHT_info* SHT_OpenSecondaryIndex(char *indexName){

}


int SHT_CloseSecondaryIndex( SHT_info* SHT_info ){

}

int SHT_SecondaryInsertEntry(SHT_info* sht_info, Record record, int block_id){

}

int SHT_SecondaryGetAllEntries(HT_info* ht_info, SHT_info* sht_info, char* name){

}



