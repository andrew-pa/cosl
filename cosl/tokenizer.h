#pragma once
#include "cmmn.h"


enum class token_type
{
	id, string_lit, number_lit, special, end
};

struct token
{
	token_type tp;
	string s;
	int source_line;
	token(){}
	token(token_type tp_, const string& s_, int l = -1)
		: tp(tp_), s(s_), source_line(l) {}

	void print(ostream& ss) const
	{
		switch (tp)
		{
		case token_type::id:
			ss << "id [" << s << " : ln#" << source_line << "]";
			break;
		case token_type::string_lit:
			ss << "str[" << s << " : ln#" << source_line << "]";
			break;
		case token_type::number_lit:
			ss << "num[" << s << " : ln#" << source_line << "]";
			break;
		case token_type::special:
			ss << "spc[" << s << " : ln#" << source_line << "]";
			break;
		case token_type::end:
			ss << "end" << " : ln#" << source_line;
			break;
		default:
			throw;
		}
	}
};

struct num_token : public token
{
	num_token(const string& d, int l = -1) : token(token_type::number_lit, d, l) {}
};
struct string_token : public token
{
	string_token(const string& d, int l = -1) : token(token_type::string_lit, d, l) {}
};
struct id_token : public token
{
	id_token(const string& d, int l = -1) : token(token_type::id, d, l) {}
};
struct special_token : public token
{
	special_token(char d, int l = -1) : token(token_type::special, string() + d, l) {}
	special_token(char da, char db, int l = -1) : token(token_type::special, (string() + da) + (string() + db), l) {} //nasty hack
};
struct end_token : public token
{
	end_token(int l = -1) : token(token_type::end, string("end"), l) {}
};

#ifdef DEBUG_AT_LINE
#define get_token() _get_token(__LINE__, __FILE__)
#endif

class tokenizer
{
	string s;
	unsigned int idx;
	token buf;
	bool full;

	token next_token();

	int current_line;
public:
	tokenizer(const string& s_)
		: s(s_), full(false), idx(0), current_line(1)
	{ }

#ifdef DEBUG_AT_LINE
	inline token _get_token(int line, const char* file)
	{
		if (full)
		{
			full = false;
			cout << "rbuf[" << line << ":" << file << "] ";
			buf.print(cout);
			cout << endl;
			return buf;
		}
		auto t = next_token();
		cout << "rnxt[" << line << ":" << file << "] ";
		t.print(cout);
		cout << endl;
		return t;
	}
#else
	inline token get_token()
	{
		if (full)
		{
			full = false;
			return buf;
		}
		auto t = next_token();
		return t;
	}
#endif

	inline void put_back(const token& t)
	{
		buf = t;
		full = true;
	}
};