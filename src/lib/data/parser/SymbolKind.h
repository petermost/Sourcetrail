#ifndef SYMBOL_KIND_H
#define SYMBOL_KIND_H

#include "utilityEnum.h"

// These values need to be the same as SymbolKind in Java code (see SymbolKind.java)
enum class SymbolKind
{	 
	UNDEFINED = 0,
	
	ANNOTATION = 1,
	BUILTIN_TYPE = 2,
	CLASS = 3,
	ENUM = 4,
	ENUM_CONSTANT = 5,
	FIELD = 6,
	FUNCTION = 7,
	GLOBAL_VARIABLE = 8,
	INTERFACE = 9,
	MACRO = 10,
	METHOD = 11,
	MODULE = 12,
	NAMESPACE = 13,
	PACKAGE = 14,
	STRUCT = 15,
	TYPEDEF = 16,
	TYPE_PARAMETER = 17,
	UNION = 18,
	RECORD = 19,
	CONCEPT = 20
};

template <>
SymbolKind intToEnum(int value);

#endif	  // SYMBOL_KIND_H
