import A_Module;
import B_Module;

void functionUsingModules()
{
	singleExportedFunctionFromModuleA();
	blockExportedFunctionFromModuleA();
	A_Namespace::blockAndNamespaceExportedFunctionFromModuleA();

	singleExportedFunctionFromModuleB();
	blockExportedFunctionFromModuleB();
}

int main(int argc, char *argv[])
{
	return 0;
}

void function()
{
}
