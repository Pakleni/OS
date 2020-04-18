#include "IVTEntry.h"
#include "dos.h"
#define N_INTER 256

IVTEntry * IVTEntry::IVTEList[N_INTER] = {0};

IVTEntry::IVTEntry(IVTNo ivtNo, pInterrupt i): n(ivtNo), oldInter(0), inter(i), event(0){
#ifndef BCC_BLOCK_IGNORE
	oldInter = getvect(n);
	IVTEList[n] = this;
	setvect(n, i);
#endif
}

IVTEntry::~IVTEntry(){
#ifndef BCC_BLOCK_IGNORE
	setvect(n,oldInter);
#endif
}

void IVTEntry::restore(){

	for (int i = 0; i < N_INTER; i++){
		delete IVTEList[i];
	}

	delete[] IVTEList;
}

IVTEntry * IVTEntry::getIVTE(IVTNo n){
	return IVTEList[n];
}

void IVTEntry::setEvent(KernelEv * e){
	event = e;
}
