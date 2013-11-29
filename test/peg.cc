#include "stf.hh"
#include "zbs.hh"
#include "zbs/peg.hh"

STF_SUITE_NAME("zbs::peg");

namespace peg = zbs::peg;

STF_TEST("basic sequence match") {
	using namespace zbs::peg;
	ast a = P("привет");
	bytecode p = compile(a);
	STF_ASSERT(p.match("привет"));
	STF_ASSERT(!p.match("приве"));
	STF_ASSERT(!p.match("Привет"));
	STF_ASSERT(!p.match("приет"));
	ast b = P("п") >> "р" >> "и" >> "вет";
	bytecode p2 = compile(b);
	STF_ASSERT(p2.match("привет"));
	STF_ASSERT(!p2.match("приве"));
	STF_ASSERT(!p2.match("Привет"));
	STF_ASSERT(!p2.match("приет"));
}

STF_TEST("range") {
	using namespace zbs::peg;
	ast a = R("ая");
	bytecode p = compile(a);
	STF_ASSERT(p.match("а"));
	STF_ASSERT(p.match("б"));
	STF_ASSERT(p.match("в"));
	STF_ASSERT(p.match("э"));
	STF_ASSERT(p.match("ю"));
	STF_ASSERT(p.match("я"));
	STF_ASSERT(!p.match("А"));
	STF_ASSERT(!p.match("Б"));
	STF_ASSERT(!p.match("Ю"));
	STF_ASSERT(!p.match("Я"));
	STF_ASSERT(!p.match("s"));
}

STF_TEST("choice") {
	using namespace zbs::peg;
	ast a = (P("abc") | P("ABC")) >> P("def");
	bytecode p = compile(a);
	STF_ASSERT(p.match("abcdef"));
	STF_ASSERT(p.match("ABCdef"));
	STF_ASSERT(!p.match("ABCDEF"));
	STF_ASSERT(!p.match("abcDEF"));
	STF_ASSERT(!p.match("aaabcdef"));
	STF_ASSERT(!p.match("aBcdef"));
}

STF_TEST("repetition") {
	using namespace zbs::peg;
	bytecode p = compile(*R("09") >> ";");
	STF_ASSERT(p.match("1235646;"));
	STF_ASSERT(!p.match("123a646;"));
	STF_ASSERT(p.match(";"));
	STF_ASSERT(!p.match(""));

	bytecode p2 = compile(+R("09"));
	STF_ASSERT(!p2.match(""));
	STF_ASSERT(p2.match("123789"));
	STF_ASSERT(!p2.match("asd543"));
	STF_ASSERT(p2.match("989asd543"));

	bytecode p3 = compile(P(":") >> -P("hello") >> ":");
	STF_ASSERT(p3.match("::"));
	STF_ASSERT(!p3.match(":"));
	STF_ASSERT(p3.match(":hello:"));
	STF_ASSERT(!p3.match(":Hello:"));
	STF_ASSERT(!p3.match(":hell:"));
	STF_ASSERT(!p3.match(""));
}

STF_TEST("and") {
	using namespace zbs::peg;
	bytecode p = compile(P("foo") >> &P("bar"));
	STF_ASSERT(p.match("foobar"));
	STF_ASSERT(!p.match("foobaz"));
	STF_ASSERT(!p.match("foo"));
	STF_ASSERT(!p.match("foob"));
}

STF_TEST("not") {
	using namespace zbs::peg;
	bytecode p = compile(+(R("09") - "6") >> ";");
	STF_ASSERT(!p.match(""));
	STF_ASSERT(p.match("12345;"));
	STF_ASSERT(!p.match("4647;"));
	STF_ASSERT(!p.match("64;"));
	STF_ASSERT(!p.match("456;"));
	STF_ASSERT(p.match("1209;"));
}

STF_TEST("any") {
	using namespace zbs::peg;
	bytecode p = compile(+(any() - (R("09") | P(";"))) >> ";");
	STF_ASSERT(p.match("whateverвсёок;"));
	STF_ASSERT(!p.match("whatever7всёок;"));
	STF_ASSERT(p.match("И ДАЖЕ ТАК/#$%&*@!)(;"));
	STF_ASSERT(!p.match("И0ДАЖЕ9ТАК/#$%&*@!)(;"));
}

STF_TEST("set") {
	using namespace zbs::peg;
	bytecode p = compile(+S("abcdefghijklmnopqrstuvwxyz") >> ";");
	STF_ASSERT(p.match("whatever;"));
	STF_ASSERT(!p.match("Nope;"));
	STF_ASSERT(!p.match("he he"));
	STF_ASSERT(!p.match("абвгд"));

	bytecode p2 = compile(+S("абвгдеёжзийклмнопрстуфхцчшщъыьэюя") >> ";");
	STF_ASSERT(!p2.match("whatever;"));
	STF_ASSERT(p2.match("привет;"));
	STF_ASSERT(!p2.match("А вот нифига;"));
	STF_ASSERT(!p2.match("Угу;"));
}

STF_TEST("capture") {
	using namespace zbs::peg;
	ast ident = R("AZ") | R("az") | R("09") | P("_");
	ast optspace = *P(" ");
	bytecode p = compile(
		C(+ident) >>
		optspace >> P("=") >> optspace >>
		C(+ident) >> ";"
	);
	auto optresult = p.capture<sequential_capturer<zbs::string>>(
		"name = nsf;",
		[](zbs::slice<const char> s) { return zbs::string{s}; }
	);
	STF_ASSERT(optresult);
	const auto &result = *optresult;
	STF_ASSERT(result[0] == "name");
	STF_ASSERT(result[1] == "nsf");
}

STF_TEST("tcltk layout (somewhat)") {
	using namespace zbs::peg;
	ast ident_symbol = R("az") | R("AZ") | R("09") | S("_.");
	ast special = (P("x") | "-" | "^") >> !ident_symbol;
	ast captoken = C(special | +ident_symbol);
	ast space = S(" \t");
	ast opt_space_nl = *S(" \t\n");
	ast line = opt_space_nl >> captoken >> *(+space >> captoken);
	ast layout = line >> *(C(P("\n")) >> line) >> opt_space_nl >> !any();
	auto p = compile(layout);
	auto optresult = p.capture(R"(
		.f -   -      .div
		.7 .8  .9     .mul
		.4 .5  .6     .minus
		.1 .2  .3     .plus
		.0 .pm .clear .eq
	)");

	const char *expected[] = {
		".f", "-",   "-",      ".div",   "\n",
		".7", ".8",  ".9",     ".mul",   "\n",
		".4", ".5",  ".6",     ".minus", "\n",
		".1", ".2",  ".3",     ".plus",  "\n",
		".0", ".pm", ".clear", ".eq",
	};

	STF_ASSERT(optresult);
	const auto &result = *optresult;
	STF_ASSERT(result.len() == 24);
	for (int i = 0; i < 24; i++) {
		STF_ASSERT(result[i] == expected[i]);
	}
}
