compile:
	gcc -Wall -fPIC -c code_complete.c func_helper.c globals.c -I../include
	gcc -shared -O3 -o libclangpletion.so code_complete.o func_helper.o globals.o ../data_files/libclang-10.0.0.so
	rm code_complete.o func_helper.o globals.o
	cp ./libclangpletion.so ../plugin
