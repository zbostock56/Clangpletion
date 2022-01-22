gcc -Wall -g -fPIC -c -o clangpletion.o clangpletion.c -IC:/'Program Files'/LLVM/include -LC:/'Program Files'/LLVM/lib -lclang 

#gcc -shared -o libclangpletion.dll clangpletion.o

#clang -Wall -IC:/'Program Files'/LLVM/include -LC:/'Program Files'/LLVM/lib -libclang -c -o clangpletion.o clangpletion.c

