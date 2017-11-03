#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#define min(x, y) ((x)<(y)?(x):(y))

double* gen_matrix(int n, int m);
int mmult(double *c, double *a, int aRows, int aCols, double *b, int bRows, int bCols);
void compare_matrix(double *a, double *b, int nRows, int nCols);

/** 
    Program to multiply a matrix times a matrix using both
    mpi to distribute the computation among nodes and omp
    to distribute the computation among threads.
*/

int main(int argc, char* argv[])
{
  int ans;
  int i, j, numsent, sender;
  int anstype, row;
  int nrows = 5;
  int ncols = 5;
  //double *aa = (double*)malloc(sizeof(double) * nrows * ncols);	/* the A matrix */
  //double *bb;	/* the B matrix */
  double *buffer1;
  double *buffer2;
  double *cc1;	/* A x B computed using the omp-mpi code you write */
  double *cc2;	/* A x B computed using the conventional algorithm */
  int myid, master, numprocs;
  double starttime, endtime;
  MPI_Status status;
  /* insert other global variables here */
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    double aa[25] = {1, 2, 3, 4, 5,
	  6, 7, 8, 9, 10,
	  11, 12, 13, 14, 15,
	  16, 17, 18, 19, 20,
	  21, 22, 23, 24, 25};
    double bb[25] = {1, 0, 0, 0, 0,
	  0, 1, 0, 0, 0,
	  0, 0, 1, 0, 0,
	  0, 0, 0, 1, 0,
	  0, 0, 0, 0, 1};
    buffer1 = (double*)malloc(sizeof(double) * ncols);
    buffer2 = (double*)malloc(sizeof(double) * ncols);
    if (myid == master) {
      // Master Code goes here
      starttime = MPI_Wtime();
      numsent = 0;
      MPI_Bcast(bb, ncols, MPI_DOUBLE, master, MPI_COMM_WORLD);
      for (i = 0; i < min(numprocs-1, nrows); i++) {
	for (j = 0; j < ncols; j++) {
		buffer1[j] = aa[i * ncols + j];
	}
	MPI_Send(buffer1, ncols, MPI_DOUBLE, i+1, i+1, MPI_COMM_WORLD);
	numsent++;
      }
      /* Insert your master code here to store the product into cc1 */
      for (i = 0; i < nrows; i++) {
	MPI_Recv(&ans, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	sender = status.MPI_TAG;
	anstype = status.MPI_TAG;
	if (anstype < 25)
		cc1[anstype-1] = ans;
	if(numsent < nrows) {
		for (j = 0; j < ncols; j++) {
			//buffer1[j] = aa[numsent*ncols + j];
		}
		MPI_Send(buffer1, ncols, MPI_DOUBLE, sender, numsent+1, MPI_COMM_WORLD);
		numsent++;
	}
	else {
		MPI_Send(MPI_BOTTOM, 0, MPI_DOUBLE, sender, 0, MPI_COMM_WORLD);
	}
      }
      endtime = MPI_Wtime();
      printf("%f\n",(endtime - starttime));
      //cc2  = malloc(sizeof(double) * nrows * ncols);
      //mmult(cc2, aa, nrows, ncols, bb, ncols, nrows);
      //compare_matrices(cc2, cc1, nrows, nrows);
  } else {
    // Slave Code goes here
    MPI_Bcast(bb, ncols, MPI_DOUBLE, master, MPI_COMM_WORLD);
    if(myid <= nrows) {
      while(1){
	MPI_Recv(buffer1, ncols, MPI_DOUBLE, master, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	if (status.MPI_TAG == 0) 
		break;
	ans = 0;
#pragma omp shared(ans) for reduction(+:ans)
	for (j = 0; j < ncols; j++) {
		ans += buffer1[j] * bb[5 * j];
	}
	MPI_Send(&ans, 1, MPI_DOUBLE, master, row, MPI_COMM_WORLD);
      }
  }
  else {
    fprintf(stderr, "Usage matrix_times_vector <size>\n");
  }
  MPI_Finalize();
  return 0;
}
}
