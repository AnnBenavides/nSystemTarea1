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
  int minIndex;
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
void swap(Subasta s, Oferente o){
	//al ofrecer, como el nuevo oferente tiene una mejor apuesta; entra a la subasta:
	// se saca el oferente desplazado (s->minIndex) y se le avisa que esta afuera (s->o[minIndex].e=afuera)
	s->o[s->minIndex]->e = afuera;
	nPrintf("\tEliminamos la oferta menor %lf \n",s->min);
	nSignalCondition(s->o[s->minIndex]->c);
	// en su lugar se pone el nuevo oferente(s->o[minIndex]=o) y se le avisa que esta adentro(o->e=dentro)
	nPrintf("\tAgregamos la nueva oferta %lf \n",o->p);
	s->o[s->minIndex]=o;
	s->min =o->p;
	// se recalcula el menor precio entre los oferentes de la subasta (menorPostor(s))
	menorPostor(s);
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
	Subasta s = nMalloc(sizeof(*s));
	s->m = nMakeMonitor();
	s->n = unidades;
	s->count = 0;
	Oferente off = nMalloc(sizeof(*off));
	s->o = nMalloc(unidades*sizeof(*off));//probar sin *
	s->minIndex = 0;
	nPrintf("... subasta abierta!\n");
	return s;
}

int ofrecer(Subasta s, double precio){
	// oferta (precio) de comprar un elemento de la subasta
	Oferente O = initOferta(s,precio);
	// esperando hasta que:
	// 1. la subasta se cierre (se llame a adjudicar) retornando TRUE
	// 2. los otros oferentes tienen mejores ofertas, retorna FALSE
	nEnter(s->m);
	if (s->count==0){ //primer oferente
		nPrintf("\t+ Ingresa primera oferta: %lf",precio);
		O->e=dentro;
		s->o[s->count]=O;
		s->min=precio;
		s->count++;
		nPrintf("... esperando ...\n");
		nWaitCondition(O->c);//(1)
		nExit(s->m);
	} 
	else if (s->count < s->n){//primeros n oferentes
		nPrintf("\t+ Ingresa nueva oferta: %lf",precio);
		O->e=dentro;
		s->o[s->count]=O;
		s->count++;
		menorPostor(s);
		nPrintf("... esperando ...\n");
		nWaitCondition(O->c);//(1)
		nExit(s->m);
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
			O->e=dentro;
			swap(s,O);
			nPrintf("... esperando ...\n");
			nWaitCondition(O->c);
			nExit(s->m);
		}
	}
	nPrintf("\tOferente despierto: ");
	if (O->e == dentro){
		nPrintf("DENTRO\n");
		return TRUE;
	} else {
		nPrintf("AFUERA\n");
		return FALSE;
	}	
}

double adjudicar(Subasta s, int *punidades){
	nPrintf("\tAdjudicar!\n");
	nEnter(s->m);
	int resto=s->n - s->count;
	nPrintf("\t\tQuedaron %lf unidades libres\n",resto);
	punidades=resto;
	double suma=0;
	for (int i=0;i < s->count;i++){
		suma= suma + s->o[i]->p;
		nSignalCondition(s->o[i]->c);//nNotifyAll penca
	}
	nExit(s->m);
	nPrintf("\t\tSuma = %lf\n",suma);
	return suma;
}