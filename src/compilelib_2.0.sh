gcc -Wall -I../include -fPIC -c code_complete_2.0.c func_helper.c init.c helpers.c -l:libclang.lib
gcc -L./ -shared -O3 -o libclangpletion.dll code_complete_2.0.o func_helper.o init.o helpers.o libclang.lib
rm code_complete_2.0.o func_helper.o init.o helpers.o
cp ./libclangpletion.dll ../plugin
