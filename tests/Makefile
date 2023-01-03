# paths
INCLUDE = ../include/
LIB = ../lib/

# compiler
CC = gcc

# Compile options.
CFLAGS = -Wall -g -I$(INCLUDE)
LDFLAGS = -lm 

# objects HT_table
HT_table_test_OBJS = ht_table_test.o ../src/ht_table.o ../src/record.c $(LIB)/libbf.so
# objects SHT_table
SHT_table_test_OBJS = sht_table_test.o ../src/sht_table.o ../src/record.c $(LIB)/libbf.so

# The executable program for ht
EXEC_HT = ht_test

# The executable program for sht
EXEC_SHT = sht_test


$(EXEC_HT): $(HT_table_test_OBJS)
	$(CC) $(CFLAGS) $(HT_table_test_OBJS) -o $(EXEC_HT) $(LDFLAGS)
	rm $(RMFLAGS) $(HT_table_test_OBJS)

$(EXEC_SHT): $(SHT_table_test_OBJS)
	$(CC) $(CFLAGS) $(SHT_table_test_OBJS) -o $(EXEC_SHT) $(LDFLAGS)
	rm $(RMFLAGS) $(SHT_table_test_OBJS)
clean_ht:
	rm -f $(HT_table_test_OBJS) $(EXEC_HT)

clean_sht:
	rm -f $(SHT_table_test_OBJS) $(EXEC_SHT)

run_ht: $(EXEC_HT)
	./$(EXEC_HT)

run_sht: $(EXEC_SHT)
	./$(EXEC_SHT)

valgrind_ht: $(EXEC)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(EXEC_HT)

valgrind_sht: $(EXEC)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(EXEC_SHT)
