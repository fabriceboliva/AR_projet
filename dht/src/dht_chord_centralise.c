#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NB_SITES	10
#define M			6	// cle encodee sur M bits, doit etre inferieur à NB_SITES
#define I			((int) pow(2, M))
#define K			((int) pow(2, M))

#define TAGINIT		0

#define TAGSEARCH	1
#define TAGFOUND	2

#define TAGRES		3
#define TAGEND		4

///////////////////////////////////////////////////////////////////////////////////////////
//										PROTOTYPES
///////////////////////////////////////////////////////////////////////////////////////////

static inline int contains(int value, int *table, int length);
static inline int find_position(int value, int *table, int length);
static inline int find_next_node(int value, int *chord_ids, int length);
static inline int find_position(int value, int *table, int length);


///////////////////////////////////////////////////////////////////////////////////////////
//										UTILITAIRES
///////////////////////////////////////////////////////////////////////////////////////////

void int_print_table(int *table, int length) {
	int i;
	for(i = 0; i < length; i++)
		printf("%d ", table[i]);
	printf("\n");
}


///////////////////////////////////////////////////////////////////////////////////////////
//										PROGRAMME DHT
///////////////////////////////////////////////////////////////////////////////////////////

void receive_params(int *chord_id, int **finger_table_chord_ids, int **finger_table_mpi_ids, int *length) 
{
	MPI_Status status;

	MPI_Recv(chord_id, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	
	MPI_Recv(length, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

	*finger_table_chord_ids = (int *) malloc((*length) * sizeof(int));
	if(finger_table_chord_ids == NULL)
		goto err;

	*finger_table_mpi_ids = (int *) malloc((*length) * sizeof(int));
	if(finger_table_mpi_ids == NULL)
		goto err1;

	MPI_Recv(*finger_table_chord_ids, (*length), MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	MPI_Recv(*finger_table_mpi_ids, (*length), MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

	return;

err1:
	free(finger_table_chord_ids);
	
err:
	exit(-1);
}

static int lookup(int search_data_key, int chord_id, int *finger_table_chord_ids, int length) 
{
	int i;
	
	for(i = length - 1; i >= 0; i--) {
		if(chord_id < finger_table_chord_ids[i]) {
			if(((chord_id >= search_data_key) || (search_data_key > finger_table_chord_ids[i])))
				break;
		} else {
			if(((finger_table_chord_ids[i] < search_data_key) && (search_data_key <= chord_id)))
				break;
		}
	}

	// if(i == -1) {
	// 	//send to the successor
	// 	printf("[CHORD] %d: forward key to successor %d\n", chord_id, finger_table_chord_ids[0]);
	// 	MPI_Send(&search_data_key, 1, MPI_INT, finger_table_mpi_ids[0], TAGFOUND, MPI_COMM_WORLD);
	// } else {
	// 	// send to i 
	// 	printf("[CHORD] %d: forward key to %d\n", chord_id, finger_table_chord_ids[i]);
	// 	MPI_Send(&search_data_key, 1, MPI_INT, finger_table_mpi_ids[i], TAGSEARCH, MPI_COMM_WORLD);
	// }

	

	return i;
}

void dht_node(int mpi_id) 
{
	int chord_id, *finger_table_chord_ids, *finger_table_mpi_ids, length;
	int search_data_key;
	int found, end;
	MPI_Status status;

	// reception des parametres
	receive_params(&chord_id, &finger_table_chord_ids, &finger_table_mpi_ids, &length);

	// printf("chord_id: %d; chord_succ: %d\n", chord_id, chord_succ);

	// printf("site %d: finger table1:", position);
	// int_print_table(finger_table_chord_ids, length);

	// printf("site %d: finger table2:", position);
	// int_print_table(finger_table_mpi_ids, length);
	
	// printf("ok\n");

	// reception de la requete de recherche
	end = 0;
	while(!end) {
		MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		switch(status.MPI_TAG) {
		case TAGSEARCH:
			MPI_Recv(&search_data_key, 1, MPI_INT, MPI_ANY_SOURCE, TAGSEARCH, MPI_COMM_WORLD, &status);
			printf("[CHORD] site %d: received search request of key %d\n", chord_id, search_data_key);
			found = lookup(search_data_key, chord_id, finger_table_chord_ids, length);
			if(found == -1) {
				//send to the successor
				printf("[CHORD] %d: forward key to successor %d\n", chord_id, finger_table_chord_ids[0]);
				MPI_Send(&search_data_key, 1, MPI_INT, finger_table_mpi_ids[0], TAGFOUND, MPI_COMM_WORLD);
			} else {
				// send to i 
				printf("[CHORD] %d: forward key to %d\n", chord_id, finger_table_chord_ids[found]);
				MPI_Send(&search_data_key, 1, MPI_INT, finger_table_mpi_ids[found], TAGSEARCH, MPI_COMM_WORLD);
			}
			break;
		case TAGFOUND:
			MPI_Recv(&search_data_key, 1, MPI_INT, MPI_ANY_SOURCE, TAGFOUND, MPI_COMM_WORLD, &status);
			printf("[CHORD] site %d: found key %d\n", chord_id, search_data_key);
			MPI_Send(&chord_id, 1, MPI_INT, 0, TAGRES, MPI_COMM_WORLD);
			break;
		case TAGEND:
			MPI_Recv(NULL, 0, MPI_INT, 0, TAGEND, MPI_COMM_WORLD, &status);
			end = 1;
			break;
		}
	}

	free(finger_table_chord_ids);
	free(finger_table_mpi_ids);
}

///////////////////////////////////////////////////////////////////////////////////////////
//										SIMULATEUR
///////////////////////////////////////////////////////////////////////////////////////////

static inline int contains(int value, int *table, int length) 
{
	int i;
	for(i = 0; i < length; i++)
		if(table[i] == value)
			return i;
	return -1;
}

static int new_chord_id(int *table, int length)
{
	int id;
	do {
		id = rand() % I;
	} while(contains(id, table, length) != -1);
	return id;
}

void swap(int *intArray, int num1, int num2) {
   int temp = intArray[num1];
   intArray[num1] = intArray[num2];
   intArray[num2] = temp;
}

int partition(int *intArray, int left, int right, int pivot) {
   int leftPointer = left -1;
   int rightPointer = right;

   while(1) {
      while(intArray[++leftPointer] < pivot) {
         //do nothing
      }
		
      while(rightPointer > 0 && intArray[--rightPointer] > pivot) {
         //do nothing
      }

      if(leftPointer >= rightPointer) {
         break;
      } else {
         swap(intArray, leftPointer,rightPointer);
      }
   }
	
   
   swap(intArray, leftPointer, right);
   return leftPointer;
}

void quickSort(int *intArray, int left, int right) {
   if(right-left <= 0) {
      return;   
   } else {
      int pivot = intArray[right];
      int partitionPoint = partition(intArray, left, right, pivot);
      quickSort(intArray, left,partitionPoint-1);
      quickSort(intArray, partitionPoint+1,right);
   }        
}

static inline int find_position(int value, int *table, int length) 
{
	int i, position = -1;
	for(i = 0; i < length; i++)
		if(table[i] == value)
			position = i;
	return position;
}

static inline int find_next_node(int value, int *chord_ids, int length) 
{
	int i;
	for(i = 0; i < length; i++)
		if(chord_ids[i] >= value)
			return i;
	return -1;
}

		// 	position = find_next_node(value, chord_ids, NB_SITES);
		// 	if(position == -1) {
		// 		finger_tables[i][0][j] = chord_ids[0];
		// 		finger_tables[i][1][j] = 1; // car rang MPI
		// 	} else {
		// 		finger_tables[i][0][j] = chord_ids[position];
		// 		finger_tables[i][1][j] = position + 1; // car rang MPI
		// 	}
		// }
		// printf("[CHORD] %d: finger table: ", chord_ids[i]); // id_mpi i+
		// int_print_table(finger_tables[i][0], M);
		// // int_print_table(finger_tables[i][1], M); // rang MPI
		// printf("\n");

static void calculate_finger_table(int *chord_ids, int ***finger_tables, int chord_id_position) {
	int value, position, j;
	for(j = 0; j < M; j++) {
		value = (chord_ids[chord_id_position] + ((int) pow(2, j))) % ((int) pow(2, M));
		position = find_next_node(value, chord_ids, NB_SITES);
		if(position == -1) {
			finger_tables[chord_id_position][0][j] = chord_ids[1];
			finger_tables[chord_id_position][1][j] = 1;
		} else {
			finger_tables[chord_id_position][0][j] = chord_ids[position];
			finger_tables[chord_id_position][1][j] = position + 1; // car rang MPI
		}
	}
	printf("site %d: finger table:\n", chord_id_position+1);
	int_print_table(finger_tables[chord_id_position][0], M);
	int_print_table(finger_tables[chord_id_position][1], M);
	printf("\n");
}


void simulateur() 
{
	int i, j, position, temp;
	int search_chord_id, search_data_key;
	MPI_Status status;
	int chord_id_responsible;

	printf("\nDHT Centralise\n");


	// data
	int chord_ids[NB_SITES];
	int finger_tables[NB_SITES][2][M]; // premiere colonne id chord, deuxieme colonne id MPI


	//to del:
	int value;
	
	srand(time(NULL));

	// initialisation des identifiants
	for(i = 0; i < NB_SITES; i++)
		chord_ids[i] = new_chord_id(chord_ids, i);

	
	// tri des identifiants
	quickSort(chord_ids, 0, NB_SITES-1);

	printf("chord_ids table: \n");
	int_print_table(chord_ids, NB_SITES);
	printf("\n");

	// int valeurs[M];

	// pour chaque site, calcul des finger tables
	for(i = 0; i < NB_SITES; i++) {

		for(j = 0; j < M; j++) {
			value = (chord_ids[i] + ((int) pow(2, j))) % ((int) pow(2, M));
			// valeurs[i] = value;
			position = find_next_node(value, chord_ids, NB_SITES);
			if(position == -1) {
				finger_tables[i][0][j] = chord_ids[0];
				finger_tables[i][1][j] = 1; // car rang MPI
			} else {
				finger_tables[i][0][j] = chord_ids[position];
				finger_tables[i][1][j] = position + 1; // car rang MPI
			}
		}
		printf("[CHORD] %d: finger table: ", chord_ids[i]); // id_mpi i+
		int_print_table(finger_tables[i][0], M);
		// int_print_table(finger_tables[i][1], M); // rang MPI
		printf("\n");
	}

	for(i = 0; i < NB_SITES; i++){
		position = i + 1;

		// printf("envoi a position: %d, chord_ids: %d\n", position, chord_ids[i]);
		MPI_Send(&chord_ids[i], 1, MPI_INT, position, TAGINIT, MPI_COMM_WORLD);

		temp = M;

		// printf("envoi a position: %d, succ_id_chord: %d\n", position, chord_succ[i]);
		MPI_Send(&temp, 1, MPI_INT, position, TAGINIT, MPI_COMM_WORLD);

		MPI_Send(&finger_tables[i][0], M, MPI_INT, position, TAGINIT, MPI_COMM_WORLD);
		MPI_Send(&finger_tables[i][1], M, MPI_INT, position, TAGINIT, MPI_COMM_WORLD);
	}



	// programme

	// tirage aleatoire d'un id (while id existe pas) et d'une cle de donnée
	printf("choix\n");
	do {
		search_chord_id = rand() % I + 1;
	} while(contains(search_chord_id, chord_ids, NB_SITES) == -1);

	search_data_key = rand() % I + 1;

	position = find_position(search_chord_id, chord_ids, NB_SITES) + 1;

	printf("[SIMULATOR] start search: key:%d site:%d\n", search_data_key, chord_ids[position - 1]);
	// envoi recherche
	MPI_Send(&search_data_key, 1, MPI_INT, position, TAGSEARCH, MPI_COMM_WORLD);

	// attente réponse
	MPI_Recv(&chord_id_responsible, 1, MPI_INT, MPI_ANY_SOURCE, TAGRES, MPI_COMM_WORLD, &status);
	printf("[SIMULATOR] end search: key:%d site_id:%d\n", search_data_key, chord_id_responsible);

	// envoi terminaison
	for(i = 0; i < NB_SITES; i++){
		position = i + 1;
		MPI_Send(NULL, 0, MPI_INT, position, TAGEND, MPI_COMM_WORLD);
	}
}


/******************************************************************************/

int main (int argc, char* argv[]) 
{
	int nb_proc, mpi_id;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

	if(nb_proc != NB_SITES+1) {
		printf("Nombre de processus incorrect !\n");
		MPI_Finalize();
		exit(2);
	}

	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_id);

	if(mpi_id == 0) {
		simulateur();
	} else {
		dht_node(mpi_id);
	}

	MPI_Finalize();
	return 0;
}
