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
    // Open the file to get the struct with the metadata.
    SHT_info* index_info = SHT_OpenSecondaryIndex(INDEX_NAME);

    // Create a HT_file
    HT_CreateFile(FILE_NAME,10);

    HT_info* info = HT_OpenFile(FILE_NAME);

    // Insert a record with name Feb
    // Get all entries should find it.
    Record record;
    char* name = malloc(sizeof(15));
    strcpy(name, "Feb");
    record = randomRecord_WithSpecificName(name);
    int blockId = HT_InsertEntry(info, record);
    // Check that the insertion was valid.
    TEST_CHECK(SHT_SecondaryInsertEntry(index_info, record, blockId) == 0);
    // We only have one bucket with one block
    TEST_CHECK(SHT_SecondaryGetAllEntries(info, index_info, name) == 1);
    //if i ask for a name that doesnt exist GetAllEntries shouldnt find it
    TEST_CHECK(SHT_SecondaryGetAllEntries(info, index_info, "Alexx") == -1);

    // Must been created a new block.
    int* numberOfBlocks = malloc(sizeof(int));  
    BF_GetBlockCounter(index_info->fileDesc, numberOfBlocks);
    TEST_CHECK(*numberOfBlocks == 3);


    // Insert records with name that is going 
    // to be hashed inside the same bucket as all the other records
    // We must have overflowed blocks
    // Number of buckets is 10.
    // The new block must be block with name = "a".
    int maxSHT_Entries = 25; // Max entries that a shtBlock can hold are 24
    for(int i = 0; i < maxSHT_Entries; i++){
        record = randomRecord_WithSpecificName("a");
        int block_id = HT_InsertEntry(info, record);
        SHT_SecondaryInsertEntry(index_info, record, block_id);
    }
 
    BF_GetBlockCounter(index_info->fileDesc, numberOfBlocks);
    // We must have overflowed blocks
    // A new block must been created.
    printf("%d\n", *numberOfBlocks);
    // We have already push a a record with name "Feb" inside the SHT that didnt
    // hashed into the same bucket as the name "a".
    TEST_CHECK(*numberOfBlocks == 5); 

    BF_Block *block;
	BF_Block_Init(&block);

    // We are going to check if the name of the 
    // first record is the same with the record inside
    // the the block with id == 2.
    // Get the 2 block
    BF_GetBlock(index_info->fileDesc, 2, block);
    // Get the data of this block
    SHT_Record shtRecord;
	char *data = BF_Block_GetData(block);
    memcpy(&shtRecord, data, sizeof(shtRecord));
    TEST_CHECK(!strcmp("Feb", shtRecord.name));  

    free(numberOfBlocks);
    BF_UnpinBlock(block);
    BF_Block_Destroy(&block);
	HT_CloseFile(info);
    SHT_CloseSecondaryIndex(index_info);
    BF_Close();
}



// Λίστα με όλα τα tests προς εκτέλεση
TEST_LIST = {
	{ "SHT_CreateSecondaryIndex", test_SHT_CreateSecondaryIndex },
	{ "SHT_OpenSecondaryIndex", test_SHT_OpenSecondaryIndex },
	{ "SHT_InsertEntry\n     SHT_GetAllEntries", test_SHT_Insert_SHT_Get},
	{ NULL, NULL } // τερματίζουμε τη λίστα με NULL
};