#ifndef SHT_TABLE_H
#define SHT_TABLE_H
#include "record.h"
#include "ht_table.h"


typedef struct {
    bool isSecondaryHashTable;          // Flag that identifies if a file is a HT file.
    char* fileName;                     // Name of the file.
    uint fileDesc;                      // File opening ID number from the block level.
    ulint numOfBuckets;                 // The number of "buckets" in the file hash file
} SHT_info;

typedef struct {
    int blockIndex;                 // Index of the block.             
    int next;                       // Id of the next block.
    ulint numOfSHTRecords;          // Number of records inside the block
} SHT_block_info;

/* The function SHT_CreateSecondaryIndex is used for the creation
and proper initialization of a secondary hash file with
name sfileName for the primary hash file fileName. In
case it is executed successfully, it returns 0, otherwise
it returns -1.*/
int SHT_CreateSecondaryIndex(
    char *sfileName, /* secondary index file name */
    int buckets, /* number of hash buckets */
    char* fileName /* primary index file name */);

/* The function SHT_OpenSecondaryIndex opens the file with name sfileName
and reads from the first block the information regarding the secondary
hash index.*/
SHT_info* SHT_OpenSecondaryIndex(
    char *sfileName /* secondary index file name */);

/* The function SHT_CloseSecondaryIndex closes the file specified
inside the header_info structure. In case it is executed successfully, it returns
0, otherwise it returns -1. The function is also responsible for the
deallocation of the memory occupied by the structure passed as a parameter,
in case the closure was successful.*/
int SHT_CloseSecondaryIndex( SHT_info* header_info );

/* The function SHT_SecondaryInsertEntry is used for the insertion of a
record in the hash file. The information regarding the file
is located in the header_info structure, while the record to be inserted is specified
by the record structure and the block of the primary index where the record
to be inserted exists. In case it is executed successfully, it returns 0, otherwise
it returns -1.*/
int SHT_SecondaryInsertEntry(
    SHT_info* header_info, /* header of the secondary index */
    Record record, /* the record for which we have insertion in the secondary index */
    int block_id /* the block of the hash file where the insertion was made */);

/* This function is used for the printing of all the records that
exist in the hash file which have a value in the key-field
of the secondary index equal to name. The first structure contains information
about the hash file, as they were returned during its opening.
The second structure contains information about the secondary index as
they were returned by SHT_OpenIndex. For each record that exists
in the file and has a name equal to value, its contents are printed
(including the key-field). It also returns the
number of blocks that were read until all the records were found. In
case of error it returns -1.*/
int SHT_SecondaryGetAllEntries(
    HT_info* ht_info, /* header of the primary index file */
    SHT_info* header_info, /* header of the secondary index file */
    char* name /* the name on which the search is performed */);

uint hash_string(void*);
#endif // SHT_FILE_H
