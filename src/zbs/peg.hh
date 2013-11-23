#pragma once

#include "zbs/_types.hh"
#include "zbs/_slice.hh"
#include "zbs/_vector.hh"
#include "zbs/_error.hh"
#include <memory>

namespace zbs {
namespace peg {

// PEG AST node type
enum class ast_type : byte {
	// A 'literal' node type is used in order to avoid allocating many
	// small character nodes for common case parts of grammar such as
	// keywords. Literal represents a sequence of characters, in other
	// words - one or more characters.
	literal,    // sequence of characters
	set,        // set of characters
	range,      // range of characters
	any,        // anything
	true_,      // always succeeds without consuming input
	false_,     // always fails without consuming input

	repetition, // repeat the ast node N times
	sequence,   // generic sequence of ast nodes
	choice,     // try left, then if it fails, try right
	not_,       // not PEG expression, doesn't consume input
	and_,       // and PEG expression, doesn't consume input
	call,       // recursion in grammar
};

// PEG AST node
struct ast_node {
	static constexpr int shortbuf_len = sizeof(char*);

	ast_type type;

	// literal, set:
	//    length of the buffer
	// any:
	//    number of any matches (optimization)
	// repetition:
	//    number of repetitions
	int len;

	std::unique_ptr<ast_node> left;
	std::unique_ptr<ast_node> right;

	// short buffer optimization here, don't allocate the buffer for a
	// string unless its length is greater than shortbuf_len
	union {
		char *buf;
		char shortbuf[shortbuf_len];

		// for range nodes
		struct {
			rune from;
			rune to;
		};
	};

	ast_node() = default;
	ast_node(ast_node&&) = delete;
	ast_node(const ast_node&) = delete;
	~ast_node();

	ast_node &operator=(ast_node&&) = delete;
	ast_node &operator=(const ast_node&) = delete;

	// valid for literal and set
	slice<char> buffer();
	slice<const char> buffer() const;

	ast_node *clone() const;
};

struct ast {
	std::unique_ptr<ast_node> p;

	ast() = delete;
	ast(const char *str);
	ast(int n);
	ast(char c);
	explicit ast(ast_node *p): p(p) {}
};

// P - pattern, matches string as is
ast P(const char *str);

// P(n) - matches any character n times, if n < 0, it's the same as n == 0
ast P(int n);

// P(c) - matches byte as is
ast P(char c);

// S - set, matches if any of the characters in the set matches
ast S(const char *set);

// R - range, expects two runes, inclusively matches everything in the range
ast R(const char *range);

// *patt - zero or more matches of patt
ast operator*(const ast &lhs);
ast operator*(ast &&lhs);

// +patt - one or more matches of patt
ast operator+(const ast &lhs);
ast operator+(ast &&lhs);

// -patt - zero or one match of patt
ast operator-(const ast &lhs);
ast operator-(ast &&lhs);

// patt1 >> patt2 - matches patt1 followed by patt2
ast operator>>(const ast &lhs, const ast &rhs);
ast operator>>(const ast &lhs, ast &&rhs);
ast operator>>(ast &&lhs, const ast &rhs);
ast operator>>(ast &&lhs, ast &&rhs);

// patt1 | patt2 - matches patt1 or patt2 (ordered choice)
ast operator|(const ast &lhs, const ast &rhs);
ast operator|(const ast &lhs, ast &&rhs);
ast operator|(ast &&lhs, const ast &rhs);
ast operator|(ast &&lhs, ast &&rhs);

// patt1 - patt2 - matches patt1 if patt2 doesn't match,
// equivalent to !patt2 >> patt1
ast operator-(const ast &lhs, const ast &rhs);
ast operator-(const ast &lhs, ast &&rhs);
ast operator-(ast &&lhs, const ast &rhs);
ast operator-(ast &&lhs, ast &&rhs);

// &patt - PEG's and, matches patt, but consumes no input
ast operator&(const ast &arg);
ast operator&(ast &&arg);

// !patt - PEG's not, matches if patt doesn't match, consumes no input
ast operator!(const ast &arg);
ast operator!(ast &&arg);

void dump(const ast &a);

class bytecode {
	zbs::vector<byte> code;

public:
	bytecode() = delete;
	explicit bytecode(zbs::vector<byte> code): code(code) {}
	bytecode(bytecode&&) = default;
	bytecode(const bytecode&) = default;

	bytecode &operator=(bytecode&&) = default;
	bytecode &operator=(const bytecode&) = default;

	bool match(zbs::slice<const char> input) const;
};

bytecode compile(const ast &tree, error *err = &default_error);

}} // namespace zbs::peg
