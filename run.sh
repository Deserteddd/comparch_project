gcc -o target/init init.c;
gcc -o target/scheduler scheduler.c;
target/init 123 test.txt;
target/scheduler 123 test.txt;
