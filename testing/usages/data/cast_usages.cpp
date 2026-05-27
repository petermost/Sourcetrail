void function()
{
	void *ptrFromInt = ( void * )1000;

	const int constInt = 10;
	int *ptrToMutableInt = ( int * )&constInt;

	const int *const *ptrToConstPtrToConstInt = nullptr;
	const int **ptrToPtrToConstInt = ( const int * * )ptrToConstPtrToConstInt;
}

class Class
{
public:
	void method()
	{
		int *ptrToMutableInt = ( int * )&m_constInt;

		const int **ptrToPtrToConstInt = ( const int * * )m_ptrToConstPtrToConstInt;
	}

private:
	const int m_constInt = 20;
	const int *const *m_ptrToConstPtrToConstInt = nullptr;
};
