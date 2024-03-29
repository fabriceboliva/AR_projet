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
#define TAGEND		1

#define TAGOUT		2
#define TAGIN		3

#define TAGFOUND	4
#define TAGRES		5

#define MAXNUMBER I




///////////////////////////////////////////////////////////////////////////////////////////
//										SIGNATURES
///////////////////////////////////////////////////////////////////////////////////////////

// chord functions
void simulator();
void dht_node(int mpi_id);

// Helper functions
static inline int contains(int value, int *table, int length);
// static inline int find_position(int value, int *table, int length);
// static inline int find_next_node(int value, int *chord_ids, int length);

void int_print_table(int *table, int length);
// static void print_initial_data(int *chord_ids, int finger_tables[NB_SITES][2][M]);


///////////////////////////////////////////////////////////////////////////////////////////
//										PROGRAM
///////////////////////////////////////////////////////////////////////////////////////////


int main (int argc, char* argv[]) 
{
	int nb_proc, mpi_id;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

	if(nb_proc != NB_SITES+1) {
		printf("Error in number of process !\n");
		MPI_Finalize();
		exit(2);
	}

	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_id);

	if(mpi_id == 0)
		simulator();
	else
		dht_node(mpi_id);

	MPI_Finalize();
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////
//										SIMULATOR
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
		id = rand() % I + 1;
	} while(contains(id, table, length) != -1);
	return id;
}

void simulator() 
{
	int i, position;
	int gauche, droite;

	printf("\nDHT Centralise\n");

	// data
	int chord_ids[NB_SITES];
	int state[NB_SITES];
	
	srand(time(NULL));

	// initialisation des identifiants
	for(i = 0; i < NB_SITES; i++) {
		chord_ids[i] = new_chord_id(chord_ids, i);
		state[i] = ((rand() % 4) < 1)?1:0;;
	}

	printf("chord_ids table: \n");
	printf("1 2 3 4 5 6 7 8 9 10\n");
	int_print_table(chord_ids, NB_SITES);
	printf("\n");
	
	for(i = 0; i < NB_SITES; i++){
		position = i + 1;
		droite = position % (NB_SITES + 1) - 1;
		gauche = position % NB_SITES + 1;

		droite = (droite)?droite:NB_SITES;

		// printf("envoi a position: %d, chord_ids: %d\n", position, chord_ids[i]);
		MPI_Send(&chord_ids[i], 1, MPI_INT, position, TAGINIT, MPI_COMM_WORLD);

		//envoi du voisin MPI gauche
		MPI_Send(&gauche, 1, MPI_INT, position, TAGINIT, MPI_COMM_WORLD);

		//envoi du voisin MPI droite
		MPI_Send(&droite, 1, MPI_INT, position, TAGINIT, MPI_COMM_WORLD);

		//envoie booleen intialisation
		MPI_Send(&state[i], 1, MPI_INT, position, TAGINIT, MPI_COMM_WORLD);
	}


}


///////////////////////////////////////////////////////////////////////////////////////////
//											PEERS
///////////////////////////////////////////////////////////////////////////////////////////

