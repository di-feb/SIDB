#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/acutest.h" // A simple library for unit testing
#include "../include/bf.h"
#include "../include/ht_table.h"
#include "../include/record.h"

#define RECORDS_NUM 100 
#define FILE_NAME "data.db"
#define INDEX_FILE_NAME "index.db"

void test_HT_CreateFile(void) {
	BF_Init(LRU);
	HT_CreateFile(FILE_NAME,10);
    // Check if the file is created.
    TEST_CHECK((access(FILE_NAME, F_OK) == 0));
    // Open the file to get the struct with the metadata.
    HT_info* info = HT_OpenFile(FILE_NAME);
    // Check if the number of buckets is actually 10.
    TEST_CHECK(info->numOfBuckets == 10);
    // Check if the file we created is a HashTable file.
    TEST_CHECK(info->isHashTable);

    // We close the file. 
    // The validity of the function HT_CloseFile is tested 
    // by running the program with valgrind and not having leaks.
	HT_CloseFile(info);
    BF_Close();
}

void test_HT_OpenFile(void) {
	BF_Init(LRU);
    // Open the file to get the struct with the metadata.
    HT_info* info = HT_OpenFile(FILE_NAME);
    // Check if the number of buckets is actually 10.
    TEST_CHECK(info->numOfBuckets == 10);
    TEST_CHECK(!strcmp(info->fileName, FILE_NAME));

	HT_CloseFile(info);
    BF_Close();
}

void test_HT_Insert_HT_Get(void) {
	BF_Init(LRU);
    // Open the file to get the struct with the metadata.
    HT_info* info = HT_OpenFile(FILE_NAME);

    // Insert a record with Id = 0
    // Get all entries should find it.
    Record record;
    record = randomRecord_WithSpecificID(80);
    HT_InsertEntry(info, record);
    int blocksRead = HT_GetAllEntries(info, 80);
    TEST_CHECK(blocksRead == 1);
    //if i ask for a id that doesnt exist GetAllEntries shouldnt find it
    TEST_CHECK(HT_GetAllEntries(info, 0) == -1);

    // Must been created a new block.
    int* numberOfBlocks = malloc(sizeof(int)); 
    BF_GetBlockCounter(info->fileDesc, numberOfBlocks);
    TEST_CHECK(*numberOfBlocks == 3);


    // Insert records with Id that is going 
    // to be hashed inside the same bucket as all the other records
    // We must have overflowed blocks
    // Number of buckets is 10.
    // The new block must be block with id == 3.
    // We are going to check if the name of the 
    // last record is the same with the record inside
    // the the block with id == 3.
    char name[15];
    for(int i = 0; i < 70; i = i + 10){
        record = randomRecord_WithSpecificID(i);
        HT_InsertEntry(info, record);
        if(i == 50) // The first record is going to be inside an overflowed block
            strcpy(name, record.name);        
    }
 
    BF_GetBlockCounter(info->fileDesc, numberOfBlocks);
    // We must have overflowed blocks
    // A new block must been created.
    TEST_CHECK(*numberOfBlocks == 4);

    BF_Block *block;
	BF_Block_Init(&block);
    
    // Get the 3 block
    BF_GetBlock(info->fileDesc, 3, block);
    // Get the data of this block
	char *data = BF_Block_GetData(block);
    memcpy(&record, data, sizeof(record));
    TEST_CHECK(!strcmp(name, record.name));

    // We have inserted 8 blocks the last block is the block
    // with id == 60. GetAllEntries should find it after seaching
    // inside 2 blocks.Lets check if thats true.
    blocksRead = HT_GetAllEntries(info, 60);
    printf("%d\n", blocksRead);
    TEST_CHECK(blocksRead == 2);

    // We will insert 17 more entries
    // And check again GetAllEntries
    for(int i = 70; i < 240; i = i + 10){
        record = randomRecord_WithSpecificID(i);
        HT_InsertEntry(info, record);      
    }
    blocksRead = HT_GetAllEntries(info, 230);
    printf("%d\n", blocksRead);
    TEST_CHECK(blocksRead == 5);


    free(numberOfBlocks);
    BF_UnpinBlock(block);
    BF_Block_Destroy(&block);
	HT_CloseFile(info);
    BF_Close();
}



// List of all the tests
TEST_LIST = {
	{ "HT_CreateFile", test_HT_CreateFile },
	{ "HT_OpenFile", test_HT_OpenFile },
	{ "HT_InsertEntry\n     HT_GetAllEntries", test_HT_Insert_HT_Get},
	{ NULL, NULL } // end the test list with a NULL
};