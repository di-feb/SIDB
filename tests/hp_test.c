#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/accutest.h" // A simple library for unit testing
#include "../include/bf.h"
#include "../include/hp_file.h"
#include "../include/record.h"

#define RECORDS_NUM 100 
#define FILE_NAME "data.db"

void test_HP_CreateFile(void) {
    BF_Init(LRU);
    HP_CreateFile(FILE_NAME);
    // Check if file is created
    TEST_CHECK((access(FILE_NAME, F_OK) == 0));
    // Open the file to get the struct with the metadata
    HP_info* info = HP_OpenFile(FILE_NAME);
    // Check if the file we created is a Heap File
    TEST_CHECK(info->isHeap);
}

void test_HP_OpenFile(void) {
    BF_Init(LRU);
    // Open the file to get the struct with the metadata
    HP_info* info = HP_OpenFile(FILE_NAME);
    // Check if the file has opened
    TEST_CHECK(!strcmp(info->fileName, FILE_NAME));

	HP_CloseFile(info);
    BF_Close();
}

void test_HP_Insert_HP_Get(void) {
    BF_Init(LRU);
    // Open the file to get the struct with the metadata
    HP_info* info = HP_OpenFile(FILE_NAME);

    // Insert one record with id = 0
    // Get all entries should find it
    Record record;
    record = randomRecord();
    HP_InsertEntry(info, record);
    int blocksRead = HP_GetAllEntries(info, record.id);
    TEST_CHECK(blocksRead == 1); // We don't search metadata block
    // If i ask for an id that doesnt exist GetAllEntries shouldnt find it
    TEST_CHECK(HP_GetAllEntries(info, 1) == -1);

    // A new block must have been created
    int* numberOfBlocks = malloc(sizeof(int)); 
    BF_GetBlockCounter(info->fileDesc, numberOfBlocks);
    TEST_CHECK(*numberOfBlocks == 2);

    // So far we have the metadata block and one more block
    // which contains 1 record.
    // By inserting 6 more Records, our current block will be full (max capacity == 6)
    // so a new block should be created for the last record (2 in total).
    for (int i = 1; i <= 6; i++) {
        record = randomRecord_WithSpecificID(i);
        HP_InsertEntry(info, record);
    }
    blocksRead = HP_GetAllEntries(info, 6);
    printf("%d\n", blocksRead);
    TEST_CHECK(blocksRead == 2);

    free(numberOfBlocks);
	HP_CloseFile(info);
    BF_Close();
}







TEST_LIST = {
	{ "HP_CreateFile", test_HP_CreateFile },
	{ "HP_OpenFile", test_HP_OpenFile },
	{ "HP_InsertEntry\n     HP_GetAllEntries", test_HP_Insert_HP_Get},
	{ NULL, NULL } // τερματίζουμε τη λίστα με NULL
};