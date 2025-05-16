#include "Catch2.hpp"

#include "language_packages.h"

#if BUILD_JAVA_LANGUAGE_PACKAGE

#	include "ApplicationSettings.h"
#	include "JavaEnvironmentFactory.h"
#	include "JavaParser.h"
#	include "ParserClientImpl.h"
#	include "TextAccess.h"
#	include "utility.h"
#	include "utilityJava.h"
#	include "utilityPathDetection.h"

#	include "TestStorage.h"

using namespace std;
using namespace string_literals;
using namespace utility;

namespace
{
std::string setupJavaEnvironmentFactory()
{
	return utility::prepareJavaEnvironment(FilePath("../app/data/java/"));
}

std::shared_ptr<TestStorage> parseCode(std::string code, bool /*logErrors*/ = true)
{
	setupJavaEnvironmentFactory();

	std::shared_ptr<IntermediateStorage> storage = std::make_shared<IntermediateStorage>();
	JavaParser parser(std::make_shared<ParserClientImpl>(storage), std::make_shared<IndexerStateInfo>());
	parser.buildIndex(FilePath("input.java"), TextAccess::createFromString(code));

	return TestStorage::create(storage);
}
}	 // namespace

///////////////////////////////////////////////////////////////////////////////
// test finding symbol definitions and declarations

TEST_CASE("java parser finds all jar dependencies")
{
	for (const std::string& jarName: utility::getRequiredJarNames())
	{
		FilePath jarPath = FilePath("../app/data/java/lib/").concatenate(jarName);
		REQUIRE_MESSAGE("Jar dependency path does not exist: " + jarPath.str(), jarPath.exists());
	}
}

TEST_CASE("java parser can setup environment factory")
{
	std::vector<FilePath> javaPaths = utility::getJavaRuntimePathDetector()->getPaths();
	if (!javaPaths.empty())
	{
		ApplicationSettings::getInstance()->setJavaPath(javaPaths[0]);
	}

	const std::string errorString = setupJavaEnvironmentFactory();

	REQUIRE("" == errorString);

	// if this one fails, maybe your java_path in the test settings is wrong.
	REQUIRE(JavaEnvironmentFactory::getInstance().use_count() >= 1);
}

TEST_CASE("java parser finds package declaration", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode("package foo;\n");

	REQUIRE(utility::containsElement<std::string>(client->packages, "foo <1:9 1:11>"));
}

TEST_CASE("java parser finds anotation declaration in defaut package", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public @interface SampleAnnotation\n"
		"{\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->annotations, "public SampleAnnotation <1:1 <1:19 1:34> 3:1>"));
}

TEST_CASE("java parser finds anotation member declaration", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public @interface SampleAnnotation\n"
		"{\n"
		"	public int value() default 0;\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->fields, "public int SampleAnnotation.value <3:13 3:17>"));
}

TEST_CASE("java parser finds class declaration in default package", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"}\n");

	REQUIRE(
		utility::containsElement<std::string>(client->classes, "public A <1:1 <1:14 1:14> 3:1>"));
}

TEST_CASE("java parser finds interface declaration in default package", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public interface A\n"
		"{\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->interfaces, "public A <1:1 <1:18 1:18> 3:1>"));
}

TEST_CASE("java parser finds record declaration in default package", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public record A()\n"
		"{\n"
		"}\n");

	REQUIRE(containsElement(client->records, "public A <1:1 <1:15 1:15> 3:1>"s));
}

TEST_CASE("java parser finds record declaration in named package", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public record A()\n"
		"{\n"
		"}\n");

	REQUIRE(containsElement(client->records, "public foo.A <2:1 <2:15 2:15> 4:1>"s));
}

TEST_CASE("java parser finds record declaration in nested named package", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo.bar;\n"
		"public record A()\n"
		"{\n"
		"}\n");

	REQUIRE(containsElement(client->records, "public foo.bar.A <2:1 <2:15 2:15> 4:1>"s));
}

TEST_CASE("java parser finds class declaration in named package", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class A\n"
		"{\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->classes, "public foo.A <2:1 <2:14 2:14> 4:1>"));
}

TEST_CASE("java parser finds class declaration in nested named package", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo.bar;\n"
		"public class A\n"
		"{\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->classes, "public foo.bar.A <2:1 <2:14 2:14> 4:1>"));
}

TEST_CASE("java parser finds enum declaration in named package", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public enum A\n"
		"{\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->enums, "public foo.A <2:1 <2:13 2:13> 4:1>"));
}

