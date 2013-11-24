#include "zbs/peg.hh"
#include "zbs/_utils.hh"
#include "zbs/unicode/utf8.hh"
#include <cstring>

namespace utf8 = zbs::unicode::utf8;

namespace zbs {
namespace peg {

static ast_node *_new_node(ast_type type) {
	ast_node *n = new (or_die) ast_node;
	n->type = type;
	return n;
}

ast_node::~ast_node() {
	if (type == ast_type::literal || type == ast_type::set) {
		if (len > shortbuf_len)
			delete [] buf;
	}
}

slice<char> ast_node::buffer() {
	if (len <= shortbuf_len)
		return {shortbuf, len};
	return {buf, len};
}

slice<const char> ast_node::buffer() const {
	if (len <= shortbuf_len)
		return {shortbuf, len};
	return {buf, len};
}

ast_node *ast_node::clone() const {
	ast_node *n = _new_node(type);
	n->len = len;
	if (left)
		n->left.reset(left->clone());
	if (right)
		n->right.reset(right->clone());
	switch (type) {
	case ast_type::literal:
	case ast_type::set:
		if (len > shortbuf_len)
			n->buf = new (or_die) char[len];
		zbs::copy(n->buffer(), buffer());
		break;
	case ast_type::range:
		n->from = from;
		n->to = to;
		break;
	default:
		break;
	}
	return n;
}

ast::ast(const char *str): p(P(str).p) {
}

ast::ast(char c): p(P(c).p) {
}

static ast _string_node(ast_type type, const char *str) {
	ast_node *n = _new_node(type);
	n->len = std::strlen(str);
	if (n->len <= ast_node::shortbuf_len) {
		std::memcpy(n->shortbuf, str, n->len);
	} else {
		n->buf = new (or_die) char[n->len];
		zbs::copy(n->buffer(), slice<const char>{str, n->len});
	}

	return ast{n};
}

ast any() {
	return ast{_new_node(ast_type::any)};
}

// =========== pattern ===========
ast P(const char *str) {
	return _string_node(ast_type::literal, str);
}

ast P(char c) {
	char str[2] {c, '\0'};
	return P(str);
}

// =========== set ===========
ast S(const char *set) {
	return _string_node(ast_type::set, set);
}

// =========== range ===========
ast R(const char *range) {
	auto s = slice<const char>{range};
	_ZBS_ASSERT(utf8::rune_count(s) == 2);
	auto r1 = utf8::decode_rune(s);
	auto r2 = utf8::decode_rune(s.sub(r1.size));
	_ZBS_ASSERT(r1.rune < r2.rune);
	ast_node *n = _new_node(ast_type::range);
	n->from = r1.rune;
	n->to = r2.rune;
	return ast{n};
}

// =========== repetition ===========
ast operator*(const ast &lhs) {
	ast_node *n = _new_node(ast_type::repetition);
	n->len = 0;
	n->left.reset(lhs.p->clone());
	return ast{n};
}

ast operator*(ast &&lhs) {
	ast_node *n = _new_node(ast_type::repetition);
	n->len = 0;
	n->left = std::move(lhs.p);
	return ast{n};
}

ast operator+(const ast &lhs) {
	ast_node *n = _new_node(ast_type::repetition);
	n->len = 1;
	n->left.reset(lhs.p->clone());
	return ast{n};
}

ast operator+(ast &&lhs) {
	ast_node *n = _new_node(ast_type::repetition);
	n->len = 1;
	n->left = std::move(lhs.p);
	return ast{n};
}

ast operator-(const ast &lhs) {
	ast_node *n = _new_node(ast_type::repetition);
	n->len = -1;
	n->left.reset(lhs.p->clone());
	return ast{n};
}

ast operator-(ast &&lhs) {
	ast_node *n = _new_node(ast_type::repetition);
	n->len = -1;
	n->left = std::move(lhs.p);
	return ast{n};
}

// =========== sequence ===========
ast operator>>(const ast &lhs, const ast &rhs) {
	ast_node *n = _new_node(ast_type::sequence);
	n->left.reset(lhs.p->clone());
	n->right.reset(rhs.p->clone());
	return ast{n};
}

ast operator>>(const ast &lhs, ast &&rhs) {
	ast_node *n = _new_node(ast_type::sequence);
	n->left.reset(lhs.p->clone());
	n->right = std::move(rhs.p);
	return ast{n};
}

ast operator>>(ast &&lhs, const ast &rhs) {
	ast_node *n = _new_node(ast_type::sequence);
	n->left = std::move(lhs.p);
	n->right.reset(rhs.p->clone());
	return ast{n};
}

ast operator>>(ast &&lhs, ast &&rhs) {
	ast_node *n = _new_node(ast_type::sequence);
	n->left = std::move(lhs.p);
	n->right = std::move(rhs.p);
	return ast{n};
}

// =========== choice ===========
ast operator|(const ast &lhs, const ast &rhs) {
	ast_node *n = _new_node(ast_type::choice);
	n->left.reset(lhs.p->clone());
	n->right.reset(rhs.p->clone());
	return ast{n};
}

ast operator|(const ast &lhs, ast &&rhs) {
	ast_node *n = _new_node(ast_type::choice);
	n->left.reset(lhs.p->clone());
	n->right = std::move(rhs.p);
	return ast{n};
}

ast operator|(ast &&lhs, const ast &rhs) {
	ast_node *n = _new_node(ast_type::choice);
	n->left = std::move(lhs.p);
	n->right.reset(rhs.p->clone());
	return ast{n};
}

ast operator|(ast &&lhs, ast &&rhs) {
	ast_node *n = _new_node(ast_type::choice);
	n->left = std::move(lhs.p);
	n->right = std::move(rhs.p);
	return ast{n};
}

// =========== negation ===========
ast operator-(const ast &lhs, const ast &rhs) {
	return !rhs >> lhs;
}

ast operator-(const ast &lhs, ast &&rhs) {
	return !std::move(rhs) >> lhs;
}

ast operator-(ast &&lhs, const ast &rhs) {
	return !rhs >> std::move(lhs);
}

ast operator-(ast &&lhs, ast &&rhs) {
	return !std::move(rhs) >> std::move(lhs);
}

// =========== and_ ===========
ast operator&(const ast &arg) {
	ast_node *n = _new_node(ast_type::and_);
	n->left.reset(arg.p->clone());
	return ast{n};
}
ast operator&(ast &&arg) {
	ast_node *n = _new_node(ast_type::and_);
	n->left = std::move(arg.p);
	return ast{n};
}

// =========== not_ ===========
ast operator!(const ast &arg) {
	ast_node *n = _new_node(ast_type::not_);
	n->left.reset(arg.p->clone());
	return ast{n};
}

ast operator!(ast &&arg) {
	ast_node *n = _new_node(ast_type::not_);
	n->left = std::move(arg.p);
	return ast{n};
}

}} // namespace zbs::peg

