build:
	mpicxx -fopenmp -lm -c main.c -o main.o
	nvcc -I./inc -c cudaFunctions.cu -o cudaFunctions.o
	mpicxx -fopenmp -lm -o Proj  main.o  cudaFunctions.o  /usr/local/cuda/lib64/libcudart_static.a -ldl -lrt

clean:
	rm -f *.o ./Proj

run:
	mpiexec -np 2 ./Proj

runOn2:
	mpiexec -np 2 -machinefile  mf  -map-by  node  ./Proj
