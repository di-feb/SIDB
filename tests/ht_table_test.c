#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/accutest.h" // A simple library for unit testing
#include "../include/bf.h"
#include "../include/ht_table.h"
#include "../include/record.h"

#define RECORDS_NUM 100 
#define FILE_NAME "data.db"

void test_HT_CreateFile(void) {
	BF_Init(LRU);
	HT_CreateFile(FILE_NAME,10);
    // Check if the file is created.
    TEST_CHECK((access(FILE_NAME, F_OK) == 0));
    // Open the file to get the struct with the metadata.
    HT_info* info = HT_OpenFile(FILE_NAME);
    // Check if the number of buckets is actually 10.
    TEST_CHECK(info->numOfBuckets == 10);

    // We close the file. 
    // The validity of the function HT_CloseFile is tested 
    // by running the program with valgrind and not having leaks.
	HT_CloseFile(info);
    BF_Close();
}

// Λίστα με όλα τα tests προς εκτέλεση
TEST_LIST = {
	{ "HT_CreateFile", test_HT_CreateFile },
	// { "vector_insert_last", test_insert },
	// { "vector_remove_last", test_remove },
	// { "vector_get_set_at", test_get_set_at },
	// { "vector_iterate", test_iterate },
	// { "vector_find", test_find },
	// { "vector_destroy", test_destroy },
	{ NULL, NULL } // τερματίζουμε τη λίστα με NULL
};