#include "zbs/fmt.hh"

namespace zbs {
namespace peg {

zbs::string recursive_dump(const ast_node *a) {
	slice<const char> buf;
	zbs::string lhs, rhs;
	switch (a->type) {
	case ast_type::literal:
		buf = a->buffer();
		return fmt::sprintf("P(%.*s)", buf.len(), buf.data());
	case ast_type::set:
		buf = a->buffer();
		return fmt::sprintf("S(%.*s)", buf.len(), buf.data());
	case ast_type::range:
		return fmt::sprintf("R(%d, %d)", a->from, a->to);
	case ast_type::any:
		return fmt::sprintf("P(%d)", a->len);
	case ast_type::true_:
		return fmt::sprintf("true");
	case ast_type::false_:
		return fmt::sprintf("false");
	case ast_type::repetition:
		lhs = recursive_dump(a->left.get());
		return fmt::sprintf("%s*%d", lhs.c_str(), a->len);
	case ast_type::sequence:
		lhs = recursive_dump(a->left.get());
		rhs = recursive_dump(a->right.get());
		return fmt::sprintf("(%s >> %s)", lhs.c_str(), rhs.c_str());
	case ast_type::choice:
		lhs = recursive_dump(a->left.get());
		rhs = recursive_dump(a->right.get());
		return fmt::sprintf("(%s | %s)", lhs.c_str(), rhs.c_str());
	case ast_type::not_:
		lhs = recursive_dump(a->left.get());
		return fmt::sprintf("!%s", lhs.c_str());
	case ast_type::and_:
		lhs = recursive_dump(a->left.get());
		return fmt::sprintf("&%s", lhs.c_str());
	case ast_type::call:
		return fmt::sprintf("call()");
	}
	return "";
}

void dump(const ast &a) {
	auto s = recursive_dump(a.p.get());
	printf("%.*s\n", s.len(), s.c_str());
}

}} // namespace zbs::peg
