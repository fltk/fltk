euc_tw: euc_tw.c
	gcc  euc_tw.c -o  euc_tw

all: convert_map create_table conv_gen
	./tbl_gen.sh
	./non_spacing.sh
	./case.sh

conv_gen: conv_gen.c
	gcc conv_gen.c -o conv_gen

convert_map: convert_map.c
	gcc convert_map.c -o convert_map

create_table: create_table.c
	gcc create_table.c -o create_table

clean:
	rm -f *.o convert_map create_table conv_gen euc_tw
