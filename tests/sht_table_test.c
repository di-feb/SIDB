#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/accutest.h" // A simple library for unit testing
#include "../include/bf.h"
#include "../include/ht_table.h"
#include "../include/sht_table.h"
#include "../include/record.h"

#define RECORDS_NUM 100 
#define FILE_NAME  "data.db"
#define INDEX_NAME "index.db"

void test_SHT_CreateSecondaryIndex(void) {
	BF_Init(LRU);
	SHT_CreateSecondaryIndex(INDEX_NAME,10, FILE_NAME);
    // Check if the file is created.
    TEST_CHECK((access(INDEX_NAME, F_OK) == 0));
    // Open the file to get the struct with the metadata.
    SHT_info* info = SHT_OpenSecondaryIndex(INDEX_NAME);
    // Check if the number of buckets is actually 10.
    TEST_CHECK(info->numOfBuckets == 10);
    // Check if the file we created is a HashTable file.
    TEST_CHECK(info->isSecondaryHashTable);

    // We close the file. 
    // The validity of the function SHT_CloseFile is tested 
    // by running the program with valgrind and not having leaks.
	SHT_CloseSecondaryIndex(info);
    BF_Close();
}

void test_SHT_OpenSecondaryIndex(void) {
	BF_Init(LRU);
    // Open the file to get the struct with the metadata.
    SHT_info* info = SHT_OpenSecondaryIndex(INDEX_NAME);
    // Check if the number of buckets is actually 10.
    TEST_CHECK(info->numOfBuckets == 10);
    TEST_CHECK(!strcmp(info->fileName, INDEX_NAME));

    SHT_CloseSecondaryIndex(info);
    BF_Close();
}

void test_SHT_Insert_SHT_Get(void) {
	BF_Init(LRU);
    // Create a HT_file
    HT_CreateFile(FILE_NAME,10);

    // Open the file to get the struct with the metadata.
    SHT_info* index_info = SHT_OpenSecondaryIndex(INDEX_NAME);
    HT_info* info = HT_OpenFile(FILE_NAME);

    // Insert a record with name Feb
    // Get all entries should find it.
    Record record;
    char* name = malloc(sizeof(15));
    strcpy(name, "Feb");
    record = randomRecord_WithSpecificName(name);
    int blockId = HT_InsertEntry(info, record);
    // Check that the insertion was valid.
    TEST_CHECK(!SHT_SecondaryInsertEntry(index_info, record, blockId));
    // We only have one bucket with one block
    TEST_CHECK(SHT_SecondaryGetAllEntries(info, index_info, name) == 1);
    //if i ask for a name that doesnt exist GetAllEntries shouldnt find it
    TEST_CHECK(SHT_SecondaryGetAllEntries(info, index_info, "Alexx") == -1);

    // // Must been created a new block.
    // int* numberOfBlocks = malloc(sizeof(int)); 
    // BF_GetBlockCounter(info->fileDesc, numberOfBlocks);
    // TEST_CHECK(*numberOfBlocks == 3);


    // // Insert records with Id that is going 
    // // to be hashed inside the same bucket as all the other records
    // // We must have overflowed blocks
    // // Number of buckets is 10.
    // // The new block must be block with id == 3.
    // // We are going to check if the name of the 
    // // last record is the same with the record inside
    // // the the block with id == 3.
    // for(int i = 0; i < 70; i = i + 10){
    //     record = randomRecord_WithSpecificID(i);
    //     HT_InsertEntry(info, record);
    //     if(i == 50) // The first record is going to be inside an overflowed block
    //         strcpy(name, record.name);        
    // }
 
    // BF_GetBlockCounter(info->fileDesc, numberOfBlocks);
    // // We must have overflowed blocks
    // // A new block must been created.
    // TEST_CHECK(*numberOfBlocks == 4);

    // BF_Block *block;
	// BF_Block_Init(&block);
    
    // // Get the 3 block
    // BF_GetBlock(info->fileDesc, 3, block);
    // // Get the data of this block
	// char *data = BF_Block_GetData(block);
    // memcpy(&record, data, sizeof(record));
    // TEST_CHECK(!strcmp(name, record.name));

    // // We have inserted 8 blocks the last block is the block
    // // with id == 60. GetAllEntries should find it after seaching
    // // inside 2 blocks.Lets check if thats true.
    // blocksRead = HT_GetAllEntries(info, 60);
    // printf("%d\n", blocksRead);
    // TEST_CHECK(blocksRead == 2);

    // // We will insert 17 more entries
    // // And check again GetAllEntries
    // for(int i = 70; i < 240; i = i + 10){
    //     record = randomRecord_WithSpecificID(i);
    //     HT_InsertEntry(info, record);      
    // }
    // blocksRead = HT_GetAllEntries(info, 230);
    // printf("%d\n", blocksRead);
    // TEST_CHECK(blocksRead == 5);


    // free(numberOfBlocks);
    // BF_UnpinBlock(block);
	// HT_CloseFile(info);
    // BF_Close();
}



// Λίστα με όλα τα tests προς εκτέλεση
TEST_LIST = {
	{ "SHT_CreateSecondaryIndex", test_SHT_CreateSecondaryIndex },
	{ "SHT_OpenSecondaryIndex", test_SHT_OpenSecondaryIndex },
	{ "SHT_InsertEntry\n     SHT_GetAllEntries", test_SHT_Insert_SHT_Get},
	{ NULL, NULL } // τερματίζουμε τη λίστα με NULL
};