compile:
	gcc -Wall -fPIC -c -O3 -o clangpletion.o clangpletion.c -I../../data_files
	gcc -shared -o libclangpletion.so clangpletion.o ../../data_files/libclang-10.0.0.so
	cp ./libclangpletion.so ../plugin
