module;

export module A_Module;

export void singleExportedFunctionFromModuleA();

export
{
	void blockExportedFunctionFromModuleA();
}

export namespace A_Namespace
{
	void blockAndNamespaceExportedFunctionFromModuleA();
}
