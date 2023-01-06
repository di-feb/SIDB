#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "../include/bf.h"
#include "../include/ht_table.h"
#include "../include/sht_table.h"
#include "../include/record.h"

#define UNITIALLIZED -1
#define MAX_RECORDS_PER_BLOCK (BF_BLOCK_SIZE - sizeof(HT_block_info)) / (sizeof(Record))
#define MAX_SHT_RECORDS_PER_BLOCK (BF_BLOCK_SIZE - sizeof(SHT_block_info)) / (sizeof(SHT_Record))
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
// Initiallizes all the fields exept fileName so we are
// able to free the memory.
SHT_info* createSHT_info(int fileDescriptor, int numOfBuckets){
    // Allocate the struct SHT_info
    SHT_info* info = malloc(sizeof(*info)); 
    // Initiallize it
    info->isSecondaryHashTable = true;
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
    blockInfo->numOfSHTRecords = 0;

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

    // Unpin the block 
    CALL_OR_DIE(BF_UnpinBlock(block));

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
    free(info);
    free(block_info);
}

// Checks if a specific file is a secondaryhashTable file
bool isSecondaryHashTable(SHT_info* info){
    if(info->isSecondaryHashTable)
        return false;
    return true;
}

// Borrowed by the Data Bases class of Mister Chatzikokolakis!
int hash_string(void* value) {
	// djb2 hash function, simple, fast, and at most cases effective.
    uint hash = 5381;
    for (char* s = value; *s != '\0'; s++)
		hash = (hash << 5) + hash + *s;	// hash = (hash * 33) + *s. 
    return hash;                        // foo << 5 is a faster version of foo * 32.
}

// Prints the record inside HT_Block with id:blockId and record.name = name.
// If there isnt a block with this recordId inside return -1.
// If there is this block, returns 0.
// Also incease the number of blocks that have been read until we found all the records with record.name == name
int printHT_blockId(HT_info* ht_info, int blockId, char* name){
    BF_Block *block;
	BF_Block_Init(&block);


    // Get the block with id = block
    CALL_OR_DIE(BF_GetBlock(ht_info->fileDesc, blockId, block));
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
        // Check if the record.id has the correct value, and the names are the same.
        if (!strcmp(record.name, name)){
            printRecord(record);
            // Memory Managment
            CALL_OR_DIE(BF_UnpinBlock(block));
            BF_Block_Destroy(&block);
            return 0;
        }
        // Go to the next Record
        data += sizeof(Record);
    }

    // Memory Managment
    CALL_OR_DIE(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);

    // We didnt found any record with Id == recordId.
    return -1;
}

// Mallocs an integer with value == value
int* create_int(int value) {
	int* p = malloc(sizeof(int));
	*p = value;
	return p;
}

int SHT_CreateSecondaryIndex(char *sfileName, int buckets, char* fileName){
    BF_Block* block;

    BF_Block_Init(&block); // Initiallize the struct BF_Block.
	CALL_OR_DIE(BF_CreateFile(sfileName)); // Create a file which consists of blocks. 

    int fileDescriptor; // The file descriptor of the file we are going to open.
	CALL_OR_DIE(BF_OpenFile(sfileName, &fileDescriptor)); // Open the file

    CALL_OR_DIE(BF_AllocateBlock(fileDescriptor, block));

    char* data = BF_Block_GetData(block);

    // Create struct SHT_info
	SHT_info* info = createSHT_info(fileDescriptor, buckets); 
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

    // Unpin the block
    CALL_OR_DIE(BF_UnpinBlock(block));

    // Close the file
    CALL_OR_DIE(BF_CloseFile(fileDescriptor));

    // Memory managment
    BF_Block_Destroy(&block);
    infoDestroy(info, blockInfo);
}

SHT_info* SHT_OpenSecondaryIndex(char *indexName){
    BF_Block* block;                        // block
    BF_Block_Init(&block);                  // Initiallize the BF_Block.

    SHT_info* info = malloc(sizeof(*info)); // Allocate the struct with the metadata
    int fileDescriptor;                     // FileDescriptor

    CALL_OR_DIE(BF_OpenFile(indexName, &fileDescriptor));   // Open the file
    CALL_OR_DIE(BF_GetBlock(fileDescriptor, 0, block));     // Get the first block
    char* data = BF_Block_GetData(block);                   // Get the data of the first block

    // Copy the SHT_info of file
    memcpy(info, data, sizeof(*info));
    // Update fileName
    // We allocate it inside openFile so we can free the pointer.
    info->fileName = malloc(strlen(indexName) + 1);
    strcpy(info->fileName, indexName);

    // Copy the SHT_info of file
    memcpy(data, info, sizeof(*info));

    // Check if the file is a SHT file
    if(isSecondaryHashTable(info))
        return NULL;
    
    // Unpin the block
    CALL_OR_DIE(BF_UnpinBlock(block));

    BF_Block_Destroy(&block);

    return info;
}


