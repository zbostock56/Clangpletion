LIBS := C:\msys64\mingw64\lib

compile:
	gcc -Wall -I../include -fPIC -c code_complete_2.0.c helpers.c init.c
	gcc -L$(LIBS) -shared -O3 -o libclangpletion.so code_complete_2.0.o helpers.o init.o -lclang
	rm code_complete_2.0.o helpers.o init.o
	cp ./libclangpletion.so ../plugin
