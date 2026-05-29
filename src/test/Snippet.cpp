// TODO (petermost): Move these snippets to 'testing/code_view' or 'testing/usages'
// These snippets are meant to test syntax highlighting i.e. 'bin/app/data/syntax_highlighting_rules/cpp.rules'
/*
ää ÄÄ
😀😀😀😀
ßßß😀😀😀😀
ßßß
*/

#include <string>

[[maybe_unused]]
static const int fileScopeConst = 0;

[[maybe_unused]]
static constexpr int fileScopeConstExpr = 0;

[[maybe_unused]]
static constexpr char singleLineRawString[] = R"(Some single line raw string)";

[[maybe_unused]]
static constexpr char multiLineRawString[] = R"(
	class A {
	};

	int f()
	{
		A a;
	}
)";

[[maybe_unused]]
static constexpr int integerLiterals[] = {
	42,
	42u,
	42U,

	052,
	052u,
	052U,

	0x2A,
	0x2Au,
	0x2AU,
	0X2Au,
	0X2AU,

	0b101010,
	0B101010,

	42l,
	42L,

	42ll,
	42LL

	// 42z,
	// 42Z
};

[[maybe_unused]]
static constexpr float floatLiteral = 13.4f;

[[maybe_unused]]
static constexpr double doubleLiteral = 13.4;

[[maybe_unused]]
static constexpr double scientificLiteral_1 = 1.22e11;

[[maybe_unused]]
static constexpr double scientificLiteral_2 = 1.22e-11;

void callFunction(const std::string &)
{
}

void testFunctionForTooltipWithUmlauteInSignature(/* äöüÄÖÜß€😀 */ )
{
	[[maybe_unused]]
	constexpr int functionScopeConstExpr = 0;
	
	[[maybe_unused]]
	int functionScopeVariable = functionScopeConstExpr;

	callFunction(R"(
		Multi
		line
		comment
	)");
}
