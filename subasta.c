#include <nSystem.h>
#include <stdlib.h>
#include "subasta.h"

typedef struct subasta {
  int n; //unidades
  nMonitor m;
  int count;//cupos utilizados
  int minIndex,ready;

  //Oferentes
  int *e;//estados, true si está dentro de subasta
  double *p;//precios
  nCondition *c;//condiciones
} *Subasta;

void menorPostor(Subasta s){
	// se calcula el menor precio entre los oferentes de la subasta
	// y se guarda el minimo entre estos
	for (int i=0;i < s->count;i++){
		if(s->p[i] < s->p[s->minIndex]){
			s-> minIndex = i;
		}
	}
	//nPrintf("\tNueva menor oferta: %lf \n",s->p[s->minIndex]);
}

// Programe aca las funciones nuevaSubasta, ofrecer y adjudicar

Subasta nuevaSubasta(int unidades){
	//nPrintf("\tCreando nueva subasta con %d elementos",unidades);
	Subasta s = (Subasta)nMalloc(sizeof(*s));
	s->m = nMakeMonitor();
	s->n = unidades;
	s->count = 0;
	s->e = (int*)nMalloc(unidades*sizeof(int));
	s->p = (double*)nMalloc(unidades*sizeof(double));
	s->c = (nCondition*)nMalloc(unidades*sizeof(nCondition));
	for(int i=0;i<unidades;i++){
		//nPrintf("\n\t\te->afuera, ");
		s->e[i]=FALSE;//afuera;
		//nPrintf("p->%lf, ",0);
		s->p[i] = 0.0;
		//nPrintf("c->condicion");
		s->c[i] = nMakeCondition(s->m);
	}
	s->minIndex = 0;
	s->ready=FALSE;
	//nPrintf("... subasta abierta!\n");
	return s;
}

int ofrecer(Subasta s, double precio){
	// oferta de comprar un elemento de la subasta
	// esperando hasta que
	// 1. la subasta se cierre (se llame a adjudicar) retornando TRUE
	// 2. los otros oferentes tienen mejores ofertas, retorna FALSE
	nEnter(s->m);
	int index;
	if (s->count == 0){ //primer oferente
		//nPrintf("\t+ Ingresa primera oferta: %lf",precio);
		index=0;
		s->count++;
		s->e[index] = TRUE;//dentro;
		//nPrintf("\t 1..");
		s->p[index] = precio;
		//nPrintf("\t 2..");
		menorPostor(s);
		//nPrintf("\t min..\n");
		//nPrintf("\t... esperando ...\n");
		nWaitCondition(s->c[index]);//(1)
	} 
	else if (s->count < s->n){//primeros n oferentes
		//nPrintf("\t+ Ingresa nueva oferta: %lf",precio);
		index = s->count;
		s->count++;
		s->e[index]= TRUE;//dentro;
		//nPrintf("\t 1..");
		s->p[index] = precio;
		//nPrintf("\t 2..");
		menorPostor(s);
		//nPrintf("\t min..\n");
		//nPrintf("\t... esperando ...\n");
		nWaitCondition(s->c[index]);//(1)
	} else {
		//nPrintf("\t+Ingresa nueva oferta: %lf \n",precio);
		//nPrintf("\t\tComparando %lf con la nueva oferta %lf \n",s->p[s->minIndex],precio);
		//comparar oferta con las ya existentes
		// si tiene una oferta menor retorna False (2)
		if (precio <= s->p[s->minIndex]){
			//nPrintf("\t\tOferta rechazada\n");
			nExit(s->m);
			return FALSE;
		}
		// si tiene una apuesta mayor expulsa al menor oferente y entra el (1)
		else{
			//nPrintf("\t\tCambiar oferentes en la subasta: ");
			//TODO swap
			index = s->minIndex;
			//nPrintf("Hechar al menor postor de la subasta\n");
			s->e[index] = FALSE;//afuera;
			nSignalCondition(s->c[index]);
			//nPrintf("\tAgregando la nueva oferta: ");
			s->e[index] = TRUE;//dentro;
			//nPrintf(".. 1 ");
			s->p[index] = precio;
			//nPrintf(".. 2 ");
			//nPrintf(".. min\n");
			menorPostor(s);
			//nPrintf("\t... esperando ...\n");
			nWaitCondition(s->c[index]);
			//nPrintf("\tOferente despierto: ");
			if (s->e[index] && s->ready){
				//nPrintf("DENTRO\n");
				nExit(s->m);
				return TRUE;
			} else {
				//nPrintf("AFUERA\n");
				nExit(s->m);
				return FALSE;
			}	
		}
	}
	//nPrintf("\tOferente despierto: ");
	if (s->e[index] && s->ready){
		//nPrintf("DENTRO\n");
		nExit(s->m);
		return TRUE;
	} else {
		//nPrintf("AFUERA\n");
		nExit(s->m);
		return FALSE;
	}	
}

double adjudicar(Subasta s, int *punidades){
	//nPrintf("\tAdjudicar!\n");
	nEnter(s->m);
	s->ready = TRUE;
	int resto = (int)s->n - (int)s->count;
	//nPrintf("\t\tQuedaron %d unidades libres\n",resto);
	*punidades = resto;
	double suma=0;
	for (int i=0;i < s->count;i++){
		suma = suma + s->p[i];
		nSignalCondition(s->c[i]);//nNotifyAll penca
	}
	//nPrintf("\t\tSuma = %lf\n",suma);
	nExit(s->m);
	return suma;
}