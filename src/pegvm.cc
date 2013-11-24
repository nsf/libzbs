#include "zbs/peg.hh"
#include "zbs/unicode/utf8.hh"

namespace utf8 = zbs::unicode::utf8;

namespace zbs {
namespace peg {

enum class inst_type : byte {
	any,
	string,
	set,
	range,
	end,
	choice,
	commit,
	partial_commit,
	rewind_commit,
	fail,
	fail_twice,
};

//----------------------------------------------------------------------------
// Instruction specific data structures
//----------------------------------------------------------------------------

struct inst_common {
	inst_type type;
};

template <inst_type IT>
struct inst_base {
	static constexpr inst_type class_type() { return IT; }
};

struct inst_any : inst_base<inst_type::any> {};

// match string or fail
struct inst_string : inst_base<inst_type::string> {
	inst_type type;
	uint8 len; // length of the 'str' attachment in bytes
	// utf-8 encoded string, so that you don't need to decode the input in
	// order to do the matching
	char str[1];

	slice<char> buffer() { return {str, len}; }
	slice<const char> buffer() const { return {str, len}; }
};

// match set or fail
struct inst_set : inst_base<inst_type::set> {
	inst_type type;
	uint16 len;      // length of the 'uni' attachment
	uint32 ascii[4]; // bitmap for the 7-bit ascii subset of utf-8
	rune uni[1];     // the rest is attached here in a decoded form, sorted
};

// match range or fail
struct inst_range : inst_base<inst_type::range> {
	// Only the last three bytes are used as rune. The first byte is
	// 'inst_type'.
	uint32 rune_from;
	rune rune_to;

	void set_from(rune r) {	rune_from = (rune_from & 0xFF) | r << 8; }
	void set_to(rune r) { rune_to = r; }
	rune from() const { return rune_from >> 8; }
	rune to() const { return rune_to; }
};

struct inst_end : inst_base<inst_type::end> {};

struct inst_choice : inst_base<inst_type::choice> {
	inst_type type;
	int offset;
};

struct inst_commit : inst_base<inst_type::commit> {
	inst_type type;
	int offset;
};

struct inst_partial_commit : inst_base<inst_type::partial_commit> {
	inst_type type;
	int offset;
};

struct inst_rewind_commit : inst_base<inst_type::rewind_commit> {
	inst_type type;
	int offset;
};

struct inst_fail : inst_base<inst_type::fail> {};
struct inst_fail_twice : inst_base<inst_type::fail_twice> {};

//----------------------------------------------------------------------------
// Instruction length calculation helpers
//----------------------------------------------------------------------------

// Returns the 'n' rounded to a multiple of 4. Rounds away from zero.
static int multiple_of_4(int n) {
	return (n + 3) & ~3;
}

// Calculates additional space for members at the end of the struct, assuming
// one member is part of the type. Always >= 0. Used in 'inst_len' and
// 'inst_new'.
static int addlen(int n) {
	return (n > 1) ? n - 1 : 0;
}

//----------------------------------------------------------------------------
// Length of the actual instruction, used in the VM to advance the instruction
// poiner.
//----------------------------------------------------------------------------

template <typename T>
static int inst_len(const T*) {
	// size is a multiple of 4 by default
	return multiple_of_4(sizeof(T));
}

static int inst_len(const inst_string *p) {
	return multiple_of_4(sizeof(inst_string) + addlen(p->len));
}

static int inst_len(const inst_set *p) {
	return multiple_of_4(sizeof(inst_set) + addlen(p->len)*sizeof(rune));
}

//----------------------------------------------------------------------------
// Special pointer type we use for accessing instructions in a growing buffer,
// the point is to keep it valid even if realloc happens.
//----------------------------------------------------------------------------

template <typename T>
struct inst_ptr {
	vector<byte> &instbuf;
	int offset;

