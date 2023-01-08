#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }
#define MAX_RECORDS (BF_BLOCK_SIZE - sizeof(HP_block_info)) / sizeof(Record)
#define HP_BLOCK_INFO_POSITION BF_BLOCK_SIZE - sizeof(HP_block_info)
#define RECORDS_NUM_POSITION BF_BLOCK_SIZE - sizeof(int);

// Function for new block allocation, returns block ID
int newBlock(HP_info *headerBlock) {
	BF_Block *block;
	BF_Block_Init(&block);
	
	HP_block_info *blockInfo = malloc(sizeof(HP_block_info));
	int *blocksNum = malloc(sizeof(int)); // Only used to get number of blocks
	
	int blockId = -1; // Initialised for error prevention
	int blocks;

	CALL_OR_DIE(BF_AllocateBlock(headerBlock->fileDesc, block));
	CALL_OR_DIE(BF_GetBlockCounter(headerBlock->fileDesc, blocksNum));
	blocks = *blocksNum;
	CALL_OR_DIE(BF_GetBlock(headerBlock->fileDesc, blocks - 1, block));

	// Initialise block
	blockInfo->blockId = blocks - 1;
	blockInfo->nextId = blocks;
	blockInfo->records = 0;

	char *data = BF_Block_GetData(block);

	// Go to the end of the block
	data += HP_BLOCK_INFO_POSITION;

	memcpy(data, blockInfo, sizeof(HP_block_info));

	BF_Block_SetDirty(block);
	CALL_OR_DIE(BF_UnpinBlock(block));

	BF_Block_Destroy(&block);

	// Update headerBlock
	headerBlock->blocksNo++;

	blockId = blockInfo->blockId;
	free(blockInfo);
	free(blocksNum);

  return blockId;
}

// Checks if a specific file is a Heap File
bool isHeapFile(HP_info *info) {
	if (info->isHeap) {
		return false;
	}
	return true;
}

int HP_CreateFile(char *fileName) {
	
	if (fileName == NULL)
		return -1;

	BF_Block *block0;
	BF_Block_Init(&block0);

	HP_info *headerBlock = malloc(sizeof(HP_info));
	HP_block_info *blockInfo = malloc(sizeof(HP_block_info));

	int fileDesc;

	CALL_OR_DIE(BF_CreateFile(fileName));
	CALL_OR_DIE(BF_OpenFile(fileName, &fileDesc));

	CALL_OR_DIE(BF_AllocateBlock(fileDesc, block0));
	CALL_OR_DIE(BF_GetBlock(fileDesc, 0, block0));

	// Initialise header block
	headerBlock->isHeap = T;
	headerBlock->fileDesc = fileDesc;
	headerBlock->blocksNo = 1;

	// Initialise block info
	blockInfo->blockId = 0;
	blockInfo->nextId = -1;
	blockInfo->records = 0;

	char *data = BF_Block_GetData(block0);

	// Copy headerBlock data
	memcpy(data, headerBlock, sizeof(HP_info));

	// Go to the end of the block
	data += HP_BLOCK_INFO_POSITION - sizeof(HP_info);

	// Copy blockInfo data
	memcpy(data, blockInfo, sizeof(HP_block_info));

	BF_Block_SetDirty(block0);
	CALL_OR_DIE(BF_UnpinBlock(block0));
	BF_Block_Destroy(&block0);

	CALL_OR_DIE(BF_CloseFile(fileDesc));

	free(headerBlock);
	free(blockInfo);

	return 0;
}

HP_info* HP_OpenFile(char *fileName) {
	
	if (fileName == NULL)
		return NULL;
	
	BF_Block *block0;
	BF_Block_Init(&block0);
	
	HP_info *headerBlock = malloc(sizeof(HP_info));
	HP_block_info *blockInfo = malloc(sizeof(HP_block_info));

	int fileDesc;
	
	CALL_OR_DIE(BF_OpenFile(fileName, &fileDesc));
	CALL_OR_DIE(BF_GetBlock(fileDesc, 0, block0));


	char *data = BF_Block_GetData(block0);

	memcpy(headerBlock, data, sizeof(HP_info));
	headerBlock->fileName = malloc(sizeof(fileName));
	strcpy(headerBlock->fileName, fileName);

	memcpy(data, headerBlock, sizeof(HP_info));

	// Check type of file
	if (headerBlock->isHeap == F) {
		printf("Wrong type of file\n");
		free(headerBlock);
		return NULL;
	}

	CALL_OR_DIE(BF_UnpinBlock(block0));
	BF_Block_Destroy(&block0);

	return headerBlock;
}

