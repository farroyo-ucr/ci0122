/*
  Calcula el numero PI utilizando una serie "infinita"
  Debe recibir la cantidad de iteraciones como parametro
        __
       \
  pi =  >  (-1)^i x 4/(2xi + 1)
       /__
       i=0
*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>

/*
 * Definicion de la estructura para el paso de mensajes con los resultados parciales
 */
struct msgbuf {
   long mtype;
   double parcial;
};


/*
   Realiza la acumulacion de terminos desde una posicion inicial hasta un termino final
*/
double calcularSumaParcialPi( int msgid, int proceso, long inicial, long terminos ) {
   double casiPi = 0;
   double alterna = 4;
   long divisor = 0;
   struct msgbuf mensaje;

   for ( long termino = inicial; termino < terminos; termino++ ) {
      divisor = termino + termino + 1;		// 2 x termino + 1
      casiPi += alterna/divisor;		// 4 / (2xi + 1)
      alterna *= -1;				// Pasa de 4 a -4 y viceversa, para realizar la aproximacion de los terminos
   }

   mensaje.mtype = 2020;
   mensaje.parcial = casiPi;
   msgsnd( msgid, &mensaje, sizeof( double ), IPC_NOWAIT );

   exit( 0 );

}


int main( int argc, char ** argv ) {
   long terminos, inicio, fin;
   int proceso;
   int pid, msgid;
   double resultado;
   struct msgbuf recibe;

   msgid = msgget( 0xABC123, IPC_CREAT | 0600 );
   if ( -1 == msgid ) {
      perror( "Calculo de pi: " );
      exit( 4 );
   }

   terminos = 1000000;
   if ( argc > 1 ) {
      terminos = atol( argv[ 1 ] );
   }

   for ( proceso = 0; proceso < 10; proceso++ ) {
      inicio = proceso * terminos/10;
      fin = (proceso + 1) * terminos/10;
      pid = fork();
      if ( ! pid ) {
         calcularSumaParcialPi( msgid, proceso, inicio, fin );
      } else {
         printf("Creando el proceso %d: termino inicial %ld, termino final %ld\n", pid, inicio, fin );
      }
   }

   resultado = 0.0;
   for ( proceso = 0; proceso < 10; proceso++ ) {
      msgrcv( msgid, &recibe, sizeof( double ), 2020, 0 );
      printf( "Resultado parcial recibido %15.10g \n", recibe.parcial );
      resultado += recibe.parcial;
   }

   printf( "Valor calculado de Pi es \033[91m %15.10g \033[0m con %ld terminos\n", resultado, terminos );

   msgctl( msgid, IPC_RMID, NULL );

}

