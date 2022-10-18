#https://stackoverflow.com/questions/36692315/what-exactly-does-rdynamic-do-and-when-exactly-is-it-needed
#https://users.rust-lang.org/t/exporting-dynamic-symbols-from-executable/6103
#/home/admin/.cargo/bin/rustfilt -i /tmp/demanglge.txt
#https://github.com/rust-lang/rust/issues/73295


gcc -Wall -rdynamic demo/futex_demo.c
[ $? -ne 0 ] && exit
gcc -fPIC -c -o ldpreload.o ldpreload.c
[ $? -ne 0 ] &&  exit
gcc -shared -o ldpreload.so ldpreload.o -ldl
[ $? -ne 0 ] && exit

LD_PRELOAD=$PWD/ldpreload.so ./a.out

#Also see here
