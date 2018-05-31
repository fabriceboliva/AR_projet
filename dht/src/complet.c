#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NB_SITES	6
#define M			3 // clee encodee sur M bits
#define K			(int) pow(2, M)
#define KEY			8 // voir le modulo, pour définir ensemble de valeurs
#define NB_MAX_KEY	100

#define TAGINIT		0
#define TAGMIN		1
#define TAGRES		2

///////////////////////////////////////////////////////////////////////////////////////////
//										PROTOTYPES
///////////////////////////////////////////////////////////////////////////////////////////


void recevoir_params(int *nb_voisins, int **voisins, int **voisins_restants, int *valeur);
void calcul_min(int position);

void simulateur();


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

void recevoir_params(int *nb_voisins, int **voisins, int **voisins_restants, int *valeur) 
{
	int i;
	MPI_Status status;
	MPI_Recv(nb_voisins, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

	*voisins = (int *) malloc( (*nb_voisins) * sizeof(int));
	if(*voisins == NULL) {
		perror("malloc - voisins");
		goto error_voisins;
	}

	*voisins_restants = (int *) malloc( (*nb_voisins) * sizeof(int));
	if(*voisins_restants == NULL){
		perror("malloc - voisins_restants");
		goto error_voisins_restants;
	}

	MPI_Recv(*voisins, *nb_voisins, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	MPI_Recv(valeur, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

	for(i=0;i<(*nb_voisins);i++)
		(*voisins_restants)[i] = (*voisins)[i];

	return;

error_voisins_restants:
	free(*voisins);

error_voisins:
	exit(-1);
}

void dht_node(int position) 
{
	int nb_voisins, *voisins, *voisins_restants, valeur, i, recu, j;
	MPI_Status status;

	// reception des parametres
	recevoir_params(&nb_voisins, &voisins, &voisins_restants, &valeur);

	// attente de la réception des messages des voisins
	// si feuille: pas entree dans boucle => pas d'attente
	for(i=0; i<nb_voisins-1; i++){
		MPI_Recv(&recu, 1, MPI_INT, MPI_ANY_SOURCE, TAGMIN, MPI_COMM_WORLD, &status);
		//printf("\n%d: reception du voisin: %d, min_recu: %d\n", position, status.MPI_SOURCE, recu);
		if(recu < valeur)
			valeur = recu;
		//parcours du tableau contenant les voisins restants pour la mise a jour
		for(j=0;j<nb_voisins;j++)
			if(voisins_restants[j] == status.MPI_SOURCE)
				voisins_restants[j] = -1;
	}

	// recherche du voisin restant, pas de recherche pour une feuille
	j=0;
	while(voisins_restants[j] == -1)
		j++;

	// envoi au voisin restant
	MPI_Send(&valeur, 1, MPI_INT, voisins_restants[j], TAGMIN, MPI_COMM_WORLD);

	// attente annonce et mise a jour
	MPI_Recv(&recu, 1, MPI_INT, voisins_restants[j], MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	if(recu < valeur)
		valeur = recu;
	printf("site: %d, min = %d", position, valeur);
	if(status.MPI_TAG == TAGMIN)
		printf(" -> decideur");
	printf("\n");

	// annonce
	for(i=0; i<nb_voisins; i++)
		if(voisins[i] != voisins_restants[j])
			MPI_Send(&valeur, 1, MPI_INT, voisins[i], TAGRES, MPI_COMM_WORLD);

	free(voisins);
	free(voisins_restants);
}

///////////////////////////////////////////////////////////////////////////////////////////
//										SIMULATEUR
///////////////////////////////////////////////////////////////////////////////////////////

static inline int contains(int value, int *table, int length) 
{
	int i;
	for(i = 0; i < length; i++)
		if(table[i] == value)
			return 1;
	return 0;
}

static int new_chord_id(int *table, int length)
{
	int id;
	do {
		id = rand() % NB_MAX_KEY + 1;
	} while(contains(id, table, length));
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

static inline int find_position(int value, int *table, int length) {
	int i, position = -1;
	for(i = 0; i < length; i++)
		if(table[i] == value)
			position = i;
	return position;
}

static inline int calculate_max_position(int *table, int length) 
{
	int i, position, max = table[0];
	for(i = 1; i < length; i++)
		if(table[i] > max)
			position = i;
	return position;
}

static inline int calculate_next(int current, int *table, int length) 
{
	int i, next = K;
	for(i = 0; i < length; i++) {
		if(table[i] > current && table[i] < next)
			next = table[i];
	}
	return next;
}

static inline void calculate_finger_table(int position, int *chord_ids, int *chord_succ, int id_tables_legnth, int *finger_table, int finger_table_length) 
{
	// int i, value = (chord_ids[position] + pow(2, i)) % pow(2, length), jump_id = 0;
	// for(i = 0; i < length; i++)
		// if(chord_ids[i] > value &&  < jump_id)
}


void simulateur() 
{
	int i, j, position;
	
	printf("\nDHT Centralise\n");

	int chord_ids[NB_SITES];
	int chord_ids_sorted[NB_SITES];
	int chord_ids_order[NB_SITES];
	int chord_succ[NB_SITES];
	int finger_tables[NB_SITES][K][2]; // premiere colonne id chord, deuxieme colonne id MPI

	//to del:
	int value;
	
	srand(time(NULL));

	// initialisation des identifiants
	for(i = 0; i < NB_SITES; i++)
		chord_ids[i] = new_chord_id(chord_ids, i);

	
	// tri des identifiants
	for(i = 0; i < NB_SITES; i++)
		chord_ids_sorted[i] = chord_ids[i];
	quickSort(chord_ids_sorted, 0, NB_SITES-1);


	// calcul de l'ordre des ids
	for(i = 0; i < NB_SITES; i++) {
		position = find_position(chord_ids_sorted[i], chord_ids, NB_SITES);
		if(position != -1)
			chord_ids_order[i] = position;
	}

	// calcul des successeurs
	chord_succ[i] = chord_ids[chord_ids_order[0]];
	for(i = 0; i < NB_SITES - 1; i++)
		chord_succ[i] = chord_ids[chord_ids_order[i + 1]];
	


	// calcul des finger tables
	for(i = 0; i < NB_SITES; i++) {
		for(j = 0; j < K; j++) {
			value = (chord_ids[i] + ((int) pow(2, j))) % ((int) pow(2, M));

			// if(chord_ids[i] > value &&  < jump_id)

			// finger_tables[i][j][0] = ;
			// finger_tables[i][j][1] = ;
		}
	}

	printf("chord_ids table: \n");
	int_print_table(chord_ids, NB_SITES);

	printf("chord_ids_sorted table: \n");
	int_print_table(chord_ids_sorted, NB_SITES);
	
	printf("chord_ids_order table: \n");
	int_print_table(chord_ids_order, NB_SITES);

	printf("chord_succ table: \n");
	int_print_table(chord_succ, NB_SITES);

	for(i = 0; i < NB_SITES; i++){
		position = i + 1;

		// printf("envoi a position: %d, chord_ids: %d\n", position, chord_ids[i]);
		// MPI_Send(&chord_ids[i], 1, MPI_INT, position, TAGINIT, MPI_COMM_WORLD);

		// printf("envoi a position: %d, succ_id_chord: %d\n", position, chord_succ[i]);
		// MPI_Send(&chord_succ[i], 1, MPI_INT, position, TAGINIT, MPI_COMM_WORLD);

		// for(j = 0; j < ; j++) {
		// 	// printf("envoi a position: %d, finger_table\n", i);
		// 	MPI_Send(&finger_tables[i][j], 2, MPI_INT, position, TAGINIT, MPI_COMM_WORLD);
		// }
	}

	// programme

	// tirage aleatoire d'un id (while id existe pas) et d'une cle de donnée
	printf("choix\n");

	//envoi message

err:
	return;
}






/******************************************************************************/

int main (int argc, char* argv[]) 
{
	int nb_proc,position;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

	if(nb_proc != NB_SITES+1) {
		printf("Nombre de processus incorrect !\n");
		MPI_Finalize();
		exit(2);
	}

	MPI_Comm_rank(MPI_COMM_WORLD, &position);

	if(position == 0) {
		simulateur();
	} else {
		dht_node(position);
	}

	MPI_Finalize();
	return 0;
}
