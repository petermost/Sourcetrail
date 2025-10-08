//
// Single concept usages:
//

template <typename T>
concept Plusable = requires(T a, T b)
{
	{ a + b };
};

template<typename T>
requires Plusable<T>
T add1(T x, T y);

template<typename T>
T add2(T x, T y)
requires Plusable<T>;

template<Plusable T>
T add3(T x, T y);

auto add4(Plusable auto x, Plusable auto y);

Plusable auto add5(Plusable auto x, Plusable auto y);

auto add6 = [](Plusable auto a, Plusable auto b)
{
	return a + b;
};

template<Plusable T>
struct Adder0
{
	T add0(T a, T b);
};

struct Adder1
{
	template<typename T>
	requires Plusable<T>
	T add1(T x, T y);
};

struct Adder2
{
	template<typename T>
	T add2(T x, T y)
	requires Plusable<T>;
};

struct Adder3
{
	template<Plusable T>
	T add3(T x, T y);
};

struct Adder4
{
	auto add4(Plusable auto x, Plusable auto y);
};

struct Adder5
{
	Plusable auto add5(Plusable auto x, Plusable auto y);
};

//
// Multiple concept usages:
//

template <typename T>
concept Minusable = requires(T a, T b)
{
	{ a - b };
};

template<typename T, typename U>
requires Plusable<T> && Minusable<U>
U add10(T x, U y);

template<typename T, typename U>
U add20(T x, U y)
requires Plusable<T> && Minusable<U>;

template<Plusable T, Minusable U>
U add30(T x, U y);

auto add40(Plusable auto x, Minusable auto y);

Minusable auto add50(Plusable auto x, Minusable auto y);

auto add60 = [](Plusable auto a, Minusable auto b)
{
	return a + b;
};

template<Plusable T, Minusable U>
struct Adder00
{
	U add00(T a, U b);
};

struct Adder10
{
	template<typename T, typename U>
	requires Plusable<T> && Minusable<U>
	U add10(T x, U y);
};

struct Adder20
{
	template<typename T, typename U>
	U add20(T x, U y)
	requires Plusable<T> && Minusable<U>;
};

struct Adder30
{
	template<Plusable T, Minusable U>
	U add30(T x, U y);
};

struct Adder40
{
	auto add40(Plusable auto x, Minusable auto y);
};

struct Adder50
{
	Minusable auto add50(Plusable auto x, Minusable auto y);
};



void functionUsingConstrainedAuto()
{
	Plusable auto result = 0;
}
