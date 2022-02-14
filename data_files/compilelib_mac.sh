gcc -Wall -fPIC -c -o clangpletion_mac.o clangpletion.c -I./ -L/Library/Developer/CommandLineTools/usr/lib -lclang
gcc -shared -o libclangpletion_mac.so clangpletion_mac.o libclang.dylib  
