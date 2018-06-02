#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NB_SITES	10
#define M			6	// cle encodee sur M bits, doit etre inferieur à NB_SITES
#define I			((int) pow(2, M))
#define K			((int) pow(2, M))

///////////////////////////////////////////////////////////////////////////////////////////
//										SIGNATURES
///////////////////////////////////////////////////////////////////////////////////////////


// helper functions
static void int_print_table(int *table, int length);
static void print_initial_data(int *chord_ids, int finger_tables[NB_SITES][2][M]);
static void quickSort(int *intArray, int left, int right);


///////////////////////////////////////////////////////////////////////////////////////////
//										SIMULATOR
///////////////////////////////////////////////////////////////////////////////////////////


static inline int contains(int value, int *table, int length) {
	int i;
	for(i = 0; i < length; i++)
		if(table[i] == value)
			return i;
	return -1;
}

/*
 * Helper function used to generate a new unique chord id
 */
static int new_chord_id(int *table, int length) {
	int id;
	do {
		id = rand() % I;
	} while(contains(id, table, length) != -1);
	return id;
}


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

/*
 * Helper function used in the calculation of the finger table
 */
static inline int find_next_node(int value, int *chord_ids, int length) {
	int i;
	for(i = 0; i < length; i++)
		if(chord_ids[i] >= value)
			return i;
	return -1;
}

static void calculate_finger_table(int *chord_ids, int *chord_ids_sorted, int finger_tables[NB_SITES][2][M], int chord_id_position) {
	int value, position, j, i;
	

	for(j = 0; j < M; j++) {
		value = (chord_ids[chord_id_position] + ((int) pow(2, j))) % ((int) pow(2, M));
		position = find_next_node(value, chord_ids_sorted, NB_SITES);
		if(position == -1) {
			finger_tables[chord_id_position][0][j] = chord_ids_sorted[0];
			finger_tables[chord_id_position][1][j] = 1; // car rang MPI
		} else {
			finger_tables[chord_id_position][0][j] = chord_ids_sorted[position];
			finger_tables[chord_id_position][1][j] = find_position(chord_ids_sorted[position], chord_ids, NB_SITES) + 1; // car rang MPI
		}	
	}
	// printf("printing ft\n");
	// int_print_table(finger_tables[chord_id_position][0], M);
	// int_print_table(finger_tables[chord_id_position][1], M);
}


void simulator() 
{
	int i, mpi_id, temp;
	int search_chord_id, search_data_key;
	int chord_id_responsible;

	printf("\nDHT Centralise\n");

	// data
	int chord_ids[NB_SITES];// the index + 1 is the mpi rank and the value is chord id
	int finger_tables[NB_SITES][2][M]; // premiere colonne id chord, deuxieme colonne id MPI
	
	int chord_ids_sorted[NB_SITES];

	srand(time(NULL));

	// generationg ids
	for(i = 0; i < NB_SITES; i++)
		chord_ids[i] = new_chord_id(chord_ids, i);

	for(i = 0; i < NB_SITES; i++)
		chord_ids_sorted[i] = chord_ids[i];

	quickSort(chord_ids_sorted, 0, NB_SITES-1);

	printf("1  2  3  4  5  6  7  8  9  10\n");
	int_print_table(chord_ids, NB_SITES);
	printf("\n");
	printf("\n");
	int_print_table(chord_ids_sorted, NB_SITES);
	printf("\n");
	printf("\n");

	// calculation of finger tables
	for(i = 0; i < NB_SITES; i++) {
		calculate_finger_table(chord_ids, chord_ids_sorted, finger_tables, i);
	}

	// display of generated data
	print_initial_data(chord_ids, finger_tables);

	for(i = 0; i < NB_SITES; i++){
		mpi_id = i + 1;

		// int_print_table(finger_tables[i][0], M);
		// int_print_table(finger_tables[i][1], M);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//										PROGRAM
///////////////////////////////////////////////////////////////////////////////////////////


int main(){
	simulator();

	return 0;
}


// int main(){
// 	int i, position, gauche, droite;

// 	printf("%d\n", -1 % 5);

// 	printf("i p g d\n");
// 	printf("-------\n");
// 	for(i = 0; i < NB_SITES; i++){
// 		position = i + 1;

// 		gauche = position % (NB_SITES + 1) - 1;
// 		droite = position % NB_SITES + 1;

// 		if(!gauche)
// 			gauche = NB_SITES;

// 		printf("%d %d %d %d\n", i, position, gauche, droite);
// 	}

// 	return 0;
// }



///////////////////////////////////////////////////////////////////////////////////////////
//								HELPER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////


////////////////	DISPLAY

static void int_print_table(int *table, int length) {
	int i;
	for(i = 0; i < length; i++)
		printf("%d ", table[i]);
	// printf("\n");
}


static void print_initial_data(int *chord_ids, int finger_tables[NB_SITES][2][M]) {
	int i;
	for(i = 0; i < NB_SITES; i++) {
		printf("[CHORD] peer:%d\t| finger_table: ", chord_ids[i]); // id_mpi i+
		int_print_table(finger_tables[i][0], M);
		printf("\t\t| ");
		int_print_table(finger_tables[i][1], M); // rang MPI
		printf("\n");
	}
	printf("\n");
}


////////////////	SORT

void swap(int *intArray, int num1, int num2) {
	int temp = intArray[num1];
	intArray[num1] = intArray[num2];
	intArray[num2] = temp;
}

int partition(int *intArray, int left, int right, int pivot) {
	int leftPointer = left -1;
	int rightPointer = right;

	while(1) {
		while(intArray[++leftPointer] < pivot) {}
		
		while(rightPointer > 0 && intArray[--rightPointer] > pivot) {}

		if(leftPointer >= rightPointer)
			break;
		else
			swap(intArray, leftPointer,rightPointer);
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