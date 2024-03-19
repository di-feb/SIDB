#pragma once

#include "record.h"
#include <stdbool.h>

#ifndef HT_TABLE_H
#define HT_TABLE_H

typedef unsigned int uint;
typedef unsigned long ulint;

typedef struct {
    bool isHashTable;                   // Flag that identifies if a file is a HT file.
    char* fileName;                     // Name of the file.
    uint fileDesc;                      // File opening ID number from the block level.
    ulint numOfBuckets;                 // The number of "buckets" in the file hash file.
} HT_info;

typedef struct {
    int blockIndex;                 // Index of the block.             
    int next;                       // Id of the next block.
    ulint numOfRecords;             // Number of records inside the block.
} HT_block_info;

// The HT_CreateFile function is used to create and initialize an empty hash file named fileName.
// It takes as parameters the name of the file where the heap will be built and the number of hash function buckets.
// If executed successfully, it returns 0, otherwise -1.
int HT_CreateFile(char *fileName, int buckets);

// The HT_OpenFile function opens the file named filename and reads from the first block the information about the hash file.
// Then, a structure that holds as much information as necessary for this file is updated so that you can process its records later.
// After the file information structure is properly updated, it is returned.
// If any error occurs, a NULL value is returned.
// If the file given for opening is not a hash file, this is also considered an error.
HT_info* HT_OpenFile(char *fileName);

// The HT_CloseFile function closes the file specified in the header_info structure.
// If executed successfully, it returns 0, otherwise -1.
// The function is also responsible for freeing the memory occupied by the structure that was passed as a parameter, if the closure was successful.
int HT_CloseFile(HT_info* header_info);

// The HT_InsertEntry function is used to insert a record into the hash file.
// The information about the file is in the header_info structure, while the record to be inserted is specified by the record structure.
// If executed successfully, you return the number of the block in which the insertion was made (blockId), otherwise -1.
int HT_InsertEntry(HT_info* header_info, Record record);

// This function is used to print all records in the hash file that have a value in the key field equal to value.
// The first structure gives information about the hash file, as it was returned from HT_OpenIndex.
// For each record in the file that has a value in the key field (as defined in HT_info) equal to value, its contents are printed (including the key field).
// It also returns the number of blocks that were read until all records were found.
// In case of success, it returns the number of blocks that were read, while in case of error it returns -1.
int HT_GetAllEntries(HT_info* header_info, int value);

// Prints the statistical data of a hash table file with the given file name.
// The statistics are as follows:
// 1. How many blocks a file has,
// 2. The average number of blocks each bucket has
// 3. The minimum, average, and maximum number of records each bucket of a file has,
// 4. The number of buckets that have overflow blocks, and how many blocks are these for each bucket.
int HashStatistics(char *fileName);

#endif // HT_FILE_H
