#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "ht_table.h"
#include "sht_table.h"

#define RECORDS_NUM 30 // you can change it if you want
#define FILE_NAME "data.db"
#define INDEX_NAME "index.db"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }


int main() {
    srand(12569874);
    BF_Init(LRU);
    // Initializations
    HT_CreateFile(FILE_NAME,10);
    SHT_CreateSecondaryIndex(INDEX_NAME,10,FILE_NAME);

    HT_info* info = HT_OpenFile(FILE_NAME); // HT file should be opened so that the ht, sht files get different fileDesc 
    SHT_info* index_info = SHT_OpenSecondaryIndex(INDEX_NAME);

    // We will search for the name searchName later
    Record record=randomRecord();
    char searchName[15];
    strcpy(searchName, record.name);

    // We insert random records both in the hash file which we also add to the secondary index
    printf("Insert Entries\n");
    for (int id = 0; id < RECORDS_NUM; ++id) {
        record = randomRecord();
        int block_id = HT_InsertEntry(info, record);
        SHT_SecondaryInsertEntry(index_info, record, block_id);
    }
    // We print all records with name searchName
    printf("RUN PrintAllEntries for name %s\n",searchName);
    if(SHT_SecondaryGetAllEntries(info,index_info,searchName) == -1){
        printf("-1\n");
    }
    // We close the hash file and the secondary index

    HT_CloseFile(info); 
    if(HashStatistics(FILE_NAME) == -1)
        printf("HashStatics -1\n");

    // SHT_CloseSecondaryIndex(index_info); // This will give us leaks due to an error in the BF library
    // We will still get some leaks because we didn't close the file, but everything else should be fine
    
    BF_Close();
}
