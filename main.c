#define FILE_NAME "input.txt"
#include <mpi.h>
#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <math.h>
#include "myProto.h"
#include <cstring>
void sort(int** arr,int numOfPics)//helper function to sort the result array in order to print the pictures' result in the pic ids order (pic 0 then 1 etc)
{
	int* tmp = *arr;
	int* tmpArr = (int*)malloc(5*sizeof(int));
	for(int i=0;i<numOfPics-1;i++)//sort using the buble sort methobbd
	{
		for(int j=0;j<numOfPics-1-i;j++)
		{
			if(tmp[j*5]>tmp[(j+1)*5])
			{
				memcpy(tmpArr,tmp+5*(j+1),5*sizeof(int));
				int count =0;
				for(int k=(j+1)*5;k<(j+1)*5+5;k++)
				{
					tmp[k] = tmp[j*5+count];
					count++;
				}
				count = 0;
				for(int k=j*5;k<j*5+5;k++)
				{
					tmp[k] = tmpArr[count];
					count++;
				}
			}
		}
	}
}


int isMatching(double a,double b)//helper function to check if two floats are matching
{
	if(a<b)
		return 0;
	else 
		return 1;
}
void readFromFile(const char* fileName,int** pics,int** objects,double* matching,int* numOfPics,int* numOfObjects,int** picsDims,int** picsIds,int** objectsDims,int** objectsIds)
{
	FILE* fp;
	if ((fp = fopen(fileName, "r")) == 0) 
	{
		printf("cannot open file %s for reading\n", fileName);
		exit(0);
	}
	int size = 0;
	fscanf(fp, "%lf", matching);//get the matching value
	fscanf(fp, "%d", numOfPics);//get the number of pictures
	*pics = (int*)malloc((*numOfPics)*sizeof(int));
	*picsDims = (int*)malloc((*numOfPics)*sizeof(int));
	*picsIds = (int*)malloc((*numOfPics)*sizeof(int));
	int ind = 0;
	for(int i = 0;i<(*numOfPics);i++)
	{
		int dims;
		int id;
		fscanf(fp, "%d", &id);
		fscanf(fp, "%d", &dims);
		(*picsIds)[i] = id; //get the size and id of each picture
		(*picsDims)[i] = dims;
		size += dims*dims;
		*pics = (int*)realloc(*pics,size*sizeof(int));
		for(int j=0;j<dims*dims;j++)
		{
			int tmp;
			fscanf(fp, "%d", &tmp);//get the picture values
			(*pics)[ind] = tmp;
			ind++;
		}
	}
	fscanf(fp, "%d", numOfObjects);//get the number of objects
	*objects = (int*)malloc((*numOfObjects)*sizeof(int));
	*objectsDims = (int*)malloc((*numOfObjects)*sizeof(int));
	*objectsIds = (int*)malloc((*numOfObjects)*sizeof(int));
	ind = 0;
	for(int i = 0;i<(*numOfObjects);i++)
	{
		int dims;
		int id;
		fscanf(fp, "%d", &id);
		fscanf(fp, "%d", &dims);
		(*objectsIds)[i] = id; //get the size and id of each object
		(*objectsDims)[i] = dims;
		size += dims*dims;
		//printf("%d\n",dims*dims);
		*objects = (int*)realloc(*objects,size*sizeof(int));
		for(int j=0;j<dims*dims;j++)
		{
			int tmp;
			fscanf(fp, "%d", &tmp);//get the object values
			(*objects)[ind] = tmp;
			ind++;
		}
	}
}

