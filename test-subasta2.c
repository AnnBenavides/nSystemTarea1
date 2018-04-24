#include <nSystem.h>
#include "subasta.h"

// Para poder usar rand
#include <stdlib.h>

nSem mutex;

// print_msg:
//   1   => con mensajes y delay
//   0   => sin mensajes, con delay
//   -1  => sin mensajes, sin delay

int aleatorio(Subasta s, int print_msg, char *nom, int oferta) {
  if (print_msg>=0) {
    nSetTaskName(nom);
    nWaitSem(mutex); 
    int delay= rand()%1000;
    nSignalSem(mutex);
    nSleep(delay);
  }
  if (print_msg>0)
    nPrintf("%s ofrece %d\n", nom, oferta);
  int ret= ofrecer(s, oferta);
  if (print_msg>0) {
    if (ret)
      nPrintf("%s se adjudico un item a %d\n", nom, oferta);
    else
      nPrintf("%s fallo con su oferta de %d\n", nom, oferta);
  }
  return ret;
}

int oferente(Subasta s, int print_msg, char *nom, int oferta) {
  nSetTaskName(nom);
  if (print_msg>0)
    nPrintf("%s ofrece %d\n", nom, oferta);
  int ret= ofrecer(s, oferta);
  if (print_msg>0) {
    if (ret)
      nPrintf("%s se adjudico un item a %d\n", nom, oferta);
    else
      nPrintf("%s fallo con su oferta de %d\n", nom, oferta);
  }
  return ret;
}

int contraOferta(int print_msg) {
	//cada vez que un postor es rechazado intenta entrar de nuevo a la subasta
	//con un valor mas alto
  Subasta s= nuevaSubasta(2);
  nTask pedro= nEmitTask(aleatorio, s, print_msg, "pedro", 1);
  nTask juan= nEmitTask(aleatorio, s, print_msg, "juan", 3);
  nTask diego= nEmitTask(aleatorio, s, print_msg, "diego", 4);
  nTask pepe= nEmitTask(aleatorio, s, print_msg, "pepe", 2);
  if (nWaitTask(pedro))
    nFatalError("contraOferta", "pedro debio perder con 1\n");
  if (nWaitTask(pepe))
    nFatalError("contraOferta", "pepe debio perder con 2\n");

  pedro = nEmitTask(aleatorio, s, print_msg, "pedro", 5);
  if (nWaitTask(juan))
    nFatalError("contraOferta", "juan debio perder con 3\n");

  pepe = nEmitTask(aleatorio, s, print_msg, "pepe", 6);
  if (nWaitTask(diego))
    nFatalError("contraOferta", "diego debio perder con 4\n");

  juan = nEmitTask(aleatorio, s, print_msg, "juan", 7);
  if (nWaitTask(pedro))
    nFatalError("contraOferta", "pedro debio perder con 5\n");

  diego = nEmitTask(aleatorio, s, print_msg, "diego", 8);
  if (nWaitTask(pepe))
    nFatalError("contraOferta", "pepe debio perder con 6\n");

  pedro = nEmitTask(aleatorio, s, print_msg, "pedro", 9);
  if (nWaitTask(juan))
    nFatalError("contraOferta", "juan debio perder con 7\n");

  pepe = nEmitTask(aleatorio, s, print_msg, "pepe", 10);
  if (nWaitTask(diego))
    nFatalError("contraOferta", "diego debio perder con 8\n");

  int u;
  int recaud= adjudicar(s, &u);
  if (recaud!=19)
    nFatalError("contraOferta", "La recaudacion debio ser 19 y no %d\n", recaud);
  if (u!=0)
    nFatalError("test1", "Quedaron %d unidades sin vender\n", u);
  if (!nWaitTask(pedro))
    nFatalError("nMain", "pedro debio ganar con 9\n");
  if (!nWaitTask(pepe))
    nFatalError("nMain", "pepe debio ganar con 10\n");
  if (print_msg>0)
    nPrintf("El monto recaudado es %d y quedaron %d unidades sin vender\n",
            recaud, u);
  return 0;
}

#define N 30
#define M 20000

int nMain() {
  mutex= nMakeSem(1);
  nPrintf("una sola subasta con contraofertas\n");
  contraOferta(1);
  nPrintf("test aprobado\n");
  nPrintf("-------------\n");

  nSetTimeSlice(1);
  nPrintf("%d subastas en paralelo\n", N);
  nTask *tasks1= nMalloc(M*sizeof(nTask));
  //nTask *tasks2= nMalloc(M*sizeof(nTask));
  //nTask *tasks3= nMalloc(M*sizeof(nTask));
  int k;
  for (k=1; k<N; k++) {
    tasks1[k]= nEmitTask(contraOferta, 0);
    //tasks2[k]= nEmitTask(test2, 0);
    //tasks3[k]= nEmitTask(test3, 0);
  }
  tasks1[0]= nEmitTask(contraOferta, 1);
  //tasks2[0]= nEmitTask(test2, 1);
  //tasks3[0]= nEmitTask(test3, 1);
  for (k=0; k<N; k++) {
    nWaitTask(tasks1[k]);
    //nWaitTask(tasks2[k]);
    //nWaitTask(tasks3[k]);
  }
  nPrintf("test aprobado\n");
  nPrintf("-------------\n");

  nPrintf("%d subastas en paralelo\n", M*2);
  for (k=1; k<M; k++) {
    tasks1[k]= nEmitTask(contraOferta, -1);
    //tasks2[k]= nEmitTask(test2, -1);
  }
  tasks1[0]= nEmitTask(contraOferta, 1);
  //tasks2[0]= nEmitTask(test2, 1);
  nWaitTask(tasks1[0]);
  //nWaitTask(tasks2[0]);
  nPrintf("Enterrando tareas.  Cada '.' son 30 tareas enterradas.\n");
  for (k=1; k<M; k++) {
    nWaitTask(tasks1[k]);
    // nWaitTask(tasks2[k]);
    if (k%10==0) nPrintf(".");
  }
  nPrintf("\ntest aprobado\n");
  nPrintf(  "-------------\n");
  nPrintf("Felicitaciones: paso todos los tests 6(*O*)9 \n");
  return 0;
}
