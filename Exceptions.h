#ifndef EXCEPT_H
#define EXCEPT_H

#include <YRPP.h>
#include <MacroHelpers.h> //basically indicates that this is DCoder country

class Exceptions
{
public:
	static char* PointerToText(DWORD ptr, char* out);
};

#endif
