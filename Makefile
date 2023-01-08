sht:
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/sht_main.c ./src/record.c ./src/sht_table.c ./src/ht_table.c -lbf -o ./build/sht_main -O2
	./build/sht_main

val_sht:
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/sht_main.c ./src/record.c ./src/sht_table.c ./src/ht_table.c -lbf -o ./build/sht_main -O2
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./build/sht_main 

ht:
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/ht_main.c ./src/record.c ./src/ht_table.c -lbf -o ./build/ht_main -O2
	./build/ht_main

val_ht:
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/ht_main.c ./src/record.c ./src/ht_table.c -lbf -o ./build/ht_main -O2
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./build/ht_main 

hp:
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/hp_main.c ./src/record.c ./src/hp_file.c -lbf -o ./build/hp_main -O2
	./build/hp_main

val_hp:
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/hp_main.c ./src/record.c ./src/hp_file.c -lbf -o ./build/hp_main -O2
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./build/hp_main 
clean_sht:
	rm build/sht_main
	rm data.db
	rm index.db

clean_ht:
	rm build/ht_main
	rm data.db

clean_hp:
	rm build/hp_main
	rm data.db
