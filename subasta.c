#include <nSystem.h>
#include <stdlib.h>
#include "subasta.h"

typedef enum{
	dentro, afuera
} Estado;

typedef struct oferente{
	Estado e;
	double p;
	nCondition c;
} *Oferente;

typedef struct subasta {
  int n; //cupo de subastadores
  nMonitor m;
  int count;//cupos utilizados
  Oferente *o;//ofretentes
  double min;
  int minIndex,ready;
} *Subasta;

void menorPostor(Subasta s){
	// se calcula el menor precio (s->min) entre los oferentes(s->o[:]) de la subasta
	// y se guarda el minimo entre estos (s->minIndex)
	for (int i=0;i < s->count;i++){
		if(s->o[i]->p < s->min){
			s-> min = s->o[i]->p;
			s-> minIndex = i;
		}
	}
	nPrintf("\tNueva menor oferta: %lf \n",s->min);
}

Oferente initOferta(Subasta s, double precio){
	nPrintf("\tRegistrando oferta ");
	Oferente o = nMalloc(sizeof(Oferente));
	nPrintf("e->afuera, ");
	o->e=afuera;
	nPrintf("p->%lf, ",precio);
	o->p = precio;
	nPrintf("c->condicion \n");
	o->c = nMakeCondition(s->m);
	return o;
	nPrintf("\t...\n");
}

// Programe aca las funciones nuevaSubasta, ofrecer y adjudicar

Subasta nuevaSubasta(int unidades){
	nPrintf("\tCreando nueva subasta con %d elementos",unidades);
	Subasta s = (Subasta)nMalloc(sizeof(*s));
	s->m = nMakeMonitor();
	s->n = unidades;
	s->count = 0;
	s->o = (Oferente*)nMalloc(unidades*sizeof(Oferente));//probar sin *
	for(int i=0;i<unidades;i++){
		s->o[i] = nMalloc(sizeof(Oferente));
		nPrintf("\n\t\te->afuera, ");
		s->o[i]->e=afuera;
		nPrintf("p->%lf, ",0);
		s->o[i]->p = 0.0;
		nPrintf("c->condicion");
		s->o[i]->c = nMakeCondition(s->m);
	}
	s->minIndex = 0;
	s->ready=FALSE;
	nPrintf("... subasta abierta!\n");
	return s;
}

int ofrecer(Subasta s, double precio){
	// oferta (precio) de comprar un elemento de la subasta
	Oferente O;// = initOferta(s,precio);
	// esperando hasta que:
	// 1. la subasta se cierre (se llame a adjudicar) retornando TRUE
	// 2. los otros oferentes tienen mejores ofertas, retorna FALSE
	nEnter(s->m);
	if (s->count==0){ //primer oferente
		nPrintf("\t+ Ingresa primera oferta: %lf",precio);
		s->o[0]->e = dentro;
		nPrintf("\t 1..");
		s->o[0]->p = precio;
		nPrintf("\t 2..");
		s->min = precio;
		nPrintf("\t min..\n");
		s->count++;
		nPrintf("\t... esperando ...\n");
		O=s->o[0];
		nWaitCondition(O->c);//(1)
	} 
	else if (s->count < s->n){//primeros n oferentes
		nPrintf("\t+ Ingresa nueva oferta: %lf",precio);
		s->o[s->count]->e = dentro;
		s->o[s->count]->p = precio;
		O=s->o[s->count];
		s->count++;
		menorPostor(s);
		nPrintf("\t... esperando ...\n");
		nWaitCondition(O->c);//(1)
	} else {
		nPrintf("\t+Ingresa nueva oferta: %lf \n",precio);
		nPrintf("\t\tComparando %lf con la nueva oferta %lf \n",s->min,precio);
		//comparar oferta con las ya existentes
		// si tiene una oferta menor retorna False (2)
		if (precio <= s->min){
			nPrintf("\t\tOferta rechazada\n");
			nExit(s->m);
			return FALSE;
		}
		// si tiene una apuesta mayor expulsa al menor oferente y entra el (1)
		else{
			nPrintf("\t\tCambiar oferentes en la subasta: ");
			//TODO swap
			s->o[s->minIndex]->e=afuera;
			nSignalCondition(s->o[s->minIndex]->c);
			nSleep(1000);
			nPrintf("Hechar al menor postor de la subasta\n");
			//poner los datos del nuevo oferente

			s->o[s->minIndex]->e=dentro;
			s->o[s->minIndex]->p=precio;
			int index=s->minIndex;
			s->min=precio;
			menorPostor(s);
			nPrintf("\t... esperando ...\n");
			nWaitCondition(s->o[index]->c);
		}
	}
	nPrintf("\tOferente despierto: ");
	if (O->e == dentro && s->ready){
		nPrintf("DENTRO\n");
		nExit(s->m);
		return TRUE;
	} else {
		nPrintf("AFUERA\n");
		nExit(s->m);
		return FALSE;
	}	
}

double adjudicar(Subasta s, int *punidades){
	nPrintf("\tAdjudicar!\n");
	nEnter(s->m);
	s->ready=TRUE;
	int resto=(int)s->n - (int)s->count;
	nPrintf("\t\tQuedaron %d unidades libres\n",resto);
	*punidades = resto;
	double suma=0;
	for (int i=0;i < s->count;i++){
		suma= suma + s->o[i]->p;
		nSignalCondition(s->o[i]->c);//nNotifyAll penca
	}
	nPrintf("\t\tSuma = %lf\n",suma);
	nExit(s->m);
	return suma;
}