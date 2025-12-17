module;

export module B_Module;

export void singleExportedFunctionFromModuleB();

export
{
	void blockExportedFunctionFromModuleB();
}

export namespace B_Namespace
{
	void blockAndNamespaceExportedFunctionFromModuleB();
}

