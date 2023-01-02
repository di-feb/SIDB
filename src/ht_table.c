#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "bf.h"
#include "ht_table.h"
#include "record.h"

#define UNITIALLIZED -1
#define MAX_RECORDS_PER_BLOCK (BF_BLOCK_SIZE - sizeof(HT_block_info)) / (sizeof(Record))
#define BYTES_UNTIL_NUM_OF_RECORDS BF_BLOCK_SIZE - sizeof(HT_block_info) + sizeof(int) + sizeof(int)
#define BYTES_UNTIL_NEXT BF_BLOCK_SIZE - sizeof(HT_block_info) + sizeof(int)
#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

// Mallocs and initiallizes a struct HT_info
HT_info* createHT_info(char* fileName, int fileDescriptor, int numOfBuckets, size_t size){
    // Allocate the struct with the data
    HT_info* info = malloc(sizeof(*info)); 
    // Initiallize it
    info->isHashTable = true;
    info->fileName = malloc(size);
    strcpy(info->fileName, fileName);
    info->fileDesc = fileDescriptor;
    info->numOfBuckets = numOfBuckets;

    return info;
}

// Mallocs and initiallizes a struct HT_block_info
HT_block_info* createHT_block_info(int index, int next){
    // Allocate the struct with the data
    HT_block_info* blockInfo = malloc(sizeof(*blockInfo)); 
    // Initiallize it
    blockInfo->blockIndex = index;
    blockInfo->next = next;
    blockInfo->numOfRecords = 0;

    return blockInfo;
}