TEST_CASE("java parser finds enum constant declaration", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public enum A\n"
		"{\n"
		"	A_TEST(0)\n"
		"}\n");

	REQUIRE(
		utility::containsElement<std::string>(client->enumConstants, "foo.A.A_TEST <4:2 4:7>"));
}

TEST_CASE("java parser finds constructor declaration without parameters", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class A\n"
		"{\n"
		"	public A()\n"
		"	{\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->methods, "public foo.A.A() <4:2 <4:2 <4:9 4:9> 4:11> 6:2>"));
}

TEST_CASE("java parser finds method declaration with custom type in signature", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class A\n"
		"{\n"
		"	public void bar(A a)\n"
		"	{\n"
		"	};\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->methods, "public void foo.A.bar(foo.A) <4:2 <4:2 <4:14 4:16> 4:21> 6:2>"));
}

TEST_CASE("java parser finds anonymous class declaration", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class A\n"
		"{\n"
		"	interface I { }\n"
		"	public void bar()\n"
		"	{\n"
		"		I i = new I() { };\n"
		"	};\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->classes, "foo.A.bar.anonymous class (input.java<7:17>) <7:17 <7:17 7:17> 7:19>"));
}

TEST_CASE("java parser finds method declaration in anonymous class", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class A\n"
		"{\n"
		"	interface I {\n"
		"		public void foo();\n"
		"	}\n"
		"	public void bar()\n"
		"	{\n"
		"		I i = new I()\n"
		"		{\n"
		"			public void foo() {}\n"
		"		};\n"
		"	};\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->methods,
		"public void foo.A.bar.anonymous class (input.java<10:3>).foo() <11:4 <11:4 <11:16 11:18> "
		"11:20> 11:23>"));
}

TEST_CASE("java parser finds method declaration with static keyword in signature", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class A\n"
		"{\n"
		"	static public void bar()\n"
		"	{\n"
		"	};\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->methods, "public static void foo.A.bar() <4:2 <4:2 <4:21 4:23> 4:25> 6:2>"));
}

TEST_CASE("java parser finds field declaration with initial assignment", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class A\n"
		"{\n"
		"	int bar = 0;\n"
		"}\n");

	REQUIRE(
		utility::containsElement<std::string>(client->fields, "default int foo.A.bar <4:6 4:8>"));
}

TEST_CASE("java parser finds public access specifier in field declaration", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class A\n"
		"{\n"
		"	public int bar;\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->fields, "public int foo.A.bar <4:13 4:15>"));
}

TEST_CASE("java parser finds protected access specifier in field declaration", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class A\n"
		"{\n"
		"	protected int bar;\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->fields, "protected int foo.A.bar <4:16 4:18>"));
}

TEST_CASE("java parser finds private access specifier in field declaration", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class A\n"
		"{\n"
		"	private int bar;\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->fields, "private int foo.A.bar <4:14 4:16>"));
}

TEST_CASE("java parser finds static keyword in field declaration", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class A\n"
		"{\n"
		"	static int bar;\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->fields, "default static int foo.A.bar <4:13 4:15>"));
}

TEST_CASE("java parser finds declaration of method parameter", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class A\n"
		"{\n"
		"	void bar(int i)\n"
		"	{\n"
		"	}\n"
		"}\n");

	REQUIRE(
		utility::containsElement<std::string>(client->localSymbols, "foo.A.bar<0> <4:15 4:15>"));
}

TEST_CASE("java parser finds declaration of local variable", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class A\n"
		"{\n"
		"	void bar()\n"
		"	{\n"
		"		int i;\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(client->localSymbols, "foo.A.bar<0> <6:7 6:7>"));
}

TEST_CASE("java parser finds declaration of type parameter of class", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A <T>\n"
		"{\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(client->typeParameters, "A<T>.T <1:17 1:17>"));
}

TEST_CASE("java parser finds declaration of type parameter of method", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"	public <T> void foo()\n"
		"	{\n"
		"	}\n"
		"}\n");

	REQUIRE(
		utility::containsElement<std::string>(client->typeParameters, "A.foo<T>.T <3:10 3:10>"));
}

TEST_CASE("java parser finds field of interface to be implicitly static", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public interface A\n"
		"{\n"
		"	int b = 5;\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->fields, "default static int A.b <3:6 3:6>"));
}

TEST_CASE("java parser finds line comment", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"// this is a line comment\n"
		"package foo;\n");

	REQUIRE(utility::containsElement<std::string>(client->comments, "comment <1:1 1:25>"));
}

TEST_CASE("java parser finds block comment", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"/* this is a line comment*/\n"
		"package foo;\n");

	REQUIRE(utility::containsElement<std::string>(client->comments, "comment <1:1 1:27>"));
}

