## <b>System Implementation of Data Bases 1, 2.

### <b> Authors
- Alex Alatzas      sdi1900005
- Stathis Demenagas sdi1900045

### <b>Documentation
In the context of the project you will implement a set of functions that  
manage files created based on **heap file** organization and **static hash table**.

### <b>Heap File
- All functions are implemented inside the hp_file.c file.   
- At the beginning of the hp_file.c file there is an assistant ulitity function implemented.  
- The ulitity function contains comments describing in detail its operation. 
- Related to assumptions in the code:  

    - Inside the first block of the file (block with id = 0) there is the struct HP_info  
    which contains the metadata of the Heap File. There will be no records there.
    - Block with id:0 remains pinned until file is closed in order to update its data
    - At the end of every block there is the struct HP_block_info
    which contains some data for this specific block.
    - Block indexing starts at 0, which means that block k has id: k-1  
### <b>Hash Table
- All functions are implemented inside the ht_table.c file.   
- At the beginning of the ht_table.c file some assistant ulitity functions have been implemented.  
- The ulitity functions contain comments describing in detail their operation.  
- Related to assumptions in the code:  

    - Inside the first block of the file (block with id = 0) there is the struct HT_info  
    which contains the metadata of the HT file. 
    - At the end of every block there is the struct HT_block_info    
    which contains some data for this specific block. 
    - The second block of the file contains the buckets of the Hash Table.  
    - The buckets of the Hash Table are represented as an array containing an integer  
    inside every position. This integer is the ID of the first block to which this  
    particular bucket points.    
    In other words, it is the block to which the Records whose ID hashes them are assigned.  
    in the specific position of the table. 

### <b>Secondary Hash Table 
- All functions are implemented inside the sht_table.c file.   
- At the beginning of the ht_table.c file some assistant ulitity functions have been implemented.  
- The ulitity functions contain comments describing in detail their operation.  
- Related to assumptions in the code:

    - Inside the first block of the file (block with id = 0) there is the struct SHT_info  
    which contains the metadata of the SHT file. 
    - At the end of every block there is the struct SHT_block_info    
    which contains some data for this specific block. 
    - The second block of the file contains the buckets of the Secondary Hash Table.
    - The blocks of the sht files do not hold Records.  
    They hold SHT_Records which is a struct implemented inside the record.h file.
- Τests have been implemented in the directory tests
    - We have implemented tests for every file HT, HP, SHT. 
    - There is a specific Makefile under directory tests to run these tests.
- Problems occured:
    We have a problem with the file descriptor that the BF_OPEN_FILE returns.   
    If the HT file is not opened when we create the SHT file the BF_OPEN_FILE    
    gives to both files the same file descriptor.    
    This results in closing both files after we try to close either one of them. 

### **How to compile and run:** 
#### <b><u>Heap</u>
We just write *make hp* into the current directory.    
To run with valgrind we just write *make val_hp* into the current directory.
#### <b><u>Hash Table</u>  
We just write *make ht* into the current directory.    
To run with valgrind we just write *make val_ht* into the current directory.
#### <b><u>Secondary Hash Table</u>
We just write *make sht* into the current directory.   
To run with valgrind we just write *make val_sht* into the current directory.

#### <b><u>Hp Test</u>
We just write *make hp_test*  under the /test directory.   
To run with valgrind we just write *make val_hp_test*  under the /test directory.

#### <b><u>Ht Test</u>
We just write *make ht_test*  under the /test directory.   
To run with valgrind we just write *make val_ht_test*  under the /test directory.

#### <b><u>Sht Test</u>
We just write *make sht_test* under the /test directory.   
To run with valgrind we just write *make val_sht_test* under the /test directory.

#### <b><u>Caution!!!</u>
For running HT, HP implementations and their equivalent tests:  
If we want to rerun the programs we need to delete the data.db file that is going  
to be created after the first execution.  
To delete it just write *rm data.db* inside the current directory or make clean.

For running SHT  and their equivalent test:
You will need to delele both data.db and index.db files


