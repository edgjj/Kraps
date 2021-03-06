
gcc -g -c tinywav/tinywav.c -o ./builds/tinywav.o
gcc -g -c fft/fftsg.c -o ./builds/fft.o

g++ -g -c main.cpp -o ./builds/main.o 
g++ -g -c processor/processor.cpp -o ./builds/processor.o 
g++ -g -c modulators/adsr.cpp -o ./builds/adsr.o 
g++ -g -c generators/wt.cpp -o ./builds/wt.o 
g++ -g -c generators/generator.cpp -o ./builds/generator.o 

g++ -o ./a.out ./builds/main.o ./builds/processor.o ./builds/adsr.o ./builds/wt.o ./builds/generator.o ./builds/tinywav.o ./builds/fft.o
./a.out $1 $2