TEST_CASE("java parser finds missing semicolon as parse error", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode("package foo\n");

	REQUIRE(utility::containsElement<std::string>(
		client->errors, "Syntax error on token \"foo\", ; expected after this token <1:9 1:9>"));
}

TEST_CASE("java parser finds missing import as error", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode("import foo;\n");

	REQUIRE(utility::containsElement<std::string>(
		client->errors, "The import foo cannot be resolved <1:8 1:8>"));
}


///////////////////////////////////////////////////////////////////////////////
// test finding nested symbol definitions and declarations

TEST_CASE("java parser finds class declaration nested in class", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo.bar;\n"
		"public class A\n"
		"{\n"
		"	public class B\n"
		"	{\n"
		"	}\n"
		"};\n");

	REQUIRE(utility::containsElement<std::string>(
		client->classes, "public foo.bar.A.B <4:2 <4:15 4:15> 6:2>"));
}

TEST_CASE("java parser finds class declaration nested in method", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo.bar;\n"
		"public class A\n"
		"{\n"
		"	public void bar(int i)\n"
		"	{\n"
		"		class B\n"
		"		{\n"
		"		}\n"
		"	};\n"
		"};\n");

	REQUIRE(utility::containsElement<std::string>(
		client->classes, "default foo.bar.A.bar.B <6:3 <6:9 6:9> 8:3>"));
}


///////////////////////////////////////////////////////////////////////////////
// test finding qualifier locations

TEST_CASE("java parser finds no qualifier location of standalone this expression", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class X\n"
		"{\n"
		"	public void bar()\n"
		"	{\n"
		"		X x = this;\n"
		"	}\n"
		"}\n");

	REQUIRE(client->qualifiers.size() == 0);
}

TEST_CASE("java parser finds qualifier location of import declaration", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode("import foo.bar;\n");

	REQUIRE(utility::containsElement<std::string>(client->qualifiers, "foo <1:8 1:10>"));
}

TEST_CASE("java parser finds qualifier location of simple type", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo.bar;\n"
		"public class A\n"
		"{\n"
		"	public void bar(int i)\n"
		"	{\n"
		"		foo.bar.A a;\n"
		"	};\n"
		"};\n");

	REQUIRE(utility::containsElement<std::string>(client->qualifiers, "foo <6:3 6:5>"));
	REQUIRE(utility::containsElement<std::string>(client->qualifiers, "foo.bar <6:7 6:9>"));
}

TEST_CASE("java parser finds qualifier location of field access", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class X\n"
		"{\n"
		"	public int i;\n"
		"	\n"
		"	public void bar()\n"
		"	{\n"
		"		this.i = 9;\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(client->qualifiers, "foo.X <8:3 8:6>"));
}

TEST_CASE("java parser finds qualifier location of super field access", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"class A\n"
		"{\n"
		"	int a;\n"
		"}\n"
		"\n"
		"class B extends A\n"
		"{\n"
		"	void foo()\n"
		"	{\n"
		"		B.super.a = 0;\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(client->qualifiers, "foo.B <11:3 11:3>"));

	REQUIRE(utility::containsElement<std::string>(client->qualifiers, "foo.A <11:5 11:9>"));
}

TEST_CASE("java parser finds qualifier location of this expression", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"class A\n"
		"{\n"
		"	int a;\n"
		"	\n"
		"	void foo()\n"
		"	{\n"
		"		A a = A.this;"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(client->qualifiers, "foo.A <8:9 8:9>"));
}

TEST_CASE("java parser finds qualifier location of method invocation", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class X\n"
		"{\n"
		"	public static void bar()\n"
		"	{\n"
		"		foo.X.bar();\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(client->qualifiers, "foo <6:3 6:5>"));

	REQUIRE(utility::containsElement<std::string>(client->qualifiers, "foo.X <6:7 6:7>"));
}

TEST_CASE("java parser finds qualifier location of method invocation on this", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class X\n"
		"{\n"
		"	public void bar()\n"
		"	{\n"
		"		this.bar();\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(client->qualifiers, "foo.X <6:3 6:6>"));
}

TEST_CASE("java parser finds qualifier location of super method invocation", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class X\n"
		"{\n"
		"	public class A\n"
		"	{\n"
		"		void bar()\n"
		"		{\n"
		"		}\n"
		"	}\n"
		"	\n"
		"	public class B extends A\n"
		"	{\n"
		"		void bar()\n"
		"		{\n"
		"			foo.X.B.super.bar();\n"
		"		}\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(client->qualifiers, "foo <15:4 15:6>"));

	REQUIRE(utility::containsElement<std::string>(client->qualifiers, "foo.X <15:8 15:8>"));

	REQUIRE(utility::containsElement<std::string>(client->qualifiers, "foo.X.B <15:10 15:10>"));

	REQUIRE(utility::containsElement<std::string>(client->qualifiers, "foo.X.A <15:12 15:16>"));
}

