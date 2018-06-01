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

#define TAGMSG	1
#define TAGFOUND	2

#define TAGRES		3
#define TAGEND		4




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

void receive_params(int *chord_id, int *length, int *gauche, int *droite, int *init) 
{
	MPI_Status status;

	MPI_Recv(chord_id, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	
	MPI_Recv(length, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

	MPI_Recv(gauche, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	
	MPI_Recv(droite, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

	MPI_Recv(init, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	return;

}

void receive_msg(int *rang,int *winner,int *state,int *leader,int *cpt,int *chord_id, int *length, int *gauche, int *droite, int *init, int *msg, int *msg_recu) 
{
	MPI_Status status;
	int new[NB_SITES+1];
	int i;
	MPI_Recv(msg_recu, NB_SITES+1, MPI_INT, MPI_ANY_SOURCE, TAGMSG, MPI_COMM_WORLD, &status);
	if(msg_recu[NB_SITES] == *rang){
		if(msg_recu[NB_SITES+1] > 0)
			*winner = 1;
	}
	if(msg_recu[NB_SITES] > *rang && msg_recu[NB_SITES+1] >= 1){
		*state = 0;
		*leader = msg_recu[NB_SITES];
		for(i=0;i<NB_SITES+2;i++){
			new[i] = msg[i];
		}
		for(i=0;i<NB_SITES+2;i++){
			if(msg_recu[i] != NULL)
				new[i] = msg_recu[i];
		}
		if(msg_recu[NB_SITES+1] > 1){
			new[NB_SITES+1] = msg_recu[NB_SITES+1]-1;
			MPI_Send
		}
		else{

		}
	}
	if(msg_recu[NB_SITES+1] == 0){
		if(msg_recu){

		}
		else{

		}
	}
	MPI_Recv(msg_recu, NB_SITES+1, MPI_INT, MPI_ANY_SOURCE, TAGMSG, MPI_COMM_WORLD, &status);

	return;

}

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
		id = rand() % I + 1;
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

void simulateur() 
{
	int i, j, position, temp;
	int search_chord_id, search_data_key;
	MPI_Status status;
	int chord_id_responsible;
	int gauche,droite,init;
	int nb_init;
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


	printf("chord_ids table: \n");
	int_print_table(chord_ids, NB_SITES);
	printf("\n");

	nb_init = (rand() % NB_SITES) + 1;

	for(i = 0; i < NB_SITES; i++){
		position = i + 1;
		if(position==1)
			gauche = NB_SITES;
		else{
			gauche = position-1 ;
		}
		droite = (position+1)%NB_SITES; 
		//déterminer init
		if(nb_init != 0){
			init = (rand() % 2); 
		}
		// printf("envoi a position: %d, chord_ids: %d\n", position, chord_ids[i]);
		MPI_Send(&chord_ids[i], 1, MPI_INT, position, TAGINIT, MPI_COMM_WORLD);

		temp = M;

		// printf("envoi a position: %d, succ_id_chord: %d\n", position, chord_succ[i]);
		MPI_Send(&temp, 1, MPI_INT, position, TAGINIT, MPI_COMM_WORLD);

		//envoie voisin MPI gauche
		MPI_Send(&gauche, 1, MPI_INT, position, TAGINIT, MPI_COMM_WORLD);

		//envoie voisin MPI droite
		MPI_Send(&droite, 1, MPI_INT, position, TAGINIT, MPI_COMM_WORLD);

		//envoie voisin MPI droite
		MPI_Send(&init, 1, MPI_INT, position, TAGINIT, MPI_COMM_WORLD);

	}


}


void calcul_finger(int rang)
{
	int chord_id, *finger_table_chord_ids, *finger_table_mpi_ids, length;
	int mpi_gauche,mpi_droite;
	int state;
	int search_data_key;
	int found, end;
	MPI_Status status;
	int msg[NB_SITES+1];
	int msg_recu[NB_SITES+1];
	int winner,round;
	int leader;
	int cpt;
	//printf("\nCalcul finger_tables\n");

	// reception des parametres
	receive_params(&chord_id,&length,&mpi_gauche,&mpi_droite,&state);
	leader = rang;
	round = 0;
	msg[rang] = chord_id;
	msg[NB_SITES] = rang;
	msg[NB_SITES+1] = (int) pow(2,round);
	if(state == 1){
		printf("mpi %d chord id %d gauche %d droite %d init\n",rang,chord_id,mpi_gauche,mpi_droite);
		MPI_Send(msg, NB_SITES+1, MPI_INT, mpi_gauche, TAGMSG, MPI_COMM_WORLD);
		MPI_Send(msg, NB_SITES+1, MPI_INT, mpi_droite, TAGMSG, MPI_COMM_WORLD);
		round++;
		cpt = 0;
	}
	receive_msg(&rang,&winner,&state,&leader,&cpt,&chord_id,&length,&mpi_gauche,&mpi_droite,&state,msg,msg_recu);


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
		calcul_finger(position);
	}

	MPI_Finalize();
	return 0;
}