#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NB_SITES	10
#define M			6 // clee encodee sur M bits, doit etre inferieur à NB_SITES
#define I			((int) pow(2, M))
#define K			((int) pow(2, M))

#define TAGINIT		0
#define TAGSEARCH	1
#define TAGRES		2

///////////////////////////////////////////////////////////////////////////////////////////
//										PROTOTYPES
///////////////////////////////////////////////////////////////////////////////////////////

static inline int contains(int value, int *table, int length);
static inline int find_position(int value, int *table, int length);
static inline int find_next_node(int value, int *chord_ids, int length);


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

void receive_params(int *chord_id, int *chord_succ, int **finger_table_chord_ids, int **finger_table_mpi_ids, int *length) 
{
	int i;
	MPI_Status status;

	MPI_Recv(chord_id, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	MPI_Recv(chord_succ, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	MPI_Recv(length, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

	*finger_table_chord_ids = (int *) malloc((*length) * sizeof(int));
	*finger_table_mpi_ids = (int *) malloc((*length) * sizeof(int));

	// for (i = 0; i < ; i++) {
	// 	finger_table[i] = (int *) malloc( (*length) * sizeof(int));
	

	// for(i = 0; i < (*length); i++)
	MPI_Recv(*finger_table_chord_ids, (*length), MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	if(finger_table_chord_ids == NULL)
			goto err;

	MPI_Recv(*finger_table_mpi_ids, (*length), MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	if(finger_table_chord_ids == NULL)
			goto err1;

	return;

err1:
	// todo free of other allocated sub-tables
	free(finger_table_chord_ids);
	
err:
	exit(-1);
}

static int lookup(int search_data_key, int chord_id, int *finger_table_chord_ids, int *finger_table_mpi_ids, int length) {
	int found, i;
	if(search_data_key > chord_id) { // si la donnée que 

	}

	if( chord_id > finger_table_chord_ids[length - 1] ) { // exemple: 13 à la fin de la ft de 42 et on recherche 48 qui se trouve en haut
		
	} else { // cas normal
		found = -1;
		for(i = length - 1; (found == -1) (i >= 0); i++)
			if(finger_table_chord_ids[i] <= search_data_key)
				found = i;
	}

	return 0;
}

void dht_node(int mpi_id) 
{
	int chord_id, chord_succ, *finger_table_chord_ids, *finger_table_mpi_ids, length, i, recu, j;
	int search_chord_id, search_mpi_id, search_data_key, search_position;
	MPI_Status status;

	// reception des parametres
	receive_params(&chord_id, &chord_succ, &finger_table_chord_ids, &finger_table_mpi_ids, &length);

	// printf("chord_id: %d; chord_succ: %d\n", chord_id, chord_succ);

	// printf("site %d: finger table1:", position);
	// int_print_table(finger_table_chord_ids, length);

	// printf("site %d: finger table2:", position);
	// int_print_table(finger_table_mpi_ids, length);
	
	// printf("site %d: finger table1:", position);
	// int_print_table(finger_table_chord_ids, length);
	
	// reception de la requete de recherche
	MPI_Recv(&search_data_key, 1, MPI_INT, 0, TAGSEARCH, MPI_COMM_WORLD, &status);



	//look in the finger table
	search_position = find_next_node(search_data_key, finger_table_chord_ids, length);
	if(search_position == -1) {
		search_chord_id = finger_table_chord_ids[0];
	} else {
		search_chord_id = finger_table_chord_ids[search_position];
	}
	search_mpi_id = contains(search_chord_id, finger_table_chord_ids, length);

	printf("site %d: %d %d %d\n", mpi_id, search_data_key, search_chord_id, search_mpi_id);


	// // attente de la réception des messages des voisins
	// // si feuille: pas entree dans boucle => pas d'attente
	// for(i=0; i<nb_voisins-1; i++){
	// 	MPI_Recv(&recu, 1, MPI_INT, MPI_ANY_SOURCE, TAGMIN, MPI_COMM_WORLD, &status);
	// 	//printf("\n%d: reception du voisin: %d, min_recu: %d\n", position, status.MPI_SOURCE, recu);
	// 	if(recu < valeur)
	// 		valeur = recu;
	// 	//parcours du tableau contenant les voisins restants pour la mise a jour
	// 	for(j=0;j<nb_voisins;j++)
	// 		if(voisins_restants[j] == status.MPI_SOURCE)
	// 			voisins_restants[j] = -1;
	// }

	// // recherche du voisin restant, pas de recherche pour une feuille
	// j=0;
	// while(voisins_restants[j] == -1)
	// 	j++;

	// // envoi au voisin restant
	// MPI_Send(&valeur, 1, MPI_INT, voisins_restants[j], TAGMIN, MPI_COMM_WORLD);

	// // attente annonce et mise a jour
	// MPI_Recv(&recu, 1, MPI_INT, voisins_restants[j], MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	// if(recu < valeur)
	// 	valeur = recu;
	// printf("site: %d, min = %d", position, valeur);
	// if(status.MPI_TAG == TAGMIN)
	// 	printf(" -> decideur");
	// printf("\n");

	// // annonce
	// for(i=0; i<nb_voisins; i++)
	// 	if(voisins[i] != voisins_restants[j])
	// 		MPI_Send(&valeur, 1, MPI_INT, voisins[i], TAGRES, MPI_COMM_WORLD);

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

	// pour chaque site, calcul des finger tables
	for(i = 0; i < NB_SITES; i++) {

		for(j = 0; j < M; j++) {
			value = (chord_ids[i] + ((int) pow(2, j))) % ((int) pow(2, M));
			position = find_next_node(value, chord_ids, NB_SITES);
			if(position == -1) {
				finger_tables[i][0][j] = chord_ids[1];
				finger_tables[i][1][j] = 1;
			} else {
				finger_tables[i][0][j] = chord_ids[position];
				finger_tables[i][1][j] = position + 1; // car rang MPI
			}
		}
		printf("site %d: finger table:\n", i+1);
		int_print_table(finger_tables[i][0], M);
		int_print_table(finger_tables[i][1], M);
		printf("\n");
	}

	for(i = 0; i < NB_SITES; i++){
		position = i + 1;

		// printf("envoi a position: %d, chord_ids: %d\n", position, chord_ids[i]);
		MPI_Send(&chord_ids[i], 1, MPI_INT, position, TAGINIT, MPI_COMM_WORLD);

		// printf("envoi a position: %d, succ_id_chord: %d\n", position, chord_succ[i]);
		MPI_Send(&chord_ids[(i+1)%NB_SITES], 1, MPI_INT, position, TAGINIT, MPI_COMM_WORLD);

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

	//envoi message
	MPI_Send(&search_data_key, 1, MPI_INT, position, TAGSEARCH, MPI_COMM_WORLD);

	// attente réponse


err:
	return;
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