TEST_CASE("java parser finds qualifier location of creation reference", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"	public interface Functor\n"
		"	{\n"
		"		public void doSomething();\n"
		"	}\n"
		"	\n"
		"	public class Bar\n"
		"	{\n"
		"	}\n"
		"	\n"
		"	void foo()\n"
		"	{\n"
		"		Functor method = A.Bar::new;\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(client->qualifiers, "A <14:20 14:20>"));

	REQUIRE(utility::containsElement<std::string>(client->qualifiers, "A.Bar <14:22 14:24>"));
}

TEST_CASE("java parser finds qualifier location of expression method reference", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"	public interface Functor\n"
		"	{\n"
		"		public void doSomething();\n"
		"	}\n"
		"\n"
		"	public class B\n"
		"	{\n"
		"		void bar()\n"
		"		{\n"
		"		}\n"
		"	}\n"
		"\n"
		"	void foo()\n"
		"	{\n"
		"		Functor method = B::bar;\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(client->qualifiers, "A.B <17:20 17:20>"));
}

TEST_CASE("java parser finds qualifier location of super method reference", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A {\n"
		"	public interface Functor {\n"
		"		public void doSomething();\n"
		"	}\n"
		"\n"
		"	public class B {\n"
		"		void bar() {\n"
		"		}\n"
		"	}\n"
		"\n"
		"	public class C extends B {\n"
		"		void foo() {\n"
		"			Functor method = super::bar;\n"
		"		}\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(client->qualifiers, "A.B <13:21 13:25>"));
}

TEST_CASE("java parser finds qualifier location of class instance creation", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A {\n"
		"	public interface Functor {\n"
		"		public void doSomething();\n"
		"	}\n"
		"\n"
		"	public class B {\n"
		"		void bar() {\n"
		"			B b = new A.B();\n"
		"		}\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(client->qualifiers, "A <8:14 8:14>"));
}


///////////////////////////////////////////////////////////////////////////////
// test finding usages of symbols

TEST_CASE("java parser finds usage of marker annotation", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public @interface SampleAnnotation\n"
		"{\n"
		"}\n"
		"\n"
		"@SampleAnnotation\n"
		"class Foo\n"
		"{\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->annotationUses, "Foo -> SampleAnnotation <5:2 5:17>"));
}

TEST_CASE("java parser finds usage of single member annotation", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public @interface SampleAnnotation\n"
		"{\n"
		"	public int value() default 0;\n"
		"}\n"
		"\n"
		"@SampleAnnotation(33)\n"
		"class Foo\n"
		"{\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->annotationUses, "Foo -> SampleAnnotation <6:2 6:17>"));
}

TEST_CASE("java parser finds usage of normal annotation", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public @interface SampleAnnotation\n"
		"{\n"
		"	public int a() default 0;\n"
		"	public int b() default 0;\n"
		"}\n"
		"\n"
		"@SampleAnnotation()\n"
		"class Foo\n"
		"{\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->annotationUses, "Foo -> SampleAnnotation <7:2 7:17>"));
}

TEST_CASE("java parser finds usage of normal annotation member in initialization", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public @interface SampleAnnotation\n"
		"{\n"
		"	public int a() default 0;\n"
		"	public int b() default 0;\n"
		"}\n"
		"\n"
		"@SampleAnnotation(a = 9)\n"
		"class Foo\n"
		"{\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->usages, "Foo -> int SampleAnnotation.a <7:19 7:19>"));
}

TEST_CASE("java parser finds inheritance using extends keyword", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class A\n"
		"{\n"
		"}\n"
		"\n"
		"public class B extends A\n"
		"{\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->inheritances, "foo.B -> foo.A <6:24 6:24>"));
}

TEST_CASE("java parser finds inheritance using implements keyword", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class A\n"
		"{\n"
		"}\n"
		"\n"
		"public class B implements A\n"
		"{\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->inheritances, "foo.B -> foo.A <6:27 6:27>"));
}

TEST_CASE("java parser finds inheritance of anonymous class declaration", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"	public interface Base\n"
		"	{\n"
		"	}\n"
		"\n"
		"	void foo()\n"
		"	{\n"
		"		Base b = new Base()\n"
		"		{\n"
		"		}\n"
		"	};\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->inheritances, "A.foo.anonymous class (input.java<10:3>) -> A.Base <9:16 9:19>"));
}

TEST_CASE("java parser finds usage of string for var type", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"	public void foo(){\n"
		"		var a = \"test\";\n"
		"	}\n"
		"}\n");

	REQUIRE(containsElement(client->typeUses, "void A.foo() -> java.lang.String <4:3 4:5>"s));
}

TEST_CASE("java parser finds type parameter in signature of method", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A <T>\n"
		"{\n"
		"	public A<Void> foo(A<Void> a){\n"
		"		return a;\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->typeUses, "A<java.lang.Void> A<T>.foo(A<java.lang.Void>) -> A<T> <3:9 3:9>"));
	REQUIRE(utility::containsElement<std::string>(
		client->typeUses, "A<java.lang.Void> A<T>.foo(A<java.lang.Void>) -> A<T> <3:21 3:21>"));
}

TEST_CASE("parser finds usage of type defined in base class", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class Foo {\n"
		"	public class Base {\n"
		"		public class X {\n"
		"		}\n"
		"	}\n"
		"	public class Derived extends Base {\n"
		"		public X x = null;\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->typeUses, "Foo.Base.X Foo.Derived.x -> Foo.Base.X <7:10 7:10>"));
}

TEST_CASE("java parser finds correct location of qualified type usage", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"	public class B\n"
		"	{\n"
		"	}\n"
		"	void bar()\n"
		"	{\n"
		"		A.B b = new A.B();\n"
		"	}\n"
		"}\n");

	REQUIRE(
		utility::containsElement<std::string>(client->typeUses, "void A.bar() -> A.B <8:5 8:5>"));
}

TEST_CASE("java parser finds type argument of parameterized type", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A <T> {\n"
		"	T t;\n"
		"}\n"
		"public class B {\n"
		"	void foo() {\n"
		"		A<Void> a = null;\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->typeArguments, "A<T> -> java.lang.Void <6:5 6:8>"));

	REQUIRE(utility::containsElement<std::string>(
		client->typeUses, "void B.foo() -> java.lang.Void <6:5 6:8>"));
}

TEST_CASE("java parser finds type argument of method invocation", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class X\n"
		"{\n"
		"	public static void foo() {\n"
		"	}\n"
		"\n"
		"	public static void bar() {\n"
		"		foo.X.<Void>foo();\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->typeArguments, "static void foo.X.foo() -> java.lang.Void <8:10 8:13>"));

	REQUIRE(utility::containsElement<std::string>(
		client->typeUses, "static void foo.X.bar() -> java.lang.Void <8:10 8:13>"));
}

TEST_CASE("java parser finds type argument of super method invocation", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class X\n"
		"{\n"
		"	public class A\n"
		"	{\n"
		"		void bar()\n"
		"		{\n"
		"		}\n"
		"	}\n"
		"	\n"
		"	public class B extends A\n"
		"	{\n"
		"		void bar()\n"
		"		{\n"
		"			super.<Void>bar();\n"
		"		}\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->typeArguments, "void foo.X.A.bar() -> java.lang.Void <15:11 15:14>"));

	REQUIRE(utility::containsElement<std::string>(
		client->typeUses, "void foo.X.B.bar() -> java.lang.Void <15:11 15:14>"));
}

TEST_CASE("java parser finds type argument of constructor invocation", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class Bar\n"
		"{\n"
		"	public Bar()\n"
		"	{\n"
		"	}\n"
		"	\n"
		"	public Bar(int i)\n"
		"	{\n"
		"		<Void>this();\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->typeArguments, "foo.Bar.Bar() -> java.lang.Void <10:4 10:7>"));

	REQUIRE(utility::containsElement<std::string>(
		client->typeUses, "foo.Bar.Bar(int) -> java.lang.Void <10:4 10:7>"));
}

TEST_CASE("java parser finds type argument of super constructor invocation", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"	public class Base\n"
		"	{\n"
		"	}\n"
		"\n"
		"	public class Derived extends Base\n"
		"	{\n"
		"		public Derived()\n"
		"		{\n"
		"			<Void>super();\n"
		"		}\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->typeArguments, "A.Base.Base() -> java.lang.Void <11:5 11:8>"));

	REQUIRE(utility::containsElement<std::string>(
		client->typeUses, "A.Derived.Derived() -> java.lang.Void <11:5 11:8>"));
}

TEST_CASE("java parser finds type argument of creation reference", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"	public interface Functor\n"
		"	{\n"
		"		public void doSomething();\n"
		"	}\n"
		"\n"
		"	public class B\n"
		"	{\n"
		"	}\n"
		"\n"
		"	void foo()\n"
		"	{\n"
		"		Functor method = B::<Void>new;\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->typeArguments, "A.B.B() -> java.lang.Void <14:24 14:27>"));

	REQUIRE(utility::containsElement<std::string>(
		client->typeUses, "void A.foo() -> java.lang.Void <14:24 14:27>"));
}

TEST_CASE("java parser finds type argument of expression method reference", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"	public interface Functor\n"
		"	{\n"
		"		public void doSomething();\n"
		"	}\n"
		"\n"
		"	static void bar()\n"
		"	{\n"
		"	}\n"
		"\n"
		"	void foo()\n"
		"	{\n"
		"		Functor method = A::<Void>bar;\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->typeArguments, "static void A.bar() -> java.lang.Void <14:24 14:27>"));

	REQUIRE(utility::containsElement<std::string>(
		client->typeUses, "void A.foo() -> java.lang.Void <14:24 14:27>"));
}

TEST_CASE("java parser finds type argument of super method reference", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A {\n"
		"	public interface Functor {\n"
		"		public void doSomething();\n"
		"	}\n"
		"\n"
		"	public class B {\n"
		"		<T> void bar() {\n"
		"		}\n"
		"	}\n"
		"\n"
		"	public class C extends B {\n"
		"		void foo() {\n"
		"			Functor method = super::<Void>bar;\n"
		"		}\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->typeArguments, "void A.B.bar<T>() -> java.lang.Void <13:29 13:32>"));

	REQUIRE(utility::containsElement<std::string>(
		client->typeUses, "void A.C.foo() -> java.lang.Void <13:29 13:32>"));
}

TEST_CASE("java parser finds type argument of type method reference", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A {\n"
		"	\n"
		"	void foo() {\n"
		"		Functor method = int []::<Void>clone;\n"
		"	}\n"
		"}\n");

	// currently we cannot record the typeArguments of array type methods

	REQUIRE(utility::containsElement<std::string>(
		client->typeUses, "void A.foo() -> java.lang.Void <4:29 4:32>"));
}

TEST_CASE("java parser finds type argument of class instance creation", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A {\n"
		"	public interface Functor {\n"
		"		public void doSomething();\n"
		"	}\n"
		"\n"
		"	public class B {\n"
		"		<T> B() {\n"
		"		}\n"
		"		void bar() {\n"
		"			B b = new <Void>A.B();\n"
		"		}\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->typeArguments, "A.B.B<T>() -> java.lang.Void <10:15 10:18>"));

	REQUIRE(utility::containsElement<std::string>(
		client->typeUses, "void A.B.bar() -> java.lang.Void <10:15 10:18>"));
}

TEST_CASE("java parser finds super method invocation", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class X\n"
		"{\n"
		"	public class A\n"
		"	{\n"
		"		void bar()\n"
		"		{\n"
		"		}\n"
		"	}\n"
		"	\n"
		"	public class B extends A\n"
		"	{\n"
		"		void bar()\n"
		"		{\n"
		"			super.bar();\n"
		"		}\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->calls, "void foo.X.B.bar() -> void foo.X.A.bar() <15:10 15:12>"));
}

TEST_CASE("java parser finds constructor invocation", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class Bar\n"
		"{\n"
		"	public Bar()\n"
		"	{\n"
		"	}\n"
		"	\n"
		"	public Bar(int i)\n"
		"	{\n"
		"		this();\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->calls, "foo.Bar.Bar(int) -> foo.Bar.Bar() <10:3 10:6>"));
}

TEST_CASE("java parser finds super constructor invocation", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"	public class Base\n"
		"	{\n"
		"	}\n"
		"\n"
		"	public class Derived extends Base\n"
		"	{\n"
		"		public Derived()\n"
		"		{\n"
		"			super();\n"
		"		}\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->calls, "A.Derived.Derived() -> A.Base.Base() <11:4 11:8>"));
}

TEST_CASE("java parser finds invocation of method of anonymous class", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"class Main {\n"
		"	public interface Interfaze {\n"
		"		public void foo();\n"
		"	}\n"
		"\n"
		"	private Interfaze i = new Interfaze() {\n"
		"		public void foo() {\n"
		"			bar();\n"
		"		}\n"
		"\n"
		"		private void bar() {\n"
		"		}\n"
		"	};\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->calls,
		"void Main.anonymous class (input.java<6:40>).foo() -> void Main.anonymous class "
		"(input.java<6:40>).bar() <8:4 8:6>"));
}

TEST_CASE("java parser finds overridden method with same signature", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"class Main {\n"
		"	public interface Interfaze {\n"
		"		public void foo(int t);\n"
		"	}\n"
		"\n"
		"	public class C implements Interfaze {\n"
		"		public void foo(int t) {\n"
		"		}\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->overrides, "void Main.C.foo(int) -> void Main.Interfaze.foo(int) <7:15 7:17>"));
}

TEST_CASE("java parser finds overridden method with generic signature", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"class Main <X> {\n"
		"	public interface Interfaze <T> {\n"
		"		public void foo(T t);\n"
		"	}\n"
		"\n"
		"	public class C implements Interfaze <X> {\n"
		"		public void foo(X t) {\n"
		"		}\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->overrides,
		"void Main<X>.C.foo(Main<X>.X) -> void Main<X>.Interfaze<T>.foo(Main<X>.Interfaze<T>.T) "
		"<7:15 7:17>"));
}

