gcc -Wall demo/futex_demo.c
gcc -fPIC -c -o ldpreload.o ldpreload.c
gcc -shared -o ldpreload.so ldpreload.o -ldl
LD_PRELOAD=$PWD/ldpreload.so ./a.out
