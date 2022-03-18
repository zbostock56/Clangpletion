gcc -Wall -fPIC -c code_complete.c func_helper.c globals.c -I../include -LC:/'Program Files'/LLVM/lib -lclang
gcc -shared -O3 -o libclangpletion.dll code_complete.o func_helper.o globals.o libclang.lib
rm code_complete.o func_helper.o globals.o
cp ./libclangpletion.dll ../plugin