int main(int argc, char *argv[]) 
{
	int numOfPics,numOfObjects ,PART;
	double match;
    int* pics;
    int* objects;
    int* picsDims;
    int* picsIds;
    int* objectsDims;
    int* objectsIds;
	int size, rank;
	MPI_Status  status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (size != 2) 
    {
       	printf("Run the example with two processes only\n");
       	MPI_Abort(MPI_COMM_WORLD, __LINE__);
    }
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0)
    {
		readFromFile(FILE_NAME,&pics,&objects,&match,&numOfPics,&numOfObjects,&picsDims,&picsIds,&objectsDims,&objectsIds);//read the input
		PART = numOfPics/2;//split the input between the two processes, send all the inputs to the other process
		MPI_Send(&numOfPics, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
		MPI_Send(&numOfObjects, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
		MPI_Send(&match, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
		MPI_Send(picsDims + PART, numOfPics-PART, MPI_INT, 1, 0, MPI_COMM_WORLD);
		MPI_Send(picsIds + PART, numOfPics-PART, MPI_INT, 1, 0, MPI_COMM_WORLD);
		MPI_Send(objectsDims, numOfObjects, MPI_INT, 1, 0, MPI_COMM_WORLD);
		MPI_Send(objectsIds, numOfObjects, MPI_INT, 1, 0, MPI_COMM_WORLD);
	int offset = 0;
		for(int i = 0;i<PART;i++)
		{
			offset = offset + (picsDims[i]*picsDims[i]);//the size of the first half
		}
		int picsSize = 0;
		for(int i = PART;i<numOfPics;i++)
		{
			
			picsSize = picsSize + (picsDims[i]*picsDims[i]);//the size of the other half
		}
		MPI_Send(pics+offset, picsSize, MPI_INT, 1, 0, MPI_COMM_WORLD);//send the second half of the pictures to the other process
	int objectsSize = 0;
	for(int i = 0;i<numOfObjects;i++)
	{
		objectsSize = objectsSize + objectsDims[i]*objectsDims[i];
	}	
		MPI_Send(objects, objectsSize, MPI_INT, 1, 0, MPI_COMM_WORLD);//send all the objects to the other process
	}
	else 
	{
		MPI_Recv(&numOfPics, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);//recieve the inputs from process 0
		MPI_Recv(&numOfObjects, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&match, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
		if(numOfPics%2 == 0)
			PART = numOfPics/2;
		else
			PART = numOfPics/2 + 1;
		picsDims = (int*)malloc(PART*sizeof(int));
		MPI_Recv(picsDims, PART, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		picsIds = (int*)malloc(PART*sizeof(int));
		MPI_Recv(picsIds, PART, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		objectsDims = (int*)malloc(numOfObjects*sizeof(int));
		MPI_Recv(objectsDims, numOfObjects, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		objectsIds = (int*)malloc(numOfObjects*sizeof(int));
		MPI_Recv(objectsIds, numOfObjects, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		int size = 0;
		for(int i = 0;i<PART;i++)
		{
			size = size + (picsDims[i]*picsDims[i]);
		}
		pics = (int*)malloc(size*sizeof(int**));
		int objectsSize = 0;
	for(int i = 0;i<numOfObjects;i++)
	{
		objectsSize = objectsSize + objectsDims[i]*objectsDims[i];
	}
	objects = (int*)malloc(objectsSize*sizeof(int**));
		MPI_Recv(pics, size, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(objects, objectsSize, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
	}
	int * results = (int*)malloc(sizeof(int)*PART*5);//array to hold the result of the matching, for each picture there is 5 variables (pic id, object id,a boolan variable if found match or not and the coordination of the matching point)
	int countRes = 0;//current number of pictures that we searched for match
	int offsetPic = 0;
#pragma omp parallel for shared(results)
	for(int i =0;i<PART;i++)
	{
		int dim = picsDims[i];//dim of the current pic
	int* tmpPic = (int*)malloc(sizeof(int)*dim*dim);
		int t = 0;
		memcpy(tmpPic,pics+offsetPic,dim*dim*sizeof(int));//tmp array to hold the current pic to send to GPU		
	offsetPic+=dim*dim;
	int offsetObj = 0,found = 0,objectId = 0,row=0,col=0;
	for(int j =0;j<numOfObjects;j++)
	{
		int objectDim = objectsDims[j];
		int* tmpObj = (int*)malloc(sizeof(int)*objectDim*objectDim);//tmp array to hold the current object to send to GPU
		int matchMatrixDim = dim - objectDim + 1;
		double* matchMatrix = (double*)malloc(sizeof(double)*matchMatrixDim*matchMatrixDim);//the results of the matching on the picture
		memset(matchMatrix,0, matchMatrixDim);
		int y = 0;
			memcpy(tmpObj,objects+offsetObj,objectDim*objectDim*sizeof(int));
			if (computeOnGPU(tmpPic, tmpObj,matchMatrix,dim,objectDim,picsIds[i],objectsIds[j],match) != 0)//send the pic and obj to the GPU 
				MPI_Abort(MPI_COMM_WORLD, __LINE__);
			for(int r =0;r<matchMatrixDim;r++)//search the result matrix for a amtch
			{
				for(int t=0;t<matchMatrixDim;t++)
				{
					if(isMatching(matchMatrix[r*matchMatrixDim+t], match) == 0)
					{
						found = 1;
						row = r;
						col = t;
						objectId = objectsIds[j];
						break;//if match found stop searching
					}
				}
				if(found == 1)
					break;				}
			if(found == 1)
				break;
			free(matchMatrix);
			free(tmpObj);
			offsetObj+=objectDim*objectDim;
	}
	//put the result in the result array
	if(found == 1)
	{
		results[countRes*5] = picsIds[i];
		results[countRes*5 +1] = objectId;
		results[countRes*5+2] = found;
		results[countRes*5+3] = row;
		results[countRes*5+4] = col;
		countRes++;
	}
	else
	{
		results[countRes*5] = picsIds[i];
		results[countRes*5 +1] = -1;
		results[countRes*5+2] = found;
		results[countRes*5+3] = -1;
		results[countRes*5+4] = -1;	
		countRes++;
	}
	free(tmpPic);
	}
	if(rank == 0)
	{
		int* allRes = (int*)malloc(5*numOfPics*sizeof(int));//array to hold all the results
		memcpy(allRes,results,PART*5*sizeof(int));//copy the process 0 result to allRes array
		int sizeOfOtherHalf = numOfPics - PART;
		MPI_Recv(allRes + 5*PART, sizeOfOtherHalf*5, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);//get the other half of the result from process 1
		//because of openMP the results array will have the pictures not in order of ids
		sort(&allRes,numOfPics);//sort the array by pictures id, this is for when printing the results we print the pictures in order (pic0 then pic1 then pic2 etc) just to make the input cleaner looking instead of random order
		FILE *fptr = fopen("output.txt", "w");//output file
		if (fptr == NULL)
	 {
			printf("Could not open file");
			 return 0;
	}
		for(int i=0;i<numOfPics;i++)
		{
			if(allRes[i*5+2] == 1)//print the results to the terminal and output file
			{
				printf("Picture %d found object %d in (%d,%d)\n",allRes[i*5],allRes[i*5+1],allRes[i*5+3],allRes[i*5+4]);
				fprintf(fptr,"Picture %d found object %d in (%d,%d)\n",allRes[i*5],allRes[i*5+1],allRes[i*5+3],allRes[i*5+4]);
			}
			else
			{
				fprintf(fptr,"Picture %d found no objects\n",allRes[i*5]);
				printf("Picture %d found no objects\n",allRes[i*5]);
			}
		}
		fclose(fptr);
		free(allRes);
	}
	else
	{//send the result to process 0
		MPI_Send(results,5*PART,MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
	free(pics);
	free(objects);
	free(picsDims);
	free(objectsDims);
	free(objectsIds);
	free(picsIds);	
	MPI_Finalize(); 
	return 0;
}