TEST_CASE("java parser finds method usage for creation reference", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"	public interface Functor\n"
		"	{\n"
		"		public void doSomething();\n"
		"	}\n"
		"\n"
		"	public class B\n"
		"	{\n"
		"	}\n"
		"\n"
		"	void foo()\n"
		"	{\n"
		"		Functor method = B::new;\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->usages, "void A.foo() -> A.B.B() <14:23 14:25>"));
}

TEST_CASE("java parser finds method usage for expression method reference", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"	public interface Functor\n"
		"	{\n"
		"		public void doSomething();\n"
		"	}\n"
		"\n"
		"	public class B\n"
		"	{\n"
		"		void bar()\n"
		"		{\n"
		"		}\n"
		"	}\n"
		"\n"
		"	void foo()\n"
		"	{\n"
		"		Functor method = B::bar;\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->usages, "void A.foo() -> void A.B.bar() <17:23 17:25>"));
}

TEST_CASE("java parser finds method usage for super method reference", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A {\n"
		"	public interface Functor {\n"
		"		public void doSomething();\n"
		"	}\n"
		"\n"
		"	public class B {\n"
		"		void bar() {\n"
		"		}\n"
		"	}\n"
		"\n"
		"	public class C extends B {\n"
		"		void foo() {\n"
		"			Functor method = super::bar;\n"
		"		}\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->usages, "void A.C.foo() -> void A.B.bar() <13:28 13:30>"));
}

TEST_CASE("java parser finds no method usage for type method reference", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A {\n"
		"	void foo() {\n"
		"		Functor method = int []::clone;\n"
		"	}\n"
		"}\n");

	// finding method usage here may be implemented in the future.
	REQUIRE(client->usages.size() == 0);
}

TEST_CASE("java parser finds no usage of field within that fields declaration", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class X\n"
		"{\n"
		"	private int t;\n"
		"}\n");

	REQUIRE(client->usages.size() == 0);
}

