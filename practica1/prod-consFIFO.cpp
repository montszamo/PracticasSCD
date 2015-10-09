// *********************************************************************
// SCD. Práctica 1.
//
// Ejercicio productor-consumidor (LIFO)
// Montserrat Rodriguez Zamorano
// *********************************************************************

#include <iostream>
#include <cassert>
#include <pthread.h>
#include <semaphore.h>

using namespace std ;

// ---------------------------------------------------------------------
// variables compartidas y constantes
const unsigned
  num_items  = 40 ,
  tam_vector = 10 ;

int buffer [tam_vector] ; //buffer de escritura
unsigned escritos = 0 ;
unsigned leidos = 0 ;

sem_t
  mutex ,
  puede_leer ,
  puede_escribir ;
// ---------------------------------------------------------------------

unsigned producir_dato()
{
  static int contador = 0 ;
  sem_wait (&mutex) ;
  cout << "producido: " << contador << endl << flush ;
  sem_post (&mutex) ;
  return contador++ ;
}
// ---------------------------------------------------------------------

void consumir_dato( int dato )
{
  sem_wait (&mutex) ;
    cout << "dato recibido: " << dato << endl ;
  sem_post (&mutex) ;
}
// ---------------------------------------------------------------------

void * productor( void * )
{
  for( unsigned i = 0 ; i < num_items ; i++ )
  {
    int dato = producir_dato() ;
    sem_wait(&puede_escribir) ;
      if(escritos < tam_vector) { // si se puede escribir en el buffer, producir
        buffer[escritos] = dato ;
        escritos++ ;

        if(escritos == tam_vector) {
          escritos = 0;
        }
      }
    sem_post(&puede_leer) ;
  }
  return NULL ;
}
// ---------------------------------------------------------------------

void * consumidor( void * )
{
  for( unsigned i = 0 ; i < num_items ; i++ )
  {
    sem_wait(&puede_leer) ;
      int dato ;
      if(leidos < tam_vector) { // si hay algo en el buffer, consumir
        dato = buffer[leidos];
        leidos++ ;

        if(leidos == tam_vector){
          leidos = 0;
        }
      }
    sem_post(&puede_escribir) ;
    consumir_dato(dato) ;
  }
  return NULL ;
}
//----------------------------------------------------------------------

int main()
{
  pthread_t productora, consumidora ;
  // Inicializar los semáforos
  sem_init( &puede_leer , 0 , 0 ); // inicialmente no se puede leer
  sem_init( &puede_escribir , 0 , 1 ); // inicialmente se puede escribir
  sem_init( &mutex , 0, 1);

  // Crear las hebras
  pthread_create( &productora , NULL, productor, NULL) ;
  pthread_create( &consumidora, NULL, consumidor, NULL) ;

  //Esperar a que las hebras terminen
  pthread_join( productora, NULL ) ;
  pthread_join( consumidora, NULL ) ;

  //Destruir los semáforos
  sem_destroy( &puede_leer ) ;
  sem_destroy( &puede_escribir ) ;
  sem_destroy( &mutex ) ;


   return 0 ;
}