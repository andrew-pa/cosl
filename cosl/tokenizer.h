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
	token(){}
	token(token_type tp_, const string& s_)
		: tp(tp_), s(s_){}

	void print(ostream& ss)
	{
		switch (tp)
		{
		case token_type::id:
			ss << "id [" << s << "]";
			break;
		case token_type::string_lit:
			ss << "str[" << s << "]";
			break;
		case token_type::number_lit:
			ss << "num[" << s << "]";
			break;
		case token_type::special:
			ss << "spc[" << s << "]";
			break;
		case token_type::end:
			ss << "end";
			break;
		default:
			throw;
		}
	}
};

struct num_token : public token
{
	num_token(const string& d) : token(token_type::number_lit, d) {}
};
struct string_token : public token
{
	string_token(const string& d) : token(token_type::string_lit, d) {}
};
struct id_token : public token
{
	id_token(const string& d) : token(token_type::id, d) {}
};
struct special_token : public token
{
	special_token(char d) : token(token_type::special, string() + d) {}
	special_token(char da, char db) : token(token_type::special, (string() + da) + (string() + db)) {} //nasty hack
};
struct end_token : public token
{
	end_token() : token(token_type::end, string("end")) {}
};

#define get_token() _get_token(__LINE__, __FILE__)

class tokenizer
{
	string s;
	unsigned int idx;
	token buf;
	bool full;

	token next_token();
public:
	tokenizer(const string& s_)
		: s(s_), full(false), idx(0)
	{ }

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

	inline void put_back(const token& t)
	{
		buf = t;
		full = true;
	}
};