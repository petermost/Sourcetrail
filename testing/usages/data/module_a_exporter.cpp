module;

#include <iostream>

module A_Module;

void singleExportedFunctionFromModuleA()
{
	std::cout << __func__ << std::endl;
}

void blockExportedFunctionFromModuleA()
{
	std::cout << __func__ << std::endl;
}

namespace A_Namespace
{

void blockAndNamespaceExportedFunctionFromModuleA()
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
