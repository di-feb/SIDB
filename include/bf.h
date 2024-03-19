#ifndef BF_H
#define BF_H

#ifdef __cplusplus
extern "C" {
#endif

#define BF_BLOCK_SIZE 512      /* The size of a block in bytes */
#define BF_BUFFER_SIZE 100     /* The maximum number of blocks  */
#define BF_MAX_OPEN_FILES 100  /* The maximum number of open files */

typedef enum BF_ErrorCode {
  BF_OK,
  BF_OPEN_FILES_LIMIT_ERROR,     /* There are already BF_MAX_OPEN_FILES open files */
  BF_INVALID_FILE_ERROR,         /* The file identifier does not correspond to any open file */
  BF_ACTIVE_ERROR,               /* The BF level is active and cannot be initialized */
  BF_FILE_ALREADY_EXISTS,        /* The file cannot be created because it already exists */
  BF_FULL_MEMORY_ERROR,          /* The memory is filled with active blocks */
  BF_INVALID_BLOCK_NUMBER_ERROR, /* The requested block does not exist in the file */
  BF_AVAILABLE_PIN_BLOCKS_ERROR, /* The file cannot close because there are active blocks in memory */
  BF_ERROR
} BF_ErrorCode;

typedef enum ReplacementAlgorithm {
  LRU,
  MRU
} ReplacementAlgorithm;


// Block Structure
typedef struct BF_Block BF_Block;

/*
 * The BF_Block_Init function initializes and allocates the appropriate memory
 * for the BF_BLOCK structure.
 */
void BF_Block_Init(BF_Block **block);

/*
 * The BF_Block_Destroy function deallocates the memory occupied
 * by the BF_BLOCK structure.
 */
void BF_Block_Destroy(BF_Block **block);

/*
 * The BF_Block_SetDirty function changes the state of the block to dirty.
 * This practically means that the block's data has been changed and the
 * BF level will write the block back to the disk when needed. In
 * case we just read the data without changing it then
 * we do not need to call the function.
 */
void BF_Block_SetDirty(BF_Block *block);

/*
 * The BF_Block_GetData function returns a pointer to the Block's data.
 * If we change the data we should make the block dirty by calling
 * the BF_Block_GetData function.
 */
char* BF_Block_GetData(const BF_Block *block);

/*
 * The BF_Init function initializes the BF level.
 * We can choose between two Block replacement policies
 * that of LRU and that of MRU.
 */
BF_ErrorCode BF_Init(const ReplacementAlgorithm repl_alg);

/*
 * The BF_CreateFile function creates a file named filename that
 * consists of blocks. If the file already exists then an
 * error code is returned. In case of successful execution of the function, BF_OK is returned,
 * while in case of failure, an error code is returned. If you want to
 * see the type of error you can call the BF_PrintError function.
 */
BF_ErrorCode BF_CreateFile(const char* filename);

/*
 * The BF_OpenFile function opens an existing block file named
 * filename and returns the file's identifier in the
 * file_desc variable. In case of success, BF_OK is returned while in case of
 * failure, an error code is returned. If you want to see the type
 * of error you can call the BF_PrintError function.
 */
BF_ErrorCode BF_OpenFile(const char* filename, int *file_desc);

/*
 * The function BF_CloseFile closes the open file with identifier number
 * file_desc. In case of success, BF_OK is returned, while in case of
 * failure, an error code is returned. If you want to see the
 * type of error you can call the function BF_PrintError.
 */
BF_ErrorCode BF_CloseFile(const int file_desc);

/*
 * The function Get_BlockCounter takes as an argument the identifier number
 * file_desc of an open file from block and finds the number of
 * available blocks of it, which it returns in the variable blocks_num.
 * In case of success, BF_OK is returned, while in case of failure,
 * an error code is returned. If you want to see the type of error
 * you can call the function BF_PrintError.
 */
BF_ErrorCode BF_GetBlockCounter(const int file_desc, int *blocks_num);

/*
 * With the function BF_AllocateBlock a new block is allocated for the
 * file with identifier number blockFile. The new block is always
 * allocated at the end of the file, so the number of the block is
 * BF_getBlockCounter(file_desc) - 1. The block that is allocated is pinned
 * in memory (pin) and is returned in the variable block. When we no longer
 * need this block then we must update the block level
 * by calling the function BF_UnpinBlock. In case of success,
 * BF_OK is returned, while in case of failure, an error code
 * is returned. If you want to see the type of error you can call the
 * function BF_PrintError.
 */
BF_ErrorCode BF_AllocateBlock(const int file_desc, BF_Block *block);

/*
 * The function BF_GetBlock finds the block with number block_num of the open
 * file file_desc and returns it in the variable block. The block that
 * is allocated is pinned in memory (pin). When we no longer need this
 * block then we must update the block level by calling the function
 * BF_UnpinBlock. In case of success, BF_OK is returned, while in case of
 * failure, an error code is returned. If you want to see the type of
 * error you can call the function BF_PrintError.
 */
BF_ErrorCode BF_GetBlock(const int file_desc,
                         const int block_num,
                         BF_Block *block);

/*
 * The function BF_UnpinBlock unbinds the block from the Block level which
 * at some point will write it to the disk. In case of success,
 * BF_OK is returned, while in case of failure, an error code
 * is returned. If you want to see the type of error you can call the
 * function BF_PrintError.
 */
BF_ErrorCode BF_UnpinBlock(BF_Block *block);

/*
 * The function BF_PrintError helps in printing the errors that may
 * exist with the call of block file level functions. A description of the error
 * is printed to stderr.
 */
void BF_PrintError(BF_ErrorCode err);

/*
 * The function BF_Close closes the Block level writing any
 * blocks it had in memory to the disk.
 */
BF_ErrorCode BF_Close();

#ifdef __cplusplus
}
#endif
#endif // BF_H