// Allocated a new block and returns its ID.
int createBlock(HT_info* info){
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

    // Allocate the HT_block_info struct and initiallize it.
    HT_block_info* blockInfo = createHT_block_info(index, -1);

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
void createBuckets(HT_info* info){
    
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

    HT_block_info* blockInfo = createHT_block_info(1, 2);
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
void checkBucket(HT_info* info, int* buckets, int hashedId){
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

// Frees the memory of the structs HT_info, HT_block_info.
void infoDestroy(HT_info* info, HT_block_info* block_info){
    free(info->fileName);
    free(info);
    free(block_info);
}

// Checks if a specific file is a hashTable file
bool isHashTable(HT_info* info){
    if(info->isHashTable)
        return false;
    return true;
}

int HT_CreateFile(char *fileName, int buckets){
    BF_Block* block;

    BF_Block_Init(&block); // Initiallize the struct BF_Block.
	CALL_OR_DIE(BF_CreateFile(fileName)); // Create a file which consists of blocks. 

    int fileDescriptor; // The file descriptor of the file we are going to open.
	CALL_OR_DIE(BF_OpenFile(fileName, &fileDescriptor)); // Open the file

	CALL_OR_DIE(BF_AllocateBlock(fileDescriptor, block));

    char* data = BF_Block_GetData(block);
    size_t sizeOfFilename = sizeof(fileName);

    // Create struct HT_info
	HT_info* info = createHT_info(fileName, fileDescriptor, buckets, sizeOfFilename); 
	// Store the info
    memcpy(data, info, sizeof(*info));

    // Create struct HT_block_info
    HT_block_info* blockInfo = createHT_block_info(0, 1);

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

    return 0;
}

HT_info* HT_OpenFile(char *fileName){
    BF_Block* block;                        // block
    BF_Block_Init(&block);                  // Initiallize the BF_Block.

    HT_info* info = malloc(sizeof(*info));  // Allocate the struct with the metadata
    int fileDescriptor;                     // FileDescriptor

    CALL_OR_DIE(BF_OpenFile(fileName, &fileDescriptor));    // Open the file
    CALL_OR_DIE(BF_GetBlock(fileDescriptor, 0, block));     // Get the first block
    char* data = BF_Block_GetData(block);                   // Get the data of the first block

    // Copy the HT_info of file
    memcpy(info, data, sizeof(*info));

    // Check if the file is a HT file
    if(isHashTable(info))
        return NULL;

    BF_Block_Destroy(&block);

    return info;
}


int HT_CloseFile(HT_info* HT_info){
	BF_Block* block;
    BF_Block_Init(&block); // Initiallize the BF_Block.

    // Unpin first and second blocks
	CALL_OR_DIE(BF_GetBlock(HT_info->fileDesc, 0, block));
	CALL_OR_DIE(BF_UnpinBlock(block));

    CALL_OR_DIE(BF_GetBlock(HT_info->fileDesc, 1, block));
	CALL_OR_DIE(BF_UnpinBlock(block));

    // Close the file
    CALL_OR_DIE(BF_CloseFile(HT_info->fileDesc));

    // memory managment
    BF_Block_Destroy(&block);
    free(HT_info->fileName);
    free(HT_info);
    return 0;
}

int HT_InsertEntry(HT_info* ht_info, Record record){
    BF_Block *block;
	BF_Block_Init(&block);

    // Get the block where we have stored the buckets
    CALL_OR_DIE(BF_GetBlock(ht_info->fileDesc, 1, block));

    // Get the data of this block
	char *data = BF_Block_GetData(block);

    // Malloc an array to hold the buckets 
    int *arrayOfBuckets = malloc(ht_info->numOfBuckets * sizeof(int));
    // Copy data of buckets into the array
    memcpy(arrayOfBuckets, data, ht_info->numOfBuckets * sizeof(int));

    // hash the id because we need to store the hashed_id into the buckets.
    int hashedId = record.id % ht_info->numOfBuckets;

    // Check if a specific bucket is unitiallized.
    // If it is allocate a new block and let bucket point to that block.
    // If it is NOT just return and do nothing.
    checkBucket(ht_info, arrayOfBuckets, hashedId);

    // Find the last block inside the bucket that is empty.
    int currentBlock = arrayOfBuckets[hashedId];
    int nextBlock = currentBlock;
    while(nextBlock != UNITIALLIZED){
        // Get the block of the nextBlockId
        CALL_OR_DIE(BF_GetBlock(ht_info->fileDesc, nextBlock, block));
        // Get the data of this block
        char* data = BF_Block_GetData(block);
        // Update currentBlock
        currentBlock = nextBlock;
        // Update nextBlock
        data += BYTES_UNTIL_NEXT;
        memcpy(&nextBlock, data, sizeof(int));
        // Unpin the block
        CALL_OR_DIE(BF_UnpinBlock(block));
    }

    // We found the last block inside the bucket 
    // Get its data
    CALL_OR_DIE(BF_GetBlock(ht_info->fileDesc, currentBlock, block));
    data = BF_Block_GetData(block);
    // Go to ht_block_info.numOfRecords
    data += BYTES_UNTIL_NUM_OF_RECORDS;
    // Get NumberOfRecords of currentBlock
    ulint numOfRecords; 
    memcpy(&numOfRecords, data, sizeof(ulint));
    
    // if the numOfRecords == MAX_RECORDS_PER_BLOCK allocate a new block and update 
    // the currentBlock so its next block will be the block we just allocated
    if(numOfRecords == MAX_RECORDS_PER_BLOCK){ 
        int newBlock = createBlock(ht_info);  // create a new block
        data -= sizeof(int);                  // Go to the next field of the ht_block_info struct of the currentBlock  
        memcpy(data, &newBlock, sizeof(int)); // Pass the updated next into the next field of the ht_block_info struct of the currentBlock  
        // Write changes to block
        BF_Block_SetDirty(block);   
        CALL_OR_DIE(BF_UnpinBlock(block));
        // Get the new block and its data
        CALL_OR_DIE(BF_GetBlock(ht_info->fileDesc, newBlock, block));
        char* data = BF_Block_GetData(block); 
        memcpy(data, &record, sizeof(record)); // Insert the record into the new block
        data += BYTES_UNTIL_NUM_OF_RECORDS; // Go to the HT_block_info.numOfRecords location 
        // Update the numOfRecords of ht_block_info of the newBlock
        // and write it back to the data.
        ulint newBlock_numOfRecords = 1;
        memcpy(data, &newBlock_numOfRecords, sizeof(ulint));

        // Write changes to block
        BF_Block_SetDirty(block);
        CALL_OR_DIE(BF_UnpinBlock(block));

        // Memory Managment
        BF_Block_Destroy(&block);
        free(arrayOfBuckets);
        return newBlock;
    }
    // If we have enough space for one more block:
    // Go back to the start of the data of the currentBlock
    data -= BYTES_UNTIL_NUM_OF_RECORDS;

    // Calculate the position that the record should be inserted.
    data += numOfRecords * sizeof(record);    
    // Insert the record   
    memcpy(data, &record, sizeof(record)); 
    // Update numOfRecords
    numOfRecords++; 
    // Go to the HT_block_info.numOfRecords location
    data += BYTES_UNTIL_NUM_OF_RECORDS - ((numOfRecords - 1) * sizeof(Record));
    memcpy(data, &numOfRecords, sizeof(ulint));

    // Write changes to block
    BF_Block_SetDirty(block);
    CALL_OR_DIE(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    free(arrayOfBuckets);
    return currentBlock;
}

int HT_GetAllEntries(HT_info* ht_info, int value){
    BF_Block *block;
	BF_Block_Init(&block);

    // Get the block where we have store the buckets
    CALL_OR_DIE(BF_GetBlock(ht_info->fileDesc, 1, block));

    // Get the data of this block
	char *data = BF_Block_GetData(block);

    // Malloc an array to hold the buckets 
    int *arrayOfBuckets = malloc(ht_info->numOfBuckets * sizeof(int));
    // Copy data of buckets into the array
    memcpy(arrayOfBuckets, data, ht_info->numOfBuckets * sizeof(int));

    // Find the hased id of the records.
    // The records we want are going to have this specific hashedId
    int hashedId = value % ht_info->numOfBuckets;

    int currentBlock = arrayOfBuckets[hashedId];
    int blocksRead = 1;
    // Iterate into all the blocks with this hashedId
    while(currentBlock != UNITIALLIZED){

        // Get the block with id = currentBlock
        CALL_OR_DIE(BF_GetBlock(ht_info->fileDesc, currentBlock, block));
        // Get the data of this block
        char* data = BF_Block_GetData(block);
        // Get the numOfRecords of the block
        data += BYTES_UNTIL_NUM_OF_RECORDS;
        ulint numOfRecords;
        memcpy(&numOfRecords, data, sizeof(ulint)); 
        Record record;
        // Go back to the start of the currentBlock data
        data -= BYTES_UNTIL_NUM_OF_RECORDS;
        for(int i = 0; i < numOfRecords; i++){
            // Get the record
            memcpy(&record, data, sizeof(record));
            // Check if the record.id has the correct value
            if (record.id == value){
                printRecord(record);
                
                // Memory Managment
                free(arrayOfBuckets);
                CALL_OR_DIE(BF_UnpinBlock(block));
				BF_Block_Destroy(&block);
                return blocksRead;
            }
            // Go to the next Record
			data += sizeof(Record);
        }
        // There isnt any record with id == value inside this block.
        // Go to the next block.
        // Reset data.
        data = BF_Block_GetData(block);
        data += BYTES_UNTIL_NEXT;
        memcpy(&currentBlock, data, sizeof(int));
        CALL_OR_DIE(BF_UnpinBlock(block));
        blocksRead++;
    }

    // Memory Managment
    free(arrayOfBuckets);
    BF_Block_Destroy(&block);

    // We didnt found any record with Id == value.
    return -1;
}

int HashStatistics(char *fileName) {
    // Get the HT_info of the file.
    HT_info* info = HT_OpenFile(fileName);

    // Just for beauty
    printf("\n       Statistics of the HT_file\n");
    printf("---------------------------------------\n");

    //Number of blocks inside the file
    int* numOfBlocks = malloc(sizeof(int)); 
    CALL_OR_DIE(BF_GetBlockCounter(info->fileDesc, numOfBlocks));
    printf("Number of blocks inside the HT_file:%d\n", *numOfBlocks);

    // Avg blocks per bucket
    int averageBlocksPerBucket = *numOfBlocks / info->numOfBuckets;
    printf("Average number of blocks inside each bucket:%d\n\n", averageBlocksPerBucket);

    BF_Block *block;
	BF_Block_Init(&block);

    // Get the block where we have store the buckets
    CALL_OR_DIE(BF_GetBlock(info->fileDesc, 1, block));

    // Get the data of this block
	char *data = BF_Block_GetData(block);

    // Malloc an array to hold the buckets 
    int *arrayOfBuckets = malloc(info->numOfBuckets * sizeof(int));
    // Copy data of buckets into the array
    memcpy(arrayOfBuckets, data, info->numOfBuckets * sizeof(int));

    // Iterate each bucket and hold the number of its records.    
    int minRecords = INT_MAX;       // Minimum number of records in all the buckets.
    int maxRecords = 0;             // Maximum number of records in all the buckets.
    int minRecordsBucket = 0;       // Bucket's ID that holds the minimum number of records.
    int maxRecordsBucket = 0;       // Bucket's ID that holds the maximum number of records.
    int totalRecords = 0;           // Total records in all the buckets.
    int numOfBucketsOverflowed = 0; // Number of buckets that have been overflowed.
    for(int i = 0; i < info->numOfBuckets; i++) {
        printf("BucketID:%d\n", i);
        int numOfRecords = 0;
        int currentBlock = arrayOfBuckets[i];
        while(currentBlock != UNITIALLIZED){
            int currentBlockRecords;
            // Get the block where we have store the buckets
            CALL_OR_DIE(BF_GetBlock(info->fileDesc, currentBlock, block));
            // Get the data of this block
	        char *data = BF_Block_GetData(block);
            // Go to the Ht_block_info.numOfRecords 
            data += BYTES_UNTIL_NUM_OF_RECORDS;
            memcpy(&currentBlockRecords, data, sizeof(int));
            numOfRecords += currentBlockRecords;
            // Go to Ht_block_info.next 
            data -= sizeof(int); 
            memcpy(&currentBlock, data, sizeof(int));
            BF_UnpinBlock(block);
        }
        //Update totalRecords counter 
        totalRecords += numOfRecords;
        // If the number of records inside the bucket is greater than the maximun 
        // number of records inside one block it means that the bucket has been overflowed.
        if(numOfRecords > MAX_RECORDS_PER_BLOCK){
            printf("Overflowed: YES\n");
            numOfBucketsOverflowed++; // Update counter.

            // Number of OverflowedBlocks of each bucket = bucketInfo.numOfRecords / MAX_RECORDS_PER_BLOCK
            int maxRecordsPerBlock = MAX_RECORDS_PER_BLOCK;
            printf("Number of Overflowed Blocks:%d\n\n", (numOfRecords / maxRecordsPerBlock));
        }

        if(numOfRecords > maxRecords){
            maxRecords = numOfRecords;
            maxRecordsBucket = i;
        }
        if(numOfRecords < minRecords){
            minRecords = numOfRecords;
            minRecordsBucket = i;
        }
    }

    printf("Bucket's id with LESS Records:%d and has %d Records\n", minRecordsBucket, minRecords);
    printf("Bucket's id with MORE Records:%d and has %d Records\n", maxRecordsBucket, maxRecords);
    printf("Average records per bucket:%ld\n", totalRecords/info->numOfBuckets);
    printf("Number of buckets that have been Overflowed:%d\n", numOfBucketsOverflowed);

    free(numOfBlocks);
    free(arrayOfBuckets);
    free(info);
    BF_Block_Destroy(&block);

}




