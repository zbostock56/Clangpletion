gcc -Wall -g -fPIC -c -O3 -o clangpletion.o clangpletion.c -IC:/'Program Files'/LLVM/include -LC:/'Program Files'/LLVM/lib -lclang 

gcc -shared -o libclangpletion.dll clangpletion.o libclang.lib

#clang -Wall -IC:/'Program Files'/LLVM/include -LC:/'Program Files'/LLVM/lib -libclang -c -o clangpletion.o clangpletion.c

