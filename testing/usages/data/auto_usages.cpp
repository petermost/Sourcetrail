#include <string>

struct Result
{
	double dvalue = 0.0;
	char cvalue = 0;
	int ivalue = 0;
};

Result acceptReturnResult(const Result &r)
{
	return r;
}

auto returnResult()
{
	return Result(3.14);
}

auto returnResult2() -> Result
{
	return Result { 3.14 };
}

Result returnResult3()
{
	return Result { 3.14 };
}

auto returnDouble()
{
	return 3.14;
}

auto returnInt() -> int
{
	return 3;
}

decltype(auto) returnChar()
{
	return '3';
}

std::string returnString()
{
	return "3.14";
}

void callFunction(const std::string &s1)
{
	auto copy = s1;
	auto *sp = &copy;

	int i = 0;
	auto *ip = &i;

	auto i2 = 0;

	auto [v1, v2, v3] = Result { 3.14, '3', 3 };
}

void functionUsingAuto()
{
	auto auto_double_var1 = 0.0;
	auto auto_double_var2 = auto_double_var1;

	auto auto_int_var = 0;
	auto auto_int_ptr1 = &auto_int_var;
	auto *auto_int_ptr2 = &auto_int_var;
	auto &auto_int_ref = auto_int_var;

	int *int_ptr = &auto_int_var;
	int &int_ref = auto_int_var;
}

void functionUsingDecltypeAuto()
{
	decltype(auto) auto_double_var1 = 0.0;
	decltype(auto) auto_double_var2 = auto_double_var1;

	decltype(auto) auto_int_var = 0;
	decltype(auto) auto_int_ptr1 = &auto_int_var;
	decltype(auto) auto_int_ptr2 = &auto_int_var;
	decltype(auto) auto_int_ref = auto_int_var;

	int *int_ptr = &auto_int_var;
	int &int_ref = auto_int_var;
}

template <typename C, typename R>
R functionUsingAuto2(const C &c)
{
	// This auto must not be indexed!
	auto it = c.begin();

	return it;
}
