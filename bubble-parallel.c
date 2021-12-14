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
	MPI_Status status; // status sobre o envio de informações entre processos ( fonte, tipo de dados, erros)

	MPI_Init(&argc,&argv); // inicializa o ambiente de comunicação
	MPI_Comm_rank(MPI_COMM_WORLD,&id); // determina o id do processo no comunicador
	MPI_Comm_size(MPI_COMM_WORLD,&numProcs); // determina o numero de processos no comunicador

	if(id == 0){
        // int vet[] = {-2, 45, 0, 11, -9, 10, 23};
        // vetSize = sizeof(vet) / sizeof(vet[0]);

		printf("size of the array: ");
		fflush(stdout);
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
        MPI_Scatter(data,chunkSize,MPI_INT,chunk,chunkSize,MPI_INT,0,MPI_COMM_WORLD); // divide o array principal em pedaços pada cada processo

        bubblesort(chunk,chunkSize);
    }else{
        MPI_Bcast(&chunkSize,1,MPI_INT,0,MPI_COMM_WORLD); // processos subordinados recebem o tamanho do array do processo principal
        chunk = (int *)malloc(chunkSize*sizeof(int));
        MPI_Scatter(NULL,0,NULL,chunk,chunkSize,MPI_INT,0,MPI_COMM_WORLD); // processos subordinados recebem o array dividido pelo processo principal

        bubblesort(chunk,chunkSize);
    }

	// step = 1;
	// while(step < numProcs){
	// 	if(id % (2 * step) != 0){ // id 0 nunca entra nesse if, apenas ids ímpares no primeiro step e ids pares nos steps > 1
	// 		int destinationId = id-step;
	// 		MPI_Send(&chunkSize,1,MPI_INT,destinationId,0,MPI_COMM_WORLD); // envia o tamanho do pedaço do array ordenado para o processo destinatário
	// 		MPI_Send(chunk,chunkSize,MPI_INT,destinationId,0,MPI_COMM_WORLD); // envia os valores do pedaço do array ordenado para o processo destinatário
	// 		break;
	// 	}

    //     if(id + step < numProcs){ // id 0 sempre entra nesse if e ids pares esperam receber dos ids impares no primeiro step
    //         MPI_Recv(&chunkAuxSize,1,MPI_INT,id+step,0,MPI_COMM_WORLD,&status); // espera receber o tamanho do pedaço do array ordenado do processo de origem
    //         chunkAux = (int *)malloc(chunkAuxSize*sizeof(int));

    //         MPI_Recv(chunkAux,chunkAuxSize,MPI_INT,id+step,0,MPI_COMM_WORLD,&status); // espera receber os valores do pedaço do array ordenado do processo de origem
    //         chunk = merge(chunk,chunkSize,chunkAux,chunkAuxSize); // junta os pedaços do array ordenado no array principal
    //         chunkSize = chunkSize+chunkAuxSize;
    //     } 

    //     step = step*2;
	// }

	int chunkSizeMain = chunkSize;

	if (id != 0) {
		MPI_Send(chunk,chunkSize,MPI_INT,0,0,MPI_COMM_WORLD);
	} else {
		chunkAux = (int *)malloc(chunkSize*sizeof(int));
		
		for(int i = 1; i < numProcs; i++){
			MPI_Recv(chunkAux,chunkSize,MPI_INT,i,0,MPI_COMM_WORLD,&status);
			chunk = merge(chunk,chunkSizeMain,chunkAux,chunkSize);
        	chunkSizeMain = chunkSize+chunkSizeMain;
		}
	}

	if(id==0){
		stopT = clock();

		for(i=0;i<chunkSizeMain;i++) 
            if(chunk[i] != __INT_MAX__) printf(" %d;",chunk[i]);

  		printf("\n");

		printf("array size: %d; %d processors; %f secs\n",vetSize,numProcs,(stopT-startT)/CLOCKS_PER_SEC);
	}
	MPI_Finalize();
}