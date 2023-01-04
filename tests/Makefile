sht_test:
	@echo " Compile hp_main ...";
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./sht_table_test.c ../src/record.c ../src/sht_table.c ../src/ht_table.c -lbf -o ./sht_table_test -O2
	./sht_table_test
	rm sht_table_test
	rm data.db
	rm index.db

ht_test:
	@echo " Compile hp_main ...";
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./ht_table_test.c ../src/record.c ../src/ht_table.c -lbf -o ./ht_table_test -O2
	./ht_table_test
	rm ht_table_test
	rm data.db

val_sht_test:
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./sht_table_test.c ../src/record.c ../src/sht_table.c ../src/ht_table.c -lbf -o ./sht_table_test -O2
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./sht_table_test
	rm sht_table_test
	rm data.db
	rm index.db

val_ht_test:
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./ht_table_test.c ../src/record.c ../src/ht_table.c -lbf -o ./ht_table_test -O2
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./ht_table_test
	rm ht_table_test
	rm data.db
clean_sht:
	rm sht_table_test
	rm data.db
	rm index.db

clean_ht:
	rm ht_table_test
	rm data.db
