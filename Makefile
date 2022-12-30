sht:
	@echo " Compile hp_main ...";
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/sht_main.c ./src/record.c ./src/sht_table.c ./src/ht_table.c -lbf -o ./build/sht_main -O2
	./build/sht_main
	rm build/sht_main
	rm data.db

valgrind:
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/ht_main.c ./src/record.c ./src/ht_table.c -lbf -o ./build/sht_main -O2
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./build/sht_main 
	rm build/sht_main
	rm data.db

clean:
	rm build/sht_main
