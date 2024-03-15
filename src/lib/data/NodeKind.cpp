#include "NodeKind.h"

std::map<NodeKind, std::string> nodeKinds;

int nodeKindToInt(NodeKind kind)
{
	return kind;
}

NodeKind intToNodeKind(int value)
{
	switch (value)
	{
	case NODE_TYPE:
		return NODE_TYPE;
	case NODE_BUILTIN_TYPE:
		return NODE_BUILTIN_TYPE;
	case NODE_MODULE:
		return NODE_MODULE;
	case NODE_NAMESPACE:
		return NODE_NAMESPACE;
	case NODE_PACKAGE:
		return NODE_PACKAGE;
	case NODE_STRUCT:
		return NODE_STRUCT;
	case NODE_CLASS:
		return NODE_CLASS;
	case NODE_INTERFACE:
		return NODE_INTERFACE;
	case NODE_ANNOTATION:
		return NODE_ANNOTATION;
	case NODE_GLOBAL_VARIABLE:
		return NODE_GLOBAL_VARIABLE;
	case NODE_FIELD:
		return NODE_FIELD;
	case NODE_FUNCTION:
		return NODE_FUNCTION;
	case NODE_METHOD:
		return NODE_METHOD;
	case NODE_ENUM:
		return NODE_ENUM;
	case NODE_ENUM_CONSTANT:
		return NODE_ENUM_CONSTANT;
	case NODE_TYPEDEF:
		return NODE_TYPEDEF;
	case NODE_TYPE_PARAMETER:
		return NODE_TYPE_PARAMETER;
	case NODE_FILE:
		return NODE_FILE;
	case NODE_MACRO:
		return NODE_MACRO;
	case NODE_UNION:
		return NODE_UNION;
	}

	return NODE_SYMBOL;
}

std::string getReadableNodeKindString(NodeKind kind)
{
	return nodeKinds[kind];
}

std::wstring getReadableNodeKindWString(NodeKind kind)
{
	std::string str = getReadableNodeKindString(kind);
	return std::wstring(str.begin(), str.end());
}

NodeKind getNodeKindForReadableNodeKindString(const std::wstring& str)
{
	for (NodeKindMask mask = 1; mask <= NODE_MAX_VALUE; mask *= 2)
	{
		NodeKind kind = intToNodeKind(mask);
		if (getReadableNodeKindWString(kind) == str)
		{
			return kind;
		}
	}

	return NODE_SYMBOL;
}