int HP_CloseFile(HP_info* hp_info ) {
	
	if (hp_info->fileDesc < 0)
		return -1;

	free(hp_info->fileName);

	CALL_OR_DIE(BF_CloseFile(hp_info->fileDesc))

	free(hp_info);

	return 0;
}

int HP_InsertEntry(HP_info* hp_info, Record record) {
	
	if (hp_info->fileDesc < 0)
		return -1;
	
	BF_Block *block;
	BF_Block_Init(&block);

	int *blocks = malloc(sizeof(int)); // Only used to get number of blocks
	CALL_OR_DIE(BF_GetBlockCounter(hp_info->fileDesc, blocks));
	int totalBlocks = *blocks;

	if (totalBlocks < 0) {
		printf("Error in BF_GetBlockCounter\n");
		return -1;
	}
	
	// Indexing starts at 0
	int currBlock = totalBlocks - 1;

	// We don't insert Records at the header Block
	if (currBlock == 0) {
		// Allocate a new block
		int nextBlock = newBlock(hp_info);
		if (nextBlock == -1) {
			return -1;
		}
		// Set the new block as current
		currBlock = nextBlock;
	}

	CALL_OR_DIE(BF_GetBlock(hp_info->fileDesc, currBlock, block));

	char *data = BF_Block_GetData(block);
	
	// Temporary variable to hold number of records
	int records = 0;

	// Go to the number of records
	data += RECORDS_NUM_POSITION; // Number of records is located at the final 4 bytes of each block
	memcpy(&records, data, sizeof(int));
	
	// Back at the start of block
	data = BF_Block_GetData(block);

	// If current Block is full, make a new one
	if (records == MAX_RECORDS) {
		// Unpin the full one, we don't need it
		CALL_OR_DIE(BF_UnpinBlock(block));

		int nextBlock = newBlock(hp_info);
		if (nextBlock == -1) {
			return -1;
		}
		CALL_OR_DIE(BF_GetBlock(hp_info->fileDesc, nextBlock, block));

		// Back at the start of the block
		data = BF_Block_GetData(block);
		
		data += RECORDS_NUM_POSITION;
		memcpy(&records, data, sizeof(int));
		data = BF_Block_GetData(block);

		currBlock = nextBlock;
	}

	// Put the record at the last available position
	data += sizeof(record) * records;
	memcpy(data, &record, sizeof(record));

	// Back at the start of the block
	data = BF_Block_GetData(block);
	
	// Update the number of Records
	data += RECORDS_NUM_POSITION;
	records++;
	memcpy(data, &records, sizeof(int));

	BF_Block_SetDirty(block);
	CALL_OR_DIE(BF_UnpinBlock(block));
	BF_Block_Destroy(&block);
	
	free(blocks);

	return 0;
}

int HP_GetAllEntries(HP_info* hp_info, int value) {
	
	if (hp_info->fileDesc < 0)
		return -1;
	
	BF_Block *block;
	BF_Block_Init(&block);
	
	Record *record = malloc(sizeof(Record));
	int *blocksNum = malloc(sizeof(int)); // Only used to get number of blocks
	char *data = NULL;

	int blocks, blocksRead, currBlock;

	CALL_OR_DIE(BF_GetBlockCounter(hp_info->fileDesc, blocksNum));
	blocks = *blocksNum;

	// Search every block (no need to check block0)
	for (currBlock = 1; currBlock < blocks; currBlock++) {
		CALL_OR_DIE(BF_GetBlock(hp_info->fileDesc, currBlock, block));
		data = BF_Block_GetData(block);

		int records, currRecord;

		data += RECORDS_NUM_POSITION;
		memcpy(&records, data, sizeof(int));
		
		data = BF_Block_GetData(block);

		// In each block, search every Record 
		for (currRecord = 0; currRecord < records; currRecord++) {
			memcpy(record, data, sizeof(Record));

			if (record->id == value) {
				printRecord(*record);
				
				CALL_OR_DIE(BF_UnpinBlock(block));
				BF_Block_Destroy(&block);
				free(record);
				free(blocksNum);

				blocksRead = currBlock;
				
				return blocksRead;
			}
			// Go to the next Record
			data += sizeof(Record);
		}
		CALL_OR_DIE(BF_UnpinBlock(block));
	}
	CALL_OR_DIE(BF_UnpinBlock(block));
	BF_Block_Destroy(&block);
	
	free(record);
	free(blocksNum);
	return -1;
}