	T *operator->() const { return reinterpret_cast<T*>(instbuf.data()+offset); }
};

//----------------------------------------------------------------------------
// Create a new instruction of type T, optionally adds more slots, the meaning
// of a slot is type dependent.
//----------------------------------------------------------------------------
template <typename T>
static inst_ptr<T> inst_new(vector<byte> &instbuf, int add = 0) {
	int size;
	switch (T::class_type()) {
	case inst_type::string:
		size = multiple_of_4(sizeof(T) + addlen(add));
		break;
	case inst_type::set:
		size = multiple_of_4(sizeof(T) + addlen(add)*sizeof(rune));
		break;
	default:
		size = multiple_of_4(sizeof(T));
		break;
	}

	int off = instbuf.len();
	instbuf.resize(instbuf.len() + size);
	auto ins = reinterpret_cast<inst_common*>(instbuf.data() + off);
	ins->type = T::class_type();
	return {instbuf, off};
}

//----------------------------------------------------------------------------
// Main recursive compilation routine.
//----------------------------------------------------------------------------
static void codegen(vector<byte> &instbuf,
	const ast_node *tree, error *err)
{
	if (*err)
		return;

	switch (tree->type) {
	case ast_type::literal: {
		auto ins = inst_new<inst_string>(instbuf, tree->len);
		ins->len = tree->len;
		copy(ins->buffer(), tree->buffer());
		break;
	}
	case ast_type::set: {
		break;
	}
	case ast_type::range: {
		auto ins = inst_new<inst_range>(instbuf);
		ins->set_from(tree->from);
		ins->set_to(tree->to);
		break;
	}
	case ast_type::any: {
		inst_new<inst_any>(instbuf);
		break;
	}
	case ast_type::repetition: {
		if (tree->len < 0) {
			// patt? - zero or one
			auto choice = inst_new<inst_choice>(instbuf);
			codegen(instbuf, tree->left.get(), err);
			auto commit = inst_new<inst_commit>(instbuf);
			choice->offset = commit->offset = instbuf.len();
		} else {
			// patt* - zero or more
			// patt+ - one or more
			for (int i = 0; i < tree->len; i++) {
				codegen(instbuf, tree->left.get(), err);
			}
			auto choice = inst_new<inst_choice>(instbuf);
			int start = instbuf.len();
			codegen(instbuf, tree->left.get(), err);
			inst_new<inst_partial_commit>(instbuf)->offset = start;
			choice->offset = instbuf.len();
		}
		break;
	}
	case ast_type::sequence:
		codegen(instbuf, tree->left.get(), err);
		codegen(instbuf, tree->right.get(), err);
		break;
	case ast_type::choice: {
		auto choice = inst_new<inst_choice>(instbuf);
		codegen(instbuf, tree->left.get(), err);
		auto commit = inst_new<inst_commit>(instbuf);
		choice->offset = instbuf.len();
		codegen(instbuf, tree->right.get(), err);
		commit->offset = instbuf.len();
		break;
	}
	case ast_type::not_: {
		auto choice = inst_new<inst_choice>(instbuf);
		codegen(instbuf, tree->left.get(), err);
		inst_new<inst_fail_twice>(instbuf);
		choice->offset = instbuf.len();
		break;
	}
	case ast_type::and_: {
		auto choice = inst_new<inst_choice>(instbuf);
		codegen(instbuf, tree->left.get(), err);
		auto rcommit = inst_new<inst_rewind_commit>(instbuf);
		choice->offset = instbuf.len();
		inst_new<inst_fail>(instbuf);
		rcommit->offset = instbuf.len();
		break;
	}
	default:
		printf("oops\n");
		break;
	}
}

static void dump(slice<const byte> code) {
	const byte *ip = code.data();
	for (;;) {
		auto type = reinterpret_cast<const inst_common*>(ip)->type;
		int ioff = ip-code.data();
		switch (type) {
		case inst_type::any: {
			auto ia = reinterpret_cast<const inst_any*>(ip);
			printf("%4d: inst_any\n", ioff);
			ip += inst_len(ia);
			break;
		}
		case inst_type::string: {
			auto is = reinterpret_cast<const inst_string*>(ip);
			printf("%4d: inst_string (%d, \"%.*s\")\n",
				ioff, is->len, is->len, is->str);
			ip += inst_len(is);
			break;
		}
		case inst_type::range: {
			char tmpfrom[4+1];
			char tmpto[4+1];
			auto ir = reinterpret_cast<const inst_range*>(ip);
			tmpfrom[utf8::encode_rune(tmpfrom, ir->from())] = '\0';
			tmpto[utf8::encode_rune(tmpto, ir->to())] = '\0';
			printf("%4d: inst_range ('%s' - '%s')\n",
				ioff, tmpfrom, tmpto);
			ip += inst_len(ir);
			break;
		}
		case inst_type::choice: {
			auto ic = reinterpret_cast<const inst_choice*>(ip);
			printf("%4d: inst_choice (%d)\n", ioff, ic->offset);
			ip += inst_len(ic);
			break;
		}
		case inst_type::commit: {
			auto ic = reinterpret_cast<const inst_commit*>(ip);
			printf("%4d: inst_commit (%d)\n", ioff, ic->offset);
			ip += inst_len(ic);
			break;
		}
		case inst_type::partial_commit: {
			auto ipc = reinterpret_cast<const inst_partial_commit*>(ip);
			printf("%4d: inst_partial_commit (%d)\n", ioff, ipc->offset);
			ip += inst_len(ipc);
			break;
		}
		case inst_type::rewind_commit: {
			auto irc = reinterpret_cast<const inst_rewind_commit*>(ip);
			printf("%4d: inst_rewind_commit (%d)\n", ioff, irc->offset);
			ip += inst_len(irc);
			break;
		}
		case inst_type::fail: {
			auto instf = reinterpret_cast<const inst_fail*>(ip);
			printf("%4d: inst_fail\n", ioff);
			ip += inst_len(instf);
			break;
		}
		case inst_type::fail_twice: {
			auto instf = reinterpret_cast<const inst_fail_twice*>(ip);
			printf("%4d: inst_fail_twice\n", ioff);
			ip += inst_len(instf);
			break;
		}
		case inst_type::end: {
			printf("%4d: inst_end\n", ioff);
			return;
		}
		default:
			printf("%d: unknown instruction (%d)\n",
				ioff, type);
			return;
		}
	}
}

bytecode compile(const ast &tree, error *err) {
	vector<byte> instbuf;
	codegen(instbuf, tree.p.get(), err);
	inst_new<inst_end>(instbuf);
	return bytecode{std::move(instbuf)};
}

struct stack_t {
	slice<const char> input;
	int offset;
};

bool bytecode::match(slice<const char> input) const {
	printf("--- %.*s\n", input.len(), input.data());
	dump(code);
	vector<stack_t> stack;
	stack.reserve(8);

	const byte *ip = code.data();
	for (;;) {
		auto type = reinterpret_cast<const inst_common*>(ip)->type;
		switch (type) {
		case inst_type::any: {
			auto ia = reinterpret_cast<const inst_any*>(ip);
			if (input.len() == 0)
				goto fail;
			ip += inst_len(ia);
			input = input.sub(utf8::decode_rune(input).size);
			break;
		}
		case inst_type::string: {
			auto is = reinterpret_cast<const inst_string*>(ip);
			if (is->len > input.len())
				goto fail;

			const char *s = is->str;
			const char *s2 = input.data();
			const char *e = is->str + is->len;
			while (s < e && *s == *s2) {
				s++;
				s2++;
			}
			if (s != e)
				goto fail;

			ip += inst_len(is);
			input = input.sub(is->len);
			break;
		}
		case inst_type::range: {
			auto ir = reinterpret_cast<const inst_range*>(ip);
			if (input.len() < 1)
				goto fail;

			sized_rune r = utf8::decode_rune(input);
			if (r.rune < ir->from() || ir->to() < r.rune)
				goto fail;
			ip += inst_len(ir);
			input = input.sub(r.size);
			break;
		}
		case inst_type::choice: {
			auto ic = reinterpret_cast<const inst_choice*>(ip);
			stack.append({
				input,
				ic->offset,
			});
			ip += inst_len(ic);
			break;
		}
		case inst_type::commit: {
			auto ic = reinterpret_cast<const inst_commit*>(ip);
			_ZBS_ASSERT(stack.len() > 0);
			stack.resize(stack.len()-1);
			ip = code.data() + ic->offset;
			break;
		}
		case inst_type::partial_commit: {
			auto ipc = reinterpret_cast<const inst_partial_commit*>(ip);
			_ZBS_ASSERT(stack.len() > 0);
			stack[stack.len()-1].input = input;
			ip = code.data() + ipc->offset;
			break;
		}
		case inst_type::rewind_commit: {
			auto irc = reinterpret_cast<const inst_rewind_commit*>(ip);
			_ZBS_ASSERT(stack.len() > 0);
			input = stack[stack.len()-1].input;
			stack.resize(stack.len()-1);
			ip = code.data() + irc->offset;
			break;
		}
		case inst_type::fail_twice:
			_ZBS_ASSERT(stack.len() > 0);
			stack.resize(stack.len()-1);
			// fallthrough
		case inst_type::fail: fail:
			if (stack.len() != 0) {
				const auto &last = stack[stack.len()-1];
				input = last.input;
				ip = code.data() + last.offset;
				stack.resize(stack.len()-1);
			} else {
				return false;
			}
			break;
		case inst_type::end:
			return true;
		default:
			goto fail;
		}
	}
}

}} // namespace zbs::peg
