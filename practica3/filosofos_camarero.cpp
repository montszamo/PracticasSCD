/*****************************************************************************
* Montserrat Rodriguez Zamorano
* PRACTICA 3: Implementacion de algoritmos distribuidos con MPI
* Problema 3->Cena de los filosofos con camarero en MPI
*****************************************************************************/

#include <iostream>
#include <time.h>      // incluye "time"
#include <unistd.h>    // incluye "usleep"
#include <stdlib.h>    // incluye "rand" y "srand"
#include <mpi.h>

#define NUM_FILOSOFOS   5
#define CAMARERO        10
//etiquetas
#define pedir_tenedor   0
#define soltar_tenedor  1
#define sentarse        2
#define levantarse      3

/*Plantilla colores*/
#define GREEN                   "\033[0;32m"
#define BLUE                    "\033[0;34m"
#define CYAN                    "\033[0;36m"
#define RED                     "\033[0;31m"
#define PURPLE                  "\033[0;35m"
#define GRAY                    "\033[0;37m"
#define DARK_GRAY               "\033[0;30m"
#define YELLOW                  "\033[0;33m"
#define WHITE                   "\033[0;37m"
#define DEFAULT                 "\033[0m"

using namespace std;

void Filosofo( int id, int nprocesos);
void Tenedor ( int id, int nprocesos);
void Camarero ();
// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
   int rank, size;

   srand(time(0));
   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &rank );
   MPI_Comm_size( MPI_COMM_WORLD, &size );

   if( size!=(NUM_FILOSOFOS*2 + 1))
   {
      if(rank == 0)
         cout<<RED << "El numero de procesos debe ser " << NUM_FILOSOFOS*2 + 1
         << endl << DEFAULT << flush ;
      MPI_Finalize( );
      return 0;
   }

   if ((rank%2) == 0 && rank != CAMARERO){
      Filosofo(rank,size); // Los pares son Filosofos
  }
   else if(rank == CAMARERO){
      Camarero();
    }
   else
      Tenedor(rank,size);  // Los impares son Tenedores

   MPI_Finalize( );
   return 0;
}
// ---------------------------------------------------------------------

void Filosofo( int id, int nprocesos )
{
   int izq = (id+1) % (nprocesos-1);
   int der = ((id+nprocesos-1)-1) % (nprocesos-1);
   int mensaje = 0;
   //esto es solo para que se corresponda con el esquema
   int nf = id/2;
   int td = der/2;
   int tizq = izq/2;
   //para recibir mensajes
   int buf;
   MPI_Status status;

   while(1)
   {
     //SENTARSE
     cout << YELLOW << "Filosofo "<< nf << " solicita sentarse a la mesa."
     << endl << DEFAULT << flush;
     MPI_Ssend(&mensaje, 1, MPI_INT, CAMARERO, sentarse, MPI_COMM_WORLD);
     //esperar a que el camarero le deje sentarse
     MPI_Recv(&buf,1,MPI_INT, CAMARERO, sentarse, MPI_COMM_WORLD, &status);

     //COGER TENEDORES
    // Solicita tenedor izquierdo
    cout << YELLOW << "Filosofo "<< nf << " solicita tenedor izquierdo "
    << tizq << endl << DEFAULT << flush;
    MPI_Ssend(&mensaje, 1, MPI_INT, izq, pedir_tenedor, MPI_COMM_WORLD);

    // Solicita tenedor derecho
    cout << YELLOW << "Filosofo "<< nf << " solicita tenedor derecho "
    << td << endl << DEFAULT << flush;
    MPI_Ssend(&mensaje, 1, MPI_INT, der, pedir_tenedor, MPI_COMM_WORLD);

     //COMER

     cout<< PURPLE << "Filosofo "<< nf << " COMIENDO."
     << endl << flush;
     sleep((rand() % 3)+1);  //comiendo

     //SOLTAR TENEDORES
     // Suelta el tenedor izquierdo
     cout << YELLOW << "Filosofo "<< nf << " suelta tenedor izquierdo "
     << tizq << endl << DEFAULT << flush;
     MPI_Ssend(&mensaje, 1,MPI_INT, izq, soltar_tenedor, MPI_COMM_WORLD);

     // Suelta el tenedor derecho
     cout << YELLOW << "Filosofo "<< nf << " suelta tenedor derecho " << td
     << endl << DEFAULT << flush;
     MPI_Ssend(&mensaje, 1,MPI_INT, der, soltar_tenedor, MPI_COMM_WORLD);

     //LEVANTARSE
     MPI_Ssend(&mensaje, 1, MPI_INT, CAMARERO, levantarse, MPI_COMM_WORLD);
     MPI_Recv(&buf,1,MPI_INT, CAMARERO, levantarse, MPI_COMM_WORLD, &status);

     // PENSAR
     cout << CYAN << "Filosofo " << nf << " FILOSOFANDO."
     << endl << DEFAULT << flush;

     // espera bloqueado durante un intervalo de tiempo aleatorio
     // (entre una décima de segundo y un segundo)
     usleep( 1000U * (100U+(rand()%900U)) );
 }
}
// ---------------------------------------------------------------------

void Tenedor(int id, int nprocesos)
{
  int buf;
  MPI_Status status;
  int Filo;
  int nt = id/2; //para que se corresponda con el esquema

  while(1)
  {
    MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, pedir_tenedor, MPI_COMM_WORLD, &status);
    Filo = status.MPI_SOURCE; //proceso
    //para que se corresponda con el esquema
    int nfilo = Filo/2;

    cout << RED << "Tenedor " << nt << " recibe peticion de " << nfilo
    << endl << DEFAULT << flush;
    //espera que el filosofo que lo ha cogido lo libere
    MPI_Recv(&buf, 1, MPI_INT, Filo, soltar_tenedor, MPI_COMM_WORLD, &status);
    cout << RED << "Tenedor " << nt << " recibe liberacion de " << nfilo
    << endl << DEFAULT << flush;
  }
}
// ---------------------------------------------------------------------
void Camarero()
{
  int buf;
  MPI_Status status;
  int Filo, nfilo;
  int sentados = 0;
  int mensaje = 0;
  while(1){
      if(sentados < 4){ //si hay espacio, puede recibir ambas peticiones
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      }
      else{
        MPI_Probe(MPI_ANY_SOURCE, levantarse, MPI_COMM_WORLD, &status);
      }

      Filo = status.MPI_SOURCE;
      nfilo = Filo/2;

      if(status.MPI_TAG == sentarse){
        MPI_Recv(&mensaje,1,MPI_INT, Filo,sentarse, MPI_COMM_WORLD, &status);
        MPI_Send(&mensaje,1,MPI_INT, Filo, sentarse, MPI_COMM_WORLD);
        sentados++;
        cout << GREEN << "Camarero sienta a filosofo "<< nfilo << " a la mesa."
        << endl << DEFAULT << flush;
      }
      else{
        MPI_Recv(&mensaje,1,MPI_INT, Filo,levantarse, MPI_COMM_WORLD, &status);
        MPI_Send(&mensaje,1,MPI_INT, Filo, levantarse, MPI_COMM_WORLD);
        cout << GREEN << "Camarero levanta a filosofo "<< nfilo << " de la mesa."
        << endl << DEFAULT << flush;
        sentados--;
      }
  }
}
// ---------------------------------------------------------------------
