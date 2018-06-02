#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#define NB_SITES	10
#define M			6	// cle encodee sur M bits, doit etre inferieur à NB_SITES
#define I			((int) pow(2, M))
#define K			((int) pow(2, M))

#define TAGINIT		0

#define TAGMSG	1
#define TAGFOUND	2

#define TAGRES		3
#define TAGEND		4

#define MAXNUMBER 100




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

static void print_initial_data(int *chord_ids, int finger_tables[NB_SITES][2][M]) {
	int i;
	for(i = 0; i < NB_SITES; i++) {
		printf("[CHORD] peer:%d\t| finger_table: ", chord_ids[i]); // id_mpi i+
		int_print_table(finger_tables[i][0], M);
		// int_print_table(finger_tables[i][1], M); // rang MPI
		// printf("\n");
	}
	printf("\n");
}

static void calculate_finger_table(int *chord_ids, int finger_tables[NB_SITES][2][M], int chord_id_position) {
	int value, mpi_id, j;
	for(j = 0; j < M; j++) {
		value = (chord_ids[chord_id_position] + ((int) pow(2, j))) % ((int) pow(2, M));
		mpi_id = find_next_node(value, chord_ids, NB_SITES);
		if(mpi_id == -1) {
			finger_tables[chord_id_position][0][j] = chord_ids[0];
			finger_tables[chord_id_position][1][j] = 1; // car rang MPI
		} else {
			finger_tables[chord_id_position][0][j] = chord_ids[mpi_id];
			finger_tables[chord_id_position][1][j] = mpi_id + 1; // car rang MPI
		}
	}
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

void receive_msg(int *rang,int *winner,int *state,int *leader,int *cpt,int *chord_id, int *length, int *gauche, int *droite,int *msg, int *taille_locale,int *msg_recu) 
{
	MPI_Status status;
	int *new;
	int i;
	int j=0;
	int direction,direction_inverse;
	int number_amount;
	int max;
	MPI_Recv(msg_recu, MAXNUMBER, MPI_INT, MPI_ANY_SOURCE, TAGMSG, MPI_COMM_WORLD, &status);
	MPI_Get_count(&status,MPI_INT,&number_amount);
	printf("%d\n",number_amount );
	if(status.MPI_SOURCE == *gauche){
			direction = *droite;
			direction_inverse = *gauche;
		}
	else{
			direction = *gauche;
			direction_inverse = *droite;
	}
	switch(status.MPI_TAG){
		case TAGMSG :
			if(*rang >= number_amount-2){
					new = malloc(sizeof(int) * (*rang+3));
					max = (*rang)+3;
				}
				else{
					new = malloc(sizeof(int) * number_amount);
					max = number_amount;
				}
				for(i=0;i<max-1;i++){
					new[i] = 0;
				}
				
				
				printf("%d : recu de %d avec TTL %d number_amount %d\n",*rang,status.MPI_SOURCE,msg_recu[number_amount-1],number_amount );

				if(msg_recu[number_amount-2] == *rang){
					if(msg_recu[number_amount-1] > 0){
						*winner = 1;
						printf("winner\n");
						return ;
					}
				}
					//printf("---TTL %d de rang %d au rang %d\n",msg_recu[number_amount-1],msg_recu[number_amount-2],*rang);
					if(
						(msg_recu[number_amount-2] > *rang && msg_recu[number_amount-1] >= 1)
						|| (*state == 0 && msg_recu[number_amount-1] >=1)
						){
						*state = 0;
						*leader = msg_recu[number_amount-2];
						//for(i=0;i<*taille_locale-2;i++){
							new[*rang] = msg[*rang];
						//}			

						// for(j = 0; j < *taille_locale-2; j++){
						// printf("%d ", new[j]);
						// }	
						// printf("\n%d taille %d\n",*rang,*taille_locale);

						for(i=0;i<number_amount-2;i++){
							if(msg_recu[i] != 0)
								new[i] = msg_recu[i];
						}
						new[max-2] = msg_recu[number_amount-2];
						//new[max-1] = msg_recu[number_amount-1];
						// for(j= 0; j < max; j++){
						// printf("%d ", new[j]);
						// }	
						// printf("%d max %d \n",*rang,max);

						//printf("TTL %d\n",msg_recu[number_amount-1] );
						if(msg_recu[number_amount-1] > 1){
							new[max-1] = msg_recu[number_amount-1]-1;
							// msg = realloc(msg,max);
							// for(i=0;i<max;i++){
							// 	msg[i] = new[i];
							// }
							MPI_Send(new,max,MPI_INT,direction,TAGMSG,MPI_COMM_WORLD);
							printf("%d forward to %d\n", *rang,direction);
						}
						else{
							new[max-1] = 0;
							MPI_Send(new,max,MPI_INT,direction_inverse,TAGMSG,MPI_COMM_WORLD);
							printf("TTL 0 envoie done %d à %d\n", *rang,direction_inverse);
						}

						
						
					}
				if(msg_recu[number_amount-1] == 0){
					if(msg_recu[number_amount-2] != *rang){
						MPI_Send(msg_recu,number_amount,MPI_INT,direction,TAGMSG,MPI_COMM_WORLD);
						printf("Initiateur %d TTL%d source%d rang%d trasit à %d\n",msg_recu[number_amount-2],msg_recu[number_amount-1],status.MPI_SOURCE,*rang,direction );

						// for(j= 0; j < number_amount; j++){
						// printf("%d ", msg_recu[j]);
						// }	
						// printf("message de retour %d max %d \n",*rang,max);
					}
					else{
						if(number_amount <= *taille_locale){ //retour de cote inf
							for(i=0;i<number_amount-2;i++){
								if(msg_recu[i] != 0)
									msg[i] = msg_recu[i];
							}
						}
						else{ //cote sup
							new[max-2] = msg_recu[number_amount-2];
							new[max-1] = msg_recu[number_amount-1];
							for(i=0;i<number_amount-2;i++){
								if(msg_recu[i] != 0)
									new[i] = msg_recu[i];
							}
							msg = realloc(msg,max);
							msg[*taille_locale-2] = 0;
							msg[*taille_locale-1] = 0;
							for(i=0;i<number_amount;i++){
								if(msg_recu[i] != 0)
									msg[i] = new[i];
							}
							if(*taille_locale < number_amount){
								*taille_locale = number_amount;
							}
							printf("aggrandit %d %d\n",number_amount,max);
						}
							
						//printf("\n%d taille %d\n",*rang,*taille_locale);
						*cpt=*cpt+1;
						printf("%d : Arrivé sur init source %d\n",*rang,status.MPI_SOURCE);
					}
				}
					
				free(new);
		// case TAGFOUND :

		// 			if(msg_recu[number_amount-2] == *rang){
						
		// 				printf("Msg transmis\n");
		// 				return ;
						
		// 			}
		// 			else{
		// 					MPI_Send(msg_recu,number_amount,MPI_INT,direction,TAGFOUND,MPI_COMM_WORLD);
		// 					printf("%d forward data to %d\n", *rang,direction);
		// 			}
						
	}
	
	//printf("--------Fin %d\n",*rang );
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
	printf("1 2 3 4 5 6 7 8 9 10\n");
	int_print_table(chord_ids, NB_SITES);
	printf("\n");

	nb_init = 1;
 	
 	// printf("nb_init ds %d\n",nb_init );
	for(i = 0; i < NB_SITES; i++){
		position = i + 1;
		if(position==1){
			gauche = NB_SITES;
			droite = position+1 ;
		}
		else if(position == NB_SITES){
			gauche = position-1 ;
			droite = 1 ;
		}
		else{
			gauche = position-1 ;
			droite = position+1; 
		}
		//déterminer init
		if(nb_init != 0){
			init = (rand() % 2); 
			if(init == 1){
				nb_init--;

				// printf("nb_init %d\n",nb_init );	
			}

 			
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
		init=0;

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
	int *msg;
	int *msg_recu;
	int *toSend;
	int winner=0,round=0;
	int leader;
	int cpt=0;
	int i,j;
	int taille_locale;
	int chord_ids[NB_SITES];// the index + 1 is the mpi rank and the value is chord id
	int finger_tables[NB_SITES][2][M]; // premiere colonne id chord, deuxieme colonne id MPI
	//printf("\nCalcul finger_tables\n");

	// reception des parametres
	receive_params(&chord_id,&length,&mpi_gauche,&mpi_droite,&state);
	//printf("%d gauche %d droite %d\n",rang,mpi_gauche,mpi_droite );
	// printf("%d\n",state );
	leader = rang;
	round = 0;
	taille_locale = rang+3;
	msg = malloc(sizeof(int)*(taille_locale)); //nb rang + rank 0 + ttl + id
	for(i=0;i<rang+2;i++){
		msg[i] = 0;
	}
	msg_recu = malloc(sizeof(int) * MAXNUMBER);
	msg[rang] = chord_id;
	msg[rang+1] = rang;
	msg[rang+2] = (int) pow(2,round);
	while(state == 1){
		//printf("\nmpi %d envoie avec TTL %d-----\n",rang,(int)pow(2,round));
		MPI_Send(msg, taille_locale, MPI_INT, mpi_gauche, TAGMSG, MPI_COMM_WORLD);
		MPI_Send(msg, taille_locale, MPI_INT, mpi_droite, TAGMSG, MPI_COMM_WORLD);
		while((cpt != 2) && winner == 0 ){
			receive_msg(&rang,&winner,&state,&leader,&cpt,&chord_id,&length,&mpi_gauche,&mpi_droite,msg,&taille_locale,msg_recu);
		}
		// printf("%d a recu un retour cpt=%d\n",rang,cpt);
		round++;
		cpt = 0;
		msg[taille_locale-1] = (int) pow(2,round);

		for(j = 0; j < taille_locale; j++){
			printf("%d ", msg[j]);
		}	
		printf("\nNext round%d taille %d\n",rang,taille_locale);

		sleep(2);
		printf("---------------------------------------\n");
		//msg[sizeof]
		if(winner){
			printf(" Mpi %d est le seul vainqueur\n",rang);
			MPI_Send(msg, taille_locale, MPI_INT, mpi_droite, TAGFOUND, MPI_COMM_WORLD);

			break;
		}
	}
	while(winner != 1){
		receive_msg(&rang,&winner,&state,&leader,&cpt,&chord_id,&length,&mpi_gauche,&mpi_droite,msg,&taille_locale,msg_recu);
	}


	printf("FIN %d------------------------------------------------------------\n",rang);
	for(i=0;i<NB_SITES;i++){
		chord_ids[i] = msg[i];
	}
	for(i = 0; i < NB_SITES; i++) {
		calculate_finger_table(chord_ids, finger_tables, i);
	}

	print_initial_data(chord_ids,finger_tables);
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