TEST_CASE("java parser finds no usage of enum constant within that enum constants declaration", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public enum X\n"
		"{\n"
		"	OPTION_0;\n"
		"}\n");

	REQUIRE(client->usages.size() == 0);
}

TEST_CASE("java parser finds usage of field with same name as method parameter", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class X\n"
		"{\n"
		"	private int t;\n"
		"	public X(int t)\n"
		"	{\n"
		"		this.t = t;\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->usages, "foo.X.X(int) -> int foo.X.t <7:8 7:8>"));
}

TEST_CASE("java parser does not confuse method name with field name", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class X\n"
		"{\n"
		"	private int foo;\n"
		"	public void foo()\n"
		"	{\n"
		"		this.foo = 5;\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->usages, "void foo.X.foo() -> int foo.X.foo <7:8 7:10>"));
}

TEST_CASE("java parser finds assignment of method parameter", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class A\n"
		"{\n"
		"	void bar(int i)\n"
		"	{\n"
		"		i = 0;\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(client->localSymbols, "foo.A.bar<0> <6:3 6:3>"));
}

TEST_CASE("java parser finds assignment of local variable", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"public class A\n"
		"{\n"
		"	void bar()\n"
		"	{\n"
		"		int i;\n"
		"		i = 0;\n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(client->localSymbols, "foo.A.bar<0> <7:3 7:3>"));
}

