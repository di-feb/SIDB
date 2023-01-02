#ifndef RECORD_H
#define RECORD_H




typedef enum Record_Attribute {
  ID,
  NAME,
  SURNAME,
  CITY
} Record_Attribute;

typedef struct Record {
    char record[15];
	int id;
	char name[15];
	char surname[20];
	char city[20];
} Record;

typedef struct SHT_Record {
    char name[15];      // Key of the SHT, aka name field of the record struct. 
	int blockId;        // The block Id inside the primitive HT where this name is stored. 
} SHT_Record;

Record randomRecord();

void printRecord(Record record);

#endif
