#include <nSystem.h>
#include <stdlib.h>
#include "subasta.h"

typedef enum{
	dentro, afuera;
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

int compararPrecios(Subasta s,Oferente o){
	//tomando el precio minimo ofertado en la subasta (s->min)
	// y que este adentro comparamos los precios
	// 0 si son iguales
	if (s->min==o->p)
		return 0;
	// 1 si precio es mayor al minimo
	if (s->min>o->p)
		return 1;
	// -1 se precio es menor al minimo
	else 
		return -1;
}

void menorPostor(Subasta s){
	// se calcula el menor precio (s->min) entre los oferentes(s->o[:]) de la subasta
	// y se guarda el minimo entre estos (s->minIndex)
	for (int i=0;i < s->count;i++){
		if(s->o[i]->p < s->min){
			s-> min = s->o[i]->p;
			s-> minIndex = i;
		}
	}
	nPrintf("Nueva menor oferta: %d \n",s->min);
}
int swap(Subasta s, Oferente o){
	//al ofrecer, como el nuevo oferente tiene una mejor apuesta; entra a la subasta:
	// se saca el oferente desplazado (s->minIndex) y se le avisa que esta afuera (s->o[minIndex].e=afuera)
	s->o[minIndex]->e = afuera;
	nPrintf("Eliminamos la oferta menor %d /n",s->min);
	nSignalCondition(s->o[minIndex]->c);
	// en su lugar se pone el nuevo oferente(s->o[minIndex]=o) y se le avisa que esta adentro(o->e=dentro)
	o->e=dentro;
	nPrintf("Agregamos la nueva oferta %d /n",o->p);
	s->o[minIndex]=o;
	// se recalcula el menor precio entre los oferentes de la subasta (menorPostor(s))
	menorPostor(s);
}

Oferente initOferta(Subasta s, double precio){
	Oferente o= (Oferente)nMalloc(sizeof(*o));
	o->e=afuera;
	o->p = precio;
	o->c = nMakeCondition(s->m);
	return o;
	nPrintf("Registrando oferta:\n");
}

double sumarPrecios(Subasta s){
	double suma=0;
	for (int i=0;i < s->count;i++){
		suma= suma+s->o[i]->p;
	}
	nPrintf("Subasta recaudó %d",suma);
	return suma;
}

// Programe aca las funciones nuevaSubasta, ofrecer y adjudicar

Subasta nuevaSubasta(int unidades){
	nPrintf("Creando nueva subasta con %d elementos",unidades);
	Subasta s = (Subasta)nMalloc(sizeof(*s));
	s->m = nMakeMonitor();
	s->n = unidades;
	s->count = 0;
	s->o = (Oferente)nMalloc(unidades*sizeof(*Oferente));//probar sin *
	s->minIndex = 0;
	nPrintf("... subasta abierta!\n");
}

int ofrecer(Subasta s, double precio){
	// oferta (precio) de comprar un elemento de la subasta
	Oferente O= initOferta(s,precio);
	// esperando hasta que:
	// 1. la subasta se cierre (se llame a adjudicar) retornando TRUE
	// 2. los otros oferentes tienen mejores ofertas, retorna FALSE
	nEnter(s->m);
	if (s->count==0){ //primer oferente
		nPrintf("+ Ingresa primera oferta: %d",precio);
		O->e=dentro;
		s->o[s->count]=O;
		s->min=precio;
		s->count++;
		nPrintf("... esperando ...\n");
		nWaitCondition(O->c);//(1)
	} 
	else if (s->count < s->n){//primeros n oferentes
		nPrintf("+ Ingresa nueva oferta: %d",precio);
		O->e=dentro;
		s->o[s->count]=O;
		s->count++;
		menorPostor(s);
		nPrintf("... esperando ...\n");
		nWaitCondition(O->c);//(1)
	} else {
		//comparar oferta con las ya existentes
		int comp = compararPrecios(s,o);
		// si tiene una oferta menor retorna False (2)
		if (comp<=0){
			nPrintf("- Oferta rechazada\n");
			O->e=afuera;
			nExit(s->m);
			return FALSE;
		}
		// si tiene una apuesta mayor expulsa al menor oferente y entra el (1)
		else{
			nPrintf("+ Cambiar oferentes en la subasta ");
			O->e=dentro;
			swap(s,O);
			nPrintf("... esperando ...\n");
			nWaitCondition(O->c);
		}
	}
	nDestroyCondition(O->c);
	nPrintf("Oferente despierto: ");
	nExit(s->m);
	if( O->e == dentro)
		return TRUE;
	else
		return FALSE;
	nPrintf("\n");
}

double adjudicar(Subasta s, int *punidades){
	nPrintf("Adjudicar!\n");
	nEnter(s->m);
	*punidades=s->n - s->count;
	int suma=sumarPrecios(s);
	for (int i=0;i < s->count;i++)
		nSignalCondition(s->o[i]->c);//nNotifyAll penca
	nExit(s->m);
	return suma;
	nPrintf("Adjudicar finalizado\n");
}