TEST_CASE("java parser finds scope of class declaration", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"}\n");

	REQUIRE(
		utility::containsElement<std::string>(client->localSymbols, "input.java<2:1> <2:1 2:1>"));
	REQUIRE(
		utility::containsElement<std::string>(client->localSymbols, "input.java<2:1> <3:1 3:1>"));
}

TEST_CASE("java parser finds scope of enum declaration", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public enum A\n"
		"{\n"
		"}\n");

	REQUIRE(
		utility::containsElement<std::string>(client->localSymbols, "input.java<2:1> <2:1 2:1>"));
	REQUIRE(
		utility::containsElement<std::string>(client->localSymbols, "input.java<2:1> <3:1 3:1>"));
}

TEST_CASE("java parser finds scope of constructor declaration", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"	public A()\n"
		"	{\n"
		"	}\n"
		"}\n");

	REQUIRE(
		utility::containsElement<std::string>(client->localSymbols, "input.java<4:2> <4:2 4:2>"));
	REQUIRE(
		utility::containsElement<std::string>(client->localSymbols, "input.java<4:2> <5:2 5:2>"));
}

TEST_CASE("java parser finds scope of method declaration", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"	public void a()\n"
		"	{\n"
		"	}\n"
		"}\n");

	REQUIRE(
		utility::containsElement<std::string>(client->localSymbols, "input.java<4:2> <4:2 4:2>"));
	REQUIRE(
		utility::containsElement<std::string>(client->localSymbols, "input.java<4:2> <5:2 5:2>"));
}

