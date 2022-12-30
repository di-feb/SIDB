## <b>System Implementation of Data Bases 1: Implemantation of HashTable  

### <b>Documentation
In the context of the project you will implement a set of functions that  
manage files created based on **heap file** organization and **static hash table**.


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

### **How to compile and run:** 
#### <b><u>Hash Table</u>  
We just write *make ht* into the current directory.  
Then to run the program we write *./build/ht_main* into the current directory.
#### <b><u>Heap</u>
We just write *make hp* into the current directory.  
Then to run the program we write *./build/hp_main* into the current directory.

#### <b><u>Caution!!!</u>
If we want to rerun the programs we need to delete the data.db file that is going  
to be created after the first execution.  
To delete it just write *rm data.db* inside the current directory.  

