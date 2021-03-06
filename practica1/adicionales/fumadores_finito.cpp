// *********************************************************************
// SCD. Práctica 1.
//
// Problema de los fumadores.
// Montserrat Rodriguez Zamorano
// *********************************************************************

#include <iostream>
#include <cassert>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>      // incluye "time(....)"
#include <unistd.h>    // incluye "usleep(...)"
#include <stdlib.h>    // incluye "rand(...)" y "srand"
#include <utility>

using namespace std ;

// -------------------Variables constantes y globales---------------------------
const unsigned num_fumadores = 3 ;
unsigned long sobre_el_mostrador = -1; //variable compartida que representa el ingrediente
//ingredientes y fumador al que le falta
pair <int, string> ingredientes[num_fumadores];
const unsigned limite_estanco = 10 ;
static unsigned contador_estanco = 0 ;
pthread_t fumadores[num_fumadores], estanco ; //se declaran aqui para poder hacer pthread_exit
// ------------------------------Semáforos--------------------------------------
sem_t
  puede_suministrar ,
  puede_fumar[num_fumadores] ,
  mutex ;
// ------------------------------Funciones--------------------------------------
// función que simula la acción de fumar  como un retardo aleatorio de la hebra

void fumar()
{
   //  inicializa la semilla aleatoria  (solo la primera vez)
   static bool primera_vez = true ;
   if ( primera_vez )
   {   primera_vez = false ;
      srand( time(NULL) );
   }

   // calcular un numero aleatorio de milisegundos (entre 1/10 y 2 segundos)
   const unsigned miliseg = 100U + (rand() % 1900U) ;

   // retraso bloqueado durante 'miliseg' milisegundos
   usleep( 1000U*miliseg );
}
// ----------------------------------------------------------------------------
//función que simula la acción de suministrar ingredientes

int suministrar() {
  static bool primera_vez = true ;
  //  inicializa la semilla aleatoria  (solo la primera vez)
  if ( primera_vez )
  {   primera_vez = false ;
     srand( time(NULL) );
  }

  unsigned ing = rand() % 3 ;

  sem_wait( &mutex) ;
    cout << "El estanquero coloca en el mostrador " << ingredientes[ing].second
    << "." << endl ;
  sem_post( &mutex) ;
  return ing ; //devuelve el índice del ingrediente correspondiente
}
// ----------------------------------------------------------------------------
//función "consumidor"

void * fumador(void * ih_void) {
  unsigned long ih = (unsigned long) ih_void ;
  while(contador_estanco < limite_estanco) {
    ///////////recoger/////////
    sem_wait ( &puede_fumar[ih]) ;
    sem_wait( &mutex) ;
      cout << "El fumador " << ingredientes[ih].first
      << " recoge " << ingredientes[ih].second << "." << endl ;
    sem_post ( &mutex) ;
    /////////fumar////////////
    sem_wait( &mutex) ;
      cout << "El fumador " << ingredientes[ih].first
      << " empieza a fumar." << endl ;
    sem_post ( &mutex) ;
    sem_post ( &puede_suministrar) ; //desbloquea al estanquero
    fumar() ;
    sem_wait( &mutex) ;
      cout << "El fumador " << ingredientes[ih].first
      << " ha terminado de fumar." << endl ;
    sem_post ( &mutex) ;
  }
  sem_wait( &mutex) ;
    cout << "El fumador " << ingredientes[ih].first
    << " recoge el último ingrediente y se va a su casa." << endl ;
  sem_post ( &mutex) ;
  pthread_exit( &estanco) ;
	return NULL;
}
// ----------------------------------------------------------------------------
//función "productor"

void * estanquero(void *) {
  while(contador_estanco < limite_estanco) {
    sem_wait( &puede_suministrar) ;
      sobre_el_mostrador = suministrar() ;
      contador_estanco++;
    sem_post( &puede_fumar[sobre_el_mostrador]) ;
  }
  sem_wait( &mutex) ;
    cout << "El estanquero coloca sobre el mostrador todos los ingredientes." << endl ;
  sem_post ( &mutex) ;
  for(unsigned i = 0; i < num_fumadores; i++) {
    sem_post( &puede_fumar[i]) ;
  }
  sem_wait( &mutex) ;
    cout << "El estanquero cierra el estanco." << endl ;
  sem_post ( &mutex) ;
  return NULL ;
}
// ----------------------------------------------------------------------------

int main()
{
  pair <int,string> par = make_pair(1,"cerillas");
  ingredientes[0] = par ;
  par = make_pair(2, "tabaco") ;
  ingredientes[1] = par ;
  par = make_pair (3, "papel") ;
  ingredientes[2] = par ;

  sem_init( &puede_suministrar, 0, 1) ;
  for( unsigned i = 0 ; i < num_fumadores ; i++ ) {
    sem_init( &puede_fumar[i], 0, 0) ;
  }
  sem_init( &mutex, 0, 1) ;

  //crear las hebras
  pthread_create( &estanco, NULL, estanquero, NULL) ;
  for( unsigned i = 0 ; i < num_fumadores ; i++ ) {
    void * arg_ptr = (void *) i ; //convertir entero a puntero
    pthread_create( &fumadores[i] , NULL, fumador, arg_ptr ) ;
  }

  //esperar a que las hebras se unan
  pthread_join( estanco, NULL ) ;
  for( unsigned i = 0 ; i < num_fumadores ; i++ ) {
    pthread_join( fumadores[i] , NULL ) ;
  }

  sem_wait( &mutex) ;
    cout << "Hasta mañana." << endl ;
  sem_post ( &mutex) ;

  //destruir los semáforos
  sem_destroy( &puede_suministrar) ;
  sem_destroy( &mutex) ;
  for( unsigned i = 0 ; i < num_fumadores ; i++ ) {
    sem_destroy( &puede_fumar[i]) ;
  }

  return 0 ;
}
