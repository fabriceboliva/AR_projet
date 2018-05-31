#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define TAGINIT    0
#define NB_SITE 6
#define TAGREQ 1
#define TAGMIN 2
#define TAGANN 3

void simulateur(void) {
   int i;

   /* nb_voisins[i] est le nombre de voisins du site i */
   int nb_voisins[NB_SITE+1] = {-1, 3, 3, 2, 3, 5, 2};
   int min_local[NB_SITE+1] = {-1, 12, 11, 8, 14, 5, 17};

   /* liste des voisins */
   int voisins[NB_SITE+1][5] = {{-1, -1, -1, -1, -1},
            {2, 5, 3, -1, -1}, {4, 1, 5, -1, -1}, 
            {1, 5, -1, -1, -1}, {6, 2, 5, -1, -1},
            {1, 2, 6, 4, 3}, {4, 5, -1, -1, -1}};
                               
   for(i=1; i<=NB_SITE; i++){
      MPI_Send(&nb_voisins[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
      MPI_Send(voisins[i], nb_voisins[i], MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
      MPI_Send(&min_local[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD); 
   }

   
}

int recevoir_params(int *nb_voisins, int **voisins, int *min_local, int rang) {
   int i;
   MPI_Status status;
   MPI_Recv(nb_voisins, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

   *voisins = (int *) malloc( (*nb_voisins) * sizeof(int));
   if(*voisins == NULL) {
      perror("malloc - voisins");
      goto error_voisins;
   }

   MPI_Recv(*voisins, *nb_voisins, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
   MPI_Recv(min_local, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

   return 0;

   error_voisins:
   exit(-1);
}

void calcul_min(int rang) {
   int nb_voisins, *voisins, min_local, i, recu, j;
   int envoi, pere;
   MPI_Status status;

   // receiving parameters from the simulator process
   recevoir_params(&nb_voisins, &voisins, &min_local, rang);

   
	if (rang == 1) {

      // initiateur 
      // printf("initiateur\n");

      //envoie msg à tous ses voisins
      for(i=0; i<nb_voisins; i++){
         printf("\n%d: envoi a voisin: %d, msg: %d\n", rang, voisins[i], min_local);
         fflush(stdout);
         MPI_Send(&min_local, 1, MPI_INT, voisins[i], TAGREQ, MPI_COMM_WORLD);
      }

      // initiateur attend le message des autres voisins
      for(i=0; i<nb_voisins; i++){// si feuille 1 voisin donc i=0 et i !< 0 => pas de boucle => pas d'attente
         MPI_Recv(&recu, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
         // printf("\n%d: reception du voisin: %d, min_recu: %d\n", rang, status.MPI_SOURCE, recu);
         // fflush(stdout);
         if (recu < min_local)
            min_local = recu;
      }

      printf("%d: min_local:%d\n", rang, min_local);

      /////////////////

      // printf("%d: min_local:%d -> decideur\n", rang, min_local);
      // fflush(stdout);

      // diffusion du resultat
      for(i=0; i<nb_voisins; i++){
         // printf("\n%d: annonce a voisin: %d, min: %d\n", rang, voisins[i], min_local);
         // fflush(stdout);
         MPI_Send(&min_local, 1, MPI_INT, voisins[i], TAGANN, MPI_COMM_WORLD);
      }

	} else {
      // les non-initiateurs 

      // attente d'un msg d'un voisin
      MPI_Recv(&recu, 1, MPI_INT, MPI_ANY_SOURCE, TAGREQ, MPI_COMM_WORLD, &status);
      // printf("%d: reception du voisin: %d, msg_recu: %d\n", rang, status.MPI_SOURCE, recu);
      // fflush(stdout);
      pere = status.MPI_SOURCE;
      if (recu < min_local)
            min_local = recu;

      
      // envoi message aux autres voisins
      for(i=0; i<nb_voisins; i++){
         if(voisins[i] != pere){
            MPI_Send(&min_local, 1, MPI_INT, voisins[i], TAGREQ, MPI_COMM_WORLD);
         }
         // printf("\n%d: envoi a voisin: %d, min_local: %d\n", rang, voisins[i], min_local);
         // fflush(stdout);
      }



      
      // reception des messages depuis d'autres voisins
      for(i=0; i<nb_voisins-1; i++){
         // printf("\n%d: attente msg\n", rang);
         MPI_Recv(&recu, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
         // printf("\n%d: reception du voisin: %d, msg_recu: %d\n", rang, status.MPI_SOURCE, recu);
         // fflush(stdout);
         if (recu < min_local)
            min_local = recu;
      }
      printf("\n%d: pere:%d, min_local:%d\n", rang, pere, min_local);

      ///////////////////

      // // envoi min local au pere
      MPI_Send(&min_local, 1, MPI_INT, pere, TAGMIN, MPI_COMM_WORLD);

      // // attente min global provenant du pere
      MPI_Recv(&recu, 1, MPI_INT, pere, TAGANN, MPI_COMM_WORLD, &status);
      min_local = recu;

      printf("%d: res decision:%d\n", rang, min_local);

      // envoi message aux autres voisins
      for(i=0; i<nb_voisins; i++){
         if(voisins[i] != pere)
            MPI_Send(&min_local, 1, MPI_INT, voisins[i], TAGANN, MPI_COMM_WORLD);
         // printf("\n%d: envoi a voisin: %d, min_local: %d\n", rang, voisins[i], min_local);
         // fflush(stdout);
      }
   }

   free(voisins);
}


/******************************************************************************/

int main (int argc, char* argv[]) {
   int nb_proc,rang;
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

   if (nb_proc != NB_SITE+1) {
      printf("Nombre de processus incorrect !\n");
      MPI_Finalize();
      exit(2);
   }
  
   MPI_Comm_rank(MPI_COMM_WORLD, &rang);
  
   if (rang == 0) {
      simulateur();
   } else {
      calcul_min(rang);
   }
  
   MPI_Finalize();
   return 0;
}
