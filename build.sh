
gcc -O2 -c tinywav/tinywav.c -o ./builds/tinywav.o
gcc -O2 -c fft/fftsg.c -o ./builds/fft.o

g++ -O2 -c main.cpp -o ./builds/main.o 
g++ -O2 -c generators/wt.cpp -o ./builds/wt.o 
g++ -O2 -c generators/generator.cpp -o ./builds/generator.o 

g++ -o ./a.out ./builds/main.o ./builds/wt.o ./builds/generator.o ./builds/tinywav.o ./builds/fft.o
./a.out $1 $2