void receive_params(int *chord_id, int *gauche, int *droite, int *state) 
{
	MPI_Status status;

	MPI_Recv(chord_id, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

	MPI_Recv(gauche, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	
	MPI_Recv(droite, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

	MPI_Recv(state, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	return;

}


// /*
//  * Helper function used in the calculation of the finger table
//  */
// static inline int find_next_node(int value, int *chord_ids, int length) {
// 	int i;
// 	for(i = 0; i < length; i++)
// 		if(chord_ids[i] >= value)
// 			return i;
// 	return -1;
// }


// static void calculate_finger_table(int *chord_ids, int finger_tables[NB_SITES][2][M], int chord_id_position) {
// 	int value, mpi_id, j;
// 	for(j = 0; j < M; j++) {
// 		value = (chord_ids[chord_id_position] + ((int) pow(2, j))) % ((int) pow(2, M));
// 		mpi_id = find_next_node(value, chord_ids, NB_SITES);
// 		if(mpi_id == -1) {
// 			finger_tables[chord_id_position][0][j] = chord_ids[0];
// 			finger_tables[chord_id_position][1][j] = 1; // car rang MPI
// 		} else {
// 			finger_tables[chord_id_position][0][j] = chord_ids[mpi_id];
// 			finger_tables[chord_id_position][1][j] = mpi_id + 1; // car rang MPI
// 		}
// 	}
// }

void receive_msg(int *rang,int *winner,int *state,int *leader,int *cpt, int *gauche, int *droite,int *msg, int *taille_locale,int *msg_recu) 
{
	MPI_Status status;
	int *new;
	int i;
	int direction,direction_inverse;
	int number_amount;
	int max;
	MPI_Recv(msg_recu, MAXNUMBER, MPI_INT, MPI_ANY_SOURCE, TAGMSG, MPI_COMM_WORLD, &status);
	MPI_Get_count(&status,MPI_INT,&number_amount);
	printf("%d\n",number_amount );
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
	
	if(status.MPI_SOURCE == *gauche){
		direction = *droite;
		direction_inverse = *gauche;
	}
	else{
		direction = *gauche;
		direction_inverse = *droite;
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
				printf("agrandit %d %d\n",number_amount,max);
			}
				
			//printf("\n%d taille %d\n",*rang,*taille_locale);
			*cpt=*cpt+1;
			printf("%d : Arrivé sur init source %d\n",*rang,status.MPI_SOURCE);
		}
	}
		
	free(new);
	//printf("--------Fin %d\n",*rang );
	return;

}


static int build_message(int **message, int round, int leader) {
	if(round == 0) {
		msg_size = mpi_id + 2; // chord_ids + leader + ttl
		msg = (int *) calloc(msg_size, sizeof(int));
		if(!msg)
			
		msg[mpi_id - 1] = chord_id;
	}

	msg[msg_size - 2] = leader;
	msg[msg_size - 1] = (int) pow(2, round);
}


void dht_node(int mpi_id)
{
	int chord_id, mpi_gauche, mpi_droite, active;
	int leader, round;
	int *chord_ids, chord_ids_size;
	int *msg, msg_size, *in_msg, in_msg_size;

	int cpt=0;
	int i, j;
	//printf("\nCalcul finger_tables\n");

	// reception des parametres
	receive_params(&chord_id, &mpi_gauche, &mpi_droite, &active);
	printf("chord %d: gauche %d, mpi %d, droite %d, active %d\n", chord_id, mpi_gauche, mpi_id, mpi_droite , active);
	
	return;

	leader = mpi_id;
	round = 0;

	chord_ids = (int *) malloc(mpi_id * sizeof(int));
	if(!chord_ids)
		goto err;

	in_msg = (int *) calloc(MAXNUMBER, sizeof(int));
	if(!in_msg)
		goto err1;

	chord_ids[mpi_id - 1] = chord_id;

	while(active == 1) {

		build_message(&message);

		

		//printf("\nmpi %d envoie avec TTL %d-----\n",mpi_id,(int)pow(2,round));
		MPI_Send(msg, msg_size, MPI_INT, mpi_gauche, TAGOUT, MPI_COMM_WORLD);
		MPI_Send(msg, msg_size, MPI_INT, mpi_droite, TAGOUT, MPI_COMM_WORLD);


		for(i = 0; i < 2; i++){
			MPI_Recv(msg_recu, MAXNUMBER, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			if()
			MPI_Get_count(&status, MPI_INT, &in_msg_size);
		}
		

		//teste si j'ai perdu si ou je sors

		//

		while(cpt != 2){
			receive_msg(&mpi_id,&winner,&active,&leader,&cpt,&mpi_gauche,&mpi_droite,msg,&taille_locale,msg_recu);
		}
		// printf("%d a recu un retour cpt=%d\n",mpi_id,cpt);
		round++;
		cpt = 0;
		msg[taille_locale-1] = (int) pow(2,round);

		for(j = 0; j < taille_locale; j++){
			printf("%d ", msg[j]);
		}	
		printf("\nNext round%d taille %d\n",mpi_id,taille_locale);

		sleep(2);
		printf("---------------------------------------\n");
		//msg[sizeof]
		if(winner){
			printf(" Mpi %d est le seul vainqueur\n",mpi_id);
			break;
		}
	}

	while(winner != 1){ // leader == moi
		receive_msg(&mpi_id,&winner,&active,&leader,&cpt,&mpi_gauche,&mpi_droite,msg,&taille_locale,msg_recu);
		winner = 1;
		for(i=0;i<NB_SITES+2;i++){
				if(msg[i] == 0){
					winner = 0;
				}
		}
	}


	// printf("FIN------------------------------------------------------------\n");
	// for(i=0;i<NB_SITES;i++){
	// 	chord_ids[i] = msg[i];
	// }
	// for(i = 0; i < NB_SITES; i++) {
	// 	calculate_finger_table(chord_ids, finger_tables, i);
	// }

	//print_initial_data(chord_ids,finger_tables);
err:
	return;
}


///////////////////////////////////////////////////////////////////////////////////////////
//										HELPER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////

////////////////	DISPLAY

void int_print_table(int *table, int length) {
	int i;
	for(i = 0; i < length; i++)
		printf("%d ", table[i]);
	printf("\n");
}

// static void print_initial_data(int *chord_ids, int finger_tables[NB_SITES][2][M]) {
// 	int i;
// 	for(i = 0; i < NB_SITES; i++) {
// 		printf("[CHORD] peer:%d\t| finger_table: ", chord_ids[i]); // id_mpi i+
// 		int_print_table(finger_tables[i][0], M);
// 		// int_print_table(finger_tables[i][1], M); // rang MPI
// 		// printf("\n");
// 	}
// 	printf("\n");
// }


////////////////	TABLES

/*
 * Helper function to find position of a value in the specified table
 */
static inline int find_position(int value, int *table, int length) {
	int i, position = -1;
	for(i = 0; i < length; i++)
		if(table[i] == value)
			position = i;
	return position;
}

