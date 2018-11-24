#include <iostream>
#include<stdlib.h>
#include<time.h>
#include "Semaforo.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>

using namespace std;

#define pisos 16
#define capacidad 8

void Ascensor();
void Persona(int i, int j);
void Subir(int piso);


Semaforo * sA;
Semaforo * sP;

//Memoria compartida
struct Compartido
{
  int pisoActual, pEnAscensor, pEspEntrar;
  int pSubiendo[pisos];
  int pBajando[pisos];
};

typedef struct Compartido varCompartidas;
varCompartidas * comp;

int id = 0;

int main(){

  id = shmget(0xB50060,sizeof(varCompartidas), 0600 | IPC_CREAT);
  if(id == 1){
    perror("main");
    exit(0);
  }
  comp = ( varCompartidas * ) shmat( id, NULL, 0 );

  srand(time(NULL));
  int num=1+rand()%(17-1); //random entre 1 y 16
  comp->pisoActual = num;
  comp->pEnAscensor = 0;
  comp->pEspEntrar = 0;
  for(int i=0; i< 16; i++){
    comp->pSubiendo[i] = 0;
    comp->pBajando[i] = 0;
  }
  
  sA = new Semaforo(0);
  sP = new Semaforo(0);
  int iter = 0;
  while(iter++ <= 10){
    if(!fork()){
      int pSube = 1+rand()%(17-1);//random entre 1 y 16
      srand(time(NULL)); //nueva semilla
      int pBaja = 1+rand()%(17-1);//random entre 1 y 16
      cout << "Piso Sube: " << pSube << " , piso Baja: " << pBaja << endl;
      Persona(pSube, pBaja);
      Ascensor();
    }
    iter++;
  }

  //printf( "Destruyendo los recursos de memoria compartida\n");
  shmdt( comp );
  shmctl( id, IPC_RMID, NULL );

  return 0;
}

void Ascensor(){
  bool direccion = true; //sube = true, baja = false
  if(comp->pEnAscensor == 0 && comp->pEspEntrar ==0){
    sP->Signal();
    sA->Wait();
  } //else{
    //while(comp->pEnAscensor > 0){
      if(direccion){ //sube
        Subir(comp->pisoActual);
        for(int i = comp->pisoActual; i<pisos; i++){
          if(comp->pBajando[i] > 0){
            comp->pEnAscensor -= comp->pBajando[i];
            comp->pBajando[i] = 0;
            if(comp->pEnAscensor == 0){
              sP->Signal();
              sA->Wait();
            }
          }
          if(comp->pSubiendo[i] > 0){
            Subir(i);
          }
        }
        direccion = !direccion;
      } else{ //baja
        Subir(comp->pisoActual);
        for(int i = comp->pisoActual; i<= 0; i--){
          if(comp->pBajando[i] > 0){
            comp->pEnAscensor -= comp->pBajando[i];
            comp->pBajando[i] = 0;
            if(comp->pEnAscensor == 0){
              sP->Signal();
              sA->Wait();
            }
          }
          if(comp->pSubiendo[i] > 0){
            Subir(i);
          }
        }
        cout << "Num Personas en ascensor: " << comp->pEnAscensor << endl;
        direccion = !direccion; //cambia de dirección
      }
    //}
   sP->Signal();
   sA->Wait(); 
  //}
}

void Persona(int i, int j){
  comp->pEspEntrar++;
  comp->pSubiendo[i]++; //piso donde sube
  sP->Wait(); //espera al ascensor
  comp->pBajando[j]++; //piso donde baja
  cout << "Persona sube en el piso: " << i << " y baja en el " << j << endl;
  sA->Signal();
  sP->Wait();
}

void Subir(int piso){
  comp->pEnAscensor +=comp->pSubiendo[piso];
  if(comp->pSubiendo[piso] > 0){
    if(comp->pEnAscensor > capacidad){ //deben subir algunos pasajeros
      int aux = comp->pSubiendo[piso];
      comp->pSubiendo[piso] = comp->pEnAscensor % capacidad;
      comp->pEspEntrar -= (aux - comp->pSubiendo[piso]);
    } else{
      comp->pEspEntrar -= comp->pSubiendo[piso];
      comp->pSubiendo[piso] = 0;
    } 
    sP->Signal();
  }
}