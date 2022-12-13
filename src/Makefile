LIB := ../data_files/libclang-10.0.0.so

compile:
	gcc -Wall -I../include -fPIC -c code_complete_2.0.c func_helper.c helpers.c init.c
	gcc -shared -O3 -o libclangpletion.so code_complete_2.0.o func_helper.o helpers.o init.o $(LIB)
	rm code_complete_2.0.o func_helper.o helpers.o init.o
	cp ./libclangpletion.so ../plugin
