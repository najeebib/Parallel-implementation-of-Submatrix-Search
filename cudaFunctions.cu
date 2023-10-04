#include <cuda_runtime.h>
#include <helper_cuda.h>
#include "myProto.h"

#define BLOCK_DIM 16
__global__ void matching(int *pic, int* object,double* matchMatrix, double match, int picDim, int objDim, int picId, int objectId)
{
	int col = blockIdx.x * blockDim.x + threadIdx.x;
    	int row = blockIdx.y * blockDim.y + threadIdx.y;
   	int k = col + row * picDim;//the point in the matrix
   	double sum = 0;								
   	if (k + objDim < picDim*picDim && col+objDim <= picDim && row+objDim <= picDim) //make sure the point is in the matrix
   	{	
   		for(int i=0;i<objDim;i++)//pass through all the points in the matching area										
   		{
   			for(int j=0;j<objDim;j++)
   			{
   				double tmpValue = ((double)pic[k+(i*picDim) + j] - (double)object[i*objDim+j])/(double)pic[k+(i*picDim) + j];
   				
   				if(tmpValue<0)
   					tmpValue = tmpValue * -1;
   				sum += tmpValue;
   			}
   		}
   		int mtachMatDim = picDim - objDim + 1;
   		
   		matchMatrix[(k/picDim)*mtachMatDim +  k%picDim] = sum;//put the result in the results matrix
    	}
}

int computeOnGPU(int *pic,int* object,double* matchMatrix,int picDim,int objectDim,int picId,int objectId, double match)
{
	cudaError_t err = cudaSuccess;
	int sizePic = picDim*picDim;
	int sizeObject = objectDim*objectDim;
	int matchMatrixDim = picDim - objectDim + 1;
	int *d_A, *d_B;
	double *d_MatchMatrix;
	err = cudaMalloc((void **)&d_A, sizePic*sizeof(int));
    	if (err != cudaSuccess) {
        	fprintf(stderr, "Failed to allocate device memory - %s\n", cudaGetErrorString(err));
       		 exit(EXIT_FAILURE);
    	}
    	err = cudaMalloc((void **)&d_B, sizeObject*sizeof(int));
    	if (err != cudaSuccess) {
        	fprintf(stderr, "Failed to allocate device memory - %s\n", cudaGetErrorString(err));
       		 exit(EXIT_FAILURE);
    	}
    	err = cudaMalloc((void **)&d_MatchMatrix, matchMatrixDim*matchMatrixDim*sizeof(double));
    	if (err != cudaSuccess) {
        	fprintf(stderr, "Failed to allocate device memory - %s\n", cudaGetErrorString(err));
       		 exit(EXIT_FAILURE);
    	}
    	err = cudaMemcpy(d_A, pic, sizePic*sizeof(int), cudaMemcpyHostToDevice);
   	 if (err != cudaSuccess) {
       	 	fprintf(stderr, "Failed to copy data from host to device - %s\n", cudaGetErrorString(err));
        	exit(EXIT_FAILURE);
    	}
    	err = cudaMemcpy(d_B, object, sizeObject*sizeof(int), cudaMemcpyHostToDevice);
   	 if (err != cudaSuccess) {
       	 	fprintf(stderr, "Failed to copy data from host to device - %s\n", cudaGetErrorString(err));
        	exit(EXIT_FAILURE);
    	}
    	err = cudaMemcpy(d_MatchMatrix, matchMatrix, matchMatrixDim*matchMatrixDim*sizeof(double), cudaMemcpyHostToDevice);
   	 if (err != cudaSuccess) {
       	 	fprintf(stderr, "Failed to copy data from host to device - %s\n", cudaGetErrorString(err));
        	exit(EXIT_FAILURE);
    	}
	dim3 dimBlock(BLOCK_DIM, BLOCK_DIM);
    	dim3 dimGrid((picDim+dimBlock.x-1)/dimBlock.x, (picDim+dimBlock.y-1)/dimBlock.y);

    	
    	matching<<<dimGrid,dimBlock>>>(d_A, d_B,d_MatchMatrix, match, picDim, objectDim, picId, objectId);
    	
    	err = cudaGetLastError();
    	if (err != cudaSuccess) {
        	fprintf(stderr, "Failed to launch vectorAdd kernel -  %s\n", cudaGetErrorString(err));
        	exit(EXIT_FAILURE);
    	}
    	err = cudaMemcpy(matchMatrix, d_MatchMatrix, matchMatrixDim*matchMatrixDim*sizeof(double), cudaMemcpyDeviceToHost);
    	if (err != cudaSuccess) {
      	  fprintf(stderr, "Failed to copy result array from device to host -%s\n", cudaGetErrorString(err));
     	   exit(EXIT_FAILURE);
   	 }

    	// Free allocated memory on GPU
   	 if (cudaFree(d_MatchMatrix) != cudaSuccess) {
   	     fprintf(stderr, "Failed to free device data - %s\n", cudaGetErrorString(err));
   	     exit(EXIT_FAILURE);
   	 }
   	 if (cudaFree(d_A) != cudaSuccess) {
   	     fprintf(stderr, "Failed to free device data - %s\n", cudaGetErrorString(err));
   	     exit(EXIT_FAILURE);
   	 }
   	 if (cudaFree(d_B) != cudaSuccess) {
   	     fprintf(stderr, "Failed to free device data - %s\n", cudaGetErrorString(err));
   	     exit(EXIT_FAILURE);
   	 }
    	return 0;
}