int SHT_CloseSecondaryIndex(SHT_info* SHT_info ){
    // Close the file
    CALL_OR_DIE(BF_CloseFile(SHT_info->fileDesc));

    free(SHT_info->fileName);
    free(SHT_info);
    return 0;
}

int SHT_SecondaryInsertEntry(SHT_info* sht_info, Record record, int block_id){
    BF_Block *block;
	BF_Block_Init(&block);
    // Get the block where we have stored the buckets
    CALL_OR_DIE(BF_GetBlock(sht_info->fileDesc, 1, block));

    // Get the data of this block
	char *data = BF_Block_GetData(block);

    // Malloc an array to hold the buckets 
    int *arrayOfBuckets = malloc(sht_info->numOfBuckets * sizeof(int));
    // Copy data of buckets into the array
    memcpy(arrayOfBuckets, data, sht_info->numOfBuckets * sizeof(int));

    // Unpin the block with the buckets we dont need it anymore
    CALL_OR_DIE(BF_UnpinBlock(block));

    // Hash the Name.
    int hashedName = hash_string(record.name);
    int hashedIndex = hashedName % sht_info->numOfBuckets;

    // Check if a specific bucket is unitiallized.
    // If it is allocate a new block and let bucket point to that block.
    // If it is NOT just return and do nothing.
    checkBucket(sht_info, arrayOfBuckets, hashedIndex);

    // We need to find the last block inside that bucket to insert the SHT_Record
    int currentBlock = arrayOfBuckets[hashedIndex];
    int nextBlock = currentBlock;
    while(nextBlock != UNITIALLIZED){
        // Get the block of the nextBlockId
        CALL_OR_DIE(BF_GetBlock(sht_info->fileDesc, nextBlock, block));
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
    CALL_OR_DIE(BF_GetBlock(sht_info->fileDesc, currentBlock, block));
    data = BF_Block_GetData(block);
    // Go to sht_block_info.numOfSHTRecords
    data += BYTES_UNTIL_NUM_OF_RECORDS;
    // Get NumberOfRecords of currentBlock
    ulint numOfSHTRecords; 
    memcpy(&numOfSHTRecords, data, sizeof(ulint));
    
    // if the numOfSHTRecords == MAX_SHT_RECORDS_PER_BLOCK allocate a new block and update 
    // the currentBlock so its next block will be the block we just allocated
    if(numOfSHTRecords == MAX_SHT_RECORDS_PER_BLOCK){ 
        int newBlock = createBlock(sht_info);  // create a new block
        data -= sizeof(int);                   // Go to the next field of the sht_block_info struct of the currentBlock  
        memcpy(data, &newBlock, sizeof(int));  // Pass the updated next into the next field of the ht_block_info struct of the currentBlock  
        // Write changes to block
        BF_Block_SetDirty(block);  
        // Unpin the currentBlock 
        CALL_OR_DIE(BF_UnpinBlock(block));
        // Get the new block and its data
        CALL_OR_DIE(BF_GetBlock(sht_info->fileDesc, newBlock, block));
        char* data = BF_Block_GetData(block); 
        // Make a new SHT_Record and initiallize it.
        SHT_Record sht_record; 
        strcpy(sht_record.name, record.name);
        sht_record.blockId = block_id;
        memcpy(data, &sht_record, sizeof(sht_record)); // Insert the sht_record into the new block
        data += BYTES_UNTIL_NUM_OF_RECORDS; // Go to the SHT_block_info.numOfSHTRecords location 
        // Update the numOfSHTRecords of sht_block_info of the newBlock
        // and write it back to the data.
        ulint newBlock_numOfSHTRecords = 1;
        memcpy(data, &newBlock_numOfSHTRecords, sizeof(ulint));

        // Write changes to block
        BF_Block_SetDirty(block);
        CALL_OR_DIE(BF_UnpinBlock(block));

        // Memory Managment
        BF_Block_Destroy(&block);
        free(arrayOfBuckets);
        return 0;
    }
    // If we have enough space for one more block:
    // Go back to the start of the data of the currentBlock
    data -= BYTES_UNTIL_NUM_OF_RECORDS;

    // Make a new SHT_Record and initiallize it.
    SHT_Record sht_record; 
    strcpy(sht_record.name, record.name);
    sht_record.blockId = block_id;
    // Calculate the position that the record should be inserted.
    data += numOfSHTRecords * sizeof(sht_record);    
    // Insert the record   
    memcpy(data, &sht_record, sizeof(sht_record)); 
    // Update numOfSHTRecords
    numOfSHTRecords++; 
    // Go to the SHT_block_info.numOfSHTRecords location
    data += BYTES_UNTIL_NUM_OF_RECORDS - ((numOfSHTRecords - 1) * sizeof(SHT_Record));
    memcpy(data, &numOfSHTRecords, sizeof(ulint));

    // Write changes to block
    BF_Block_SetDirty(block);
    CALL_OR_DIE(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    free(arrayOfBuckets);
    return 0;
}

int SHT_SecondaryGetAllEntries(HT_info* ht_info, SHT_info* sht_info, char* name){
    BF_Block *block;
	BF_Block_Init(&block);

    int recordFound = 0; // A boolean to help us return the correct exit code.

    // Get the block where we have stored the buckets
    CALL_OR_DIE(BF_GetBlock(sht_info->fileDesc, 1, block));

    // Get the data of this block
	char *data = BF_Block_GetData(block);

    // Malloc an array to hold the buckets 
    int *arrayOfBuckets = malloc(sht_info->numOfBuckets * sizeof(int));
    // Copy data of buckets into the array
    memcpy(arrayOfBuckets, data, sht_info->numOfBuckets * sizeof(int));

    // Unpin the block with the buckets we dont need it anymore
    CALL_OR_DIE(BF_UnpinBlock(block));

    // Hash the Name.
    int hashedName = hash_string(name);
    int hashedIndex = hashedName % sht_info->numOfBuckets;

    int currentBlock = arrayOfBuckets[hashedIndex];
    int* blocksRead = create_int(1);
    // Iterate into all the blocks with this hashedIndex
    while(currentBlock != UNITIALLIZED){

        // Get the block with id = currentBlock
        CALL_OR_DIE(BF_GetBlock(sht_info->fileDesc, currentBlock, block));
        // Get the data of this block
        char* data = BF_Block_GetData(block);
        // Get the numOfSHTRecords of the block
        data += BYTES_UNTIL_NUM_OF_RECORDS;
        ulint numOfSHTRecords;
        memcpy(&numOfSHTRecords, data, sizeof(ulint)); 
        SHT_Record sht_record; // temporary sht_record
        // Go back to the start of the currentBlock data
        data -= BYTES_UNTIL_NUM_OF_RECORDS;
        for(int i = 0; i < numOfSHTRecords; i++){
            // Get the sht_record
            memcpy(&sht_record, data, sizeof(sht_record));
            // Check if a identical record.name exist inside this block
            if (!strcmp(sht_record.name, name)){
                // If there is one go and find the block with this name inside the primary hash table.
                // Then print this record.
                int htBlockNumber = printHT_blockId(ht_info, sht_record.blockId, name);
                if(htBlockNumber == -1){// Error Handling
                    perror("There is not a block inside HT with this record\n"); 
                    free(arrayOfBuckets);
                    free(blocksRead);
                    CALL_OR_DIE(BF_UnpinBlock(block));
                    BF_Block_Destroy(&block);
                    exit(1);
                }
                recordFound = 1;
            }
            // Its possible that there are multiple Records with the same name.
            // We want to print them all.
            // Go to the next SHT_Record
            data += sizeof(SHT_Record);
        }
        // There isnt any sht_record with sht_record.name == name inside this block.
        // Go to the next block.
        // Reset data.
        data = BF_Block_GetData(block);
        data += BYTES_UNTIL_NEXT;
        memcpy(&currentBlock, data, sizeof(int));
        CALL_OR_DIE(BF_UnpinBlock(block));
        (*blocksRead)++;
    }
    // Memory Managment
    free(arrayOfBuckets);
    BF_Block_Destroy(&block);

    int totalBlocksRead = *blocksRead;
    free(blocksRead);
    if(recordFound)
        return totalBlocksRead;
    // We didnt found any record with record.name = name   
    return -1;
}




