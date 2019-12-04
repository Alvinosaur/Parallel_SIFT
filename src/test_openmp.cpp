#include <omp.h>
#include <stdio.h>
 #define N 20

 main(int argc, char *argv[]) {

 int i;
 float a[N], b[N], c[N], d[N];

 /* Some initializations */
 for (i=0; i < N; i++) {
   a[i] = i * 1.5;
   b[i] = i + 22.35;
   }

  #pragma omp parallel for schedule(static)
  for(int n=0; n<10; ++n) {
    printf(" %d", n);
  }
  printf(".\n");
  return 0;
/*
 #pragma omp parallel shared(a,b,c,d) private(i)
   {
   #pragma omp sections nowait
     {
     #pragma omp section
     for (i=0; i < N; i++){
       c[i] = a[i] + b[i];
       printf("Thread: %d, i: %d\n", omp_get_thread_num(), i);
     }
     #pragma omp section
     for (i=0; i < N; i++){
       d[i] = a[i] * b[i];
       printf("Thread: %d, i: %d\n", omp_get_thread_num(), i);
     }
     }  // end of sections
   }  // end of parallel region
 */
 }