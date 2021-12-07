#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

int * merge(int *v1, int size1, int *v2, int size2);
void swap(int *v, int i, int j);
void bubblesort(int *v, int n);

double startT,stopT;

int * merge(int *v1, int size1, int *v2, int size2){
	int i,j,k;
	int * result;

	result = (int *)malloc((size1+size2)*sizeof(int));

	i=0; j=0; k=0;
	while(i < size1 && j < size2){
		if(v1[i] < v2[j]){
			result[k] = v1[i];
			i++;
		}
		else{
			result[k] = v2[j];
			j++;
		} 
        k++;
    }
	if(i == size1){
		while(j < size2){
			result[k] = v2[j];
			j++; 
            k++;
		}
    }
	else{
		while(i < size1){
			result[k] = v1[i];
			i++;
            k++;
		}
    }

	return result;
}

void swap(int *v, int i, int j){
	int t;
	t = v[i];
	v[i] = v[j];
	v[j] = t;
}

void bubblesort(int *v, int n){
	int i,j;
	for(i=n-2;i>=0;i--)
		for(j=0;j<=i;j++)
			if(v[j]>v[j+1])
				swap(v,j,j+1);
}

void main(int argc, char **argv){
	int * data;
	int * chunk;
	int * chunkAux;
	int chunkAuxSize,vetSize;
	int id,numProcs;
	int chunkSize;
	int i;  
	int step;
    int rest;
	MPI_Status status;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&id);
	MPI_Comm_size(MPI_COMM_WORLD,&numProcs);

	if(id == 0){
        // int vet[] = {-2, 45, 0, 11, -9, 10, 23};
        // vetSize = sizeof(vet) / sizeof(vet[0]);

		printf("size of the array: ");
		scanf("%i", &vetSize);
		chunkSize = vetSize/numProcs;
		rest = vetSize%numProcs;

		data = (int *)malloc((vetSize+numProcs-rest)*sizeof(int));

        for(i = 0; i < vetSize; i++)
			data[i] = random() % 1000;

		// for(i=0;i<vetSize;i++)
		// 	data[i] = vet[i];

		if(rest!=0){
			for(i=vetSize;i<vetSize+numProcs-rest;i++) data[i]=__INT_MAX__;
			chunkSize=chunkSize+1;
		}

		startT = clock();
        
        MPI_Bcast(&chunkSize,1,MPI_INT,0,MPI_COMM_WORLD); //comunica do processo principal para todos os processos o tamanho de uma parte do array
        chunk = (int *)malloc(chunkSize*sizeof(int));
        MPI_Scatter(data,chunkSize,MPI_INT,chunk,chunkSize,MPI_INT,0,MPI_COMM_WORLD); //comunica de cada processo para todos os processos o array principal e o array em pedaço

        bubblesort(chunk,chunkSize);
    }else{
        MPI_Bcast(&chunkSize,1,MPI_INT,0,MPI_COMM_WORLD);
        chunk = (int *)malloc(chunkSize*sizeof(int));
        MPI_Scatter(NULL,0,MPI_PROC_NULL,chunk,chunkSize,MPI_INT,0,MPI_COMM_WORLD); // aqui pode ser null pois é necessário ser feito apenas pelo processo principal

        bubblesort(chunk,chunkSize);
    }

	step = 1;
	while(step < numProcs){
		if(id % (2 * step) != 0){
			int destinationId = id-step;
			MPI_Send(&chunkSize,1,MPI_INT,destinationId,0,MPI_COMM_WORLD); //envia o tamanho do pedaço do array ordenado para o processo destinatário
			MPI_Send(chunk,chunkSize,MPI_INT,destinationId,0,MPI_COMM_WORLD); //envia os valores do pedaço do array ordenado para o processo destinatário
			break;
		}

        if(id + step < numProcs){
            MPI_Recv(&chunkAuxSize,1,MPI_INT,id+step,0,MPI_COMM_WORLD,&status); // espera receber o tamanho do pedaço do array ordenado do processo de origem
            chunkAux = (int *)malloc(chunkAuxSize*sizeof(int));

            MPI_Recv(chunkAux,chunkAuxSize,MPI_INT,id+step,0,MPI_COMM_WORLD,&status); // espera receber os valores do pedaço do array ordenado do processo de origem
            chunk = merge(chunk,chunkSize,chunkAux,chunkAuxSize); // junta os pedaços do array ordenado no array principal
            chunkSize = chunkSize+chunkAuxSize;
        } 

        step = step*2;
	}

	if(id==0){
		stopT = clock();
		printf("array size: %d; %d processors; %f secs\n",vetSize,numProcs,(stopT-startT)/CLOCKS_PER_SEC);

		// for(i=0;i<chunkSize;i++) 
        //     if(chunk[i] != __INT_MAX__) printf(" %d;",chunk[i]);
  		// printf("\n");
	}
	MPI_Finalize();
}