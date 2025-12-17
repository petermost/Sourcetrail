module;

#include <iostream>

module B_Module;

void singleExportedFunctionFromModuleB()
{
	std::cout << __func__ << std::endl;
}

void blockExportedFunctionFromModuleB()
{
	std::cout << __func__ << std::endl;
}

namespace B_Namespace
{

void blockAndNamespaceExportedFunctionFromModuleB()
{
	std::cout << __func__ << std::endl;
}

}

int main(int argc, char *argv[])
{
	return 0;
}

void function()
{
}