TEST_CASE("java parser finds scope of switch statement", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"	public void a()\n"
		"	{\n"
		"		switch(2)\n"
		"		{\n"
		"		case 1:\n"
		"			break;\n"
		"		}\n"
		"	}\n"
		"}\n");

	REQUIRE(
		utility::containsElement<std::string>(client->localSymbols, "input.java<6:3> <6:3 6:3>"));
	REQUIRE(
		utility::containsElement<std::string>(client->localSymbols, "input.java<6:3> <9:3 9:3>"));
}

TEST_CASE("java parser finds scope of block statement", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"	public void a()\n"
		"	{\n"
		"		{\n"
		"		}\n"
		"	}\n"
		"}\n");

	REQUIRE(
		utility::containsElement<std::string>(client->localSymbols, "input.java<5:3> <5:3 5:3>"));
	REQUIRE(
		utility::containsElement<std::string>(client->localSymbols, "input.java<5:3> <6:3 6:3>"));
}

TEST_CASE("java parser finds scope of array initialization list", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"	private int[] array = {1, 2};\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->localSymbols, "input.java<3:24> <3:24 3:24>"));
	REQUIRE(utility::containsElement<std::string>(
		client->localSymbols, "input.java<3:24> <3:29 3:29>"));
}

TEST_CASE("java parser finds scope of anonymous class declaration", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"	public interface Base\n"
		"	{\n"
		"	}\n"
		"\n"
		"	void foo()\n"
		"	{\n"
		"		Base b = new Base()\n"
		"		{\n"
		"		}\n"
		"	};\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->localSymbols, "input.java<10:3> <10:3 10:3>"));
	REQUIRE(utility::containsElement<std::string>(
		client->localSymbols, "input.java<10:3> <11:3 11:3>"));
}

TEST_CASE("java parser finds usage of type parameter of class", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A <T>\n"
		"{\n"
		"	T t;\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->typeUses, "A<T>.T A<T>.t -> A<T>.T <3:2 3:2>"));
}

TEST_CASE("java parser finds usage of type parameter of method", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A\n"
		"{\n"
		"	public <T> void foo(T t){};\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->typeUses, "void A.foo<T>(T) -> A.foo<T>.T <3:22 3:22>"));
}

TEST_CASE("java parser finds correct location of generic type usage", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A <T>\n"
		"{\n"
		"	A<Void> t;\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->typeUses, "A<java.lang.Void> A<T>.t -> A<T> <3:2 3:2>"));
}

TEST_CASE("java parser finds bound type of type parameter", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"public class A <T extends Void>\n"
		"{\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->typeUses, "A<T>.T -> java.lang.Void <1:27 1:30>"));
}

TEST_CASE("java parser supports java 14 switch expression", JAVA_TAG)
{
	std::shared_ptr<TestStorage> client = parseCode(
		"package foo;\n"
		"\n"
		"public class Foo {\n"
		"		enum FruitType {\n"
		"		PEAR, \n"
		"		APPLE\n"
		"	}\n"
		"	\n"
		"	public void foo() {\n"
		"		FruitType fruit = FruitType.APPLE; \n"
		"		int numberOfLetters2 = switch (fruit) {\n"
		"			case PEAR -> 4; \n"
		"			case APPLE -> 5; \n"
		"		}; \n"
		"	}\n"
		"}\n");

	REQUIRE(utility::containsElement<std::string>(
		client->usages, "void foo.Foo.foo() -> foo.Foo.FruitType.PEAR <12:9 12:12>"));
}


#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE
