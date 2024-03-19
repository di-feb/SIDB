# Data Structures Implementation: Hash Table, Secondary Hash Table

## Overview

This project implements a static **hash table** and a ***secondary hash table*** for ***record management***.  
The hash tables are stored in files and use the Block File (BF) library (bf.h functions) for low-level disk block abstraction.  

The project includes the following main functions:

- `HT_CreateFile`: Creates and initializes an empty hash file.
- `HT_OpenFile`: Opens a hash file and reads its information.
- `HT_CloseFile`: Closes a hash file and frees the associated memory.
- `HT_InsertEntry`: Inserts a record into a hash file.
- `HT_GetAllEntries`: Prints all records in a hash file that have a specific key value.
- `HashStatistics`: Prints statistical data of a hash file.
- `SHT_CreateSecondaryIndex`: Creates and initializes a secondary hash file for a primary hash file.
- `SHT_OpenSecondaryIndex`: Opens a secondary hash file and reads its information.
- `SHT_CloseSecondaryIndex`: Closes a secondary hash file and frees the associated memory.
- `SHT_SecondaryInsertEntry`: Inserts a record into a secondary hash file.
- `SHT_SecondaryGetAllEntries`: Prints all records in a secondary hash file that have a specific key value.

## Documentation

This project involves the implementation of a set of functions that manage files created based on **static hash table** and **static secondary hash table**  organization.  

### Hash Table

- All functions are implemented inside the `ht_table.c` file.
- Some auxiliary utility functions are implemented at the beginning of the `ht_table.c` file.
- The utility functions contain comments describing their operation in detail.
- Assumptions in the code:
  - The first block of the file (block with id = 0) contains the `HT_info` struct, which holds the metadata of the HT file.
  - The `HT_block_info` struct, which contains data for a specific block, is located at the end of every block.
  - The second block of the file contains the buckets of the Hash Table.
  - The buckets of the Hash Table are represented as an array containing an integer in every position. This integer is the ID of the first block to which this particular bucket points.

### Secondary Hash Table

- All functions are implemented inside the `sht_table.c` file.  
- Some auxiliary utility functions are implemented at the beginning of the `sht_table.c` file.  
- The utility functions contain comments describing their operation in detail.  
- Assumptions in the code:
  - The first block of the file (block with id = 0) contains the `SHT_info` struct, which holds the metadata of the SHT file.
  - The `SHT_block_info` struct, which contains data for a specific block, is located at the end of every block.
  - The second block of the file contains the buckets of the Secondary Hash Table.
  - The blocks of the SHT files do not hold Records. They hold `SHT_Records` which is a struct implemented inside the `record.h` file.

### Tests

- Tests have been implemented in the `tests` directory for each file: **ht_table.c**, **sht_table.c**.
- A specific Makefile is provided in the `tests` directory to run these tests.

### Known Issues

- There is an issue with the file descriptor that the `BF_OPEN_FILE` returns. If the HT file is not opened when we create the SHT file, `BF_OPEN_FILE` assigns the same file descriptor to both files. This results in both files being closed after we attempt to close either one of them.
- When running with valgrind, an Error will occur. This is due to a false into the library BF functions, and not the ht_table or sht_table files.

## How to Compile and Run

This project uses a Makefile for easy compilation and execution of the programs. Here are the steps to compile and run the programs:

### Run Hash Table

1. Open a terminal in the project's root directory.
2. To compile the Hash Table, use the following command:

    ```c
    make ht
    ```

3. To run the Hash Table with valgrind, use the following command:

    ```c
    make val_ht
    ```

    This will run the ht_main file inside the examples directory.

### Run Secondary Hash Table

1. Open a terminal in the project's root directory.
2. To compile the Secondary Hash Table, use the following command:

    ```c
    make sht
    ```

3. To run the Secondary Hash Table with valgrind, use the following command:

    ```c
    make val_sht
    ```
    
    This will run the sht_main file inside the examples directory.

### Run Tests

#### ht_table Test

1. Navigate to the `tests` directory:

    ```c
    cd tests
    ```

2. To compile and run the ht_table test, use the following command:

    ```c
    make ht_test
    ```

3. To run the ht_table test with valgrind, use the following command:

    ```c
    make val_ht_test
    ```

#### sht_table Test

1. Navigate to the `tests` directory:

    ```c
    cd tests
    ```

2. To compile and run the sht_table test, use the following command:

    ```c
    make sht_test
    ```

3. To run the sht_table test with valgrind, use the following command:

    ```c
    make val_sht_test
    ```

Please note that these instructions assume that you have `make` and the necessary compilers installed on your system.

### Caution

After running **ht_table** implementation and its equivalent tests, if you want to rerun the programs,  
you need to delete the `data.db` file that is created after the first execution.  
To delete these files, simply run `make clean_ht` in the current directory.

After running **sht_table.c** or its equivalent test, you will need to delete both `data.db` and `index.db` files.

To delete these files, simply run `make clean_sht` in the current directory.
