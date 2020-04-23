/*
 * IVTEntry.h
 *
 *  Created on: Apr 8, 2020
 *      Author: OS1
 */

#ifndef IVTENTRY_H_
#define IVTENTRY_H_

#include "dos.h"

typedef unsigned char IVTNo;

typedef void interrupt (*pInterrupt)(...);

class KernelEv;

class IVTEntry{
public:

	static IVTEntry * IVTEList[];
	static IVTEntry * getIVTE(IVTNo);
	static void restore();

	void setEvent(KernelEv *);

	KernelEv * event;
	int n;
	pInterrupt oldInter, inter;

	IVTEntry(IVTNo, pInterrupt);
	~IVTEntry();
};

#endif /* IVTENTRY_H_ */

