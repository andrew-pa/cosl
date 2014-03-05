#pragma once
#include "cmmn.h"

enum class shader_type
{
	vertex_shader, pixel_shader,
};

enum class base_s_type
{
	bool_t, int_t, uint_t, float_t, double_t,
	bvec_t, ivec_t, uvec_t, vec_t, dvec_t,
	mat_t, dmat_t, user_def_t, void_t
};

struct s_type
{
	base_s_type type;
	uint vecdim;
	uint matdim;
	string* udt_name;
	s_type(base_s_type t)
		: type(t), vecdim(-1), matdim(-1), udt_name(nullptr) {}
	s_type(base_s_type t, uint vd)
		: type(t), vecdim(vd), matdim(-1), udt_name(nullptr) {}

	s_type(base_s_type t, uint vd, uint md)
		: type(t), vecdim(vd), matdim(md), udt_name(nullptr) {}
	s_type(const string& udt_nm)
		: type(base_s_type::user_def_t), udt_name(new string(udt_nm)), vecdim(-1), matdim(-1) {}

};

struct semantic
{
	string name;
	uint idx;
	semantic(string n, uint i)
		: name(n), idx(i)
	{}
};

struct decl
{
	s_type* type;
	string name;
	semantic* sem;
	decl(s_type* t, string n, semantic* s)
		: type(t), name(n), sem(s) {}
};

struct decl_block
{
	vector<decl> decls;
	decl_block(vector<decl>& d)
		: decls(d)	{}
};

class input_block : public decl_block 
{
	input_block(vector<decl>& d)
		: decl_block(d)	{}
};//typedef decl_block input_block; 
class output_block : public decl_block 
{
	output_block(vector<decl>& d)
		: decl_block(d)	{}
};//typedef decl_block output_block; 

struct cbuffer_block
{
public:
	uint reg_idx;
	decl_block* dclbk;
	cbuffer_block(uint i, decl_block* dcl)
		: reg_idx(i), dclbk(dcl)	{}
};

enum class texture_type
{
	texture1D, texture2D, texture3D, textureCube,
};

struct texture_def 
{
public:
	texture_type type;
	string name;
	uint reg_idx;
	texture_def(texture_type t, const string& n, uint rx)
		: type(t), name(n), reg_idx(rx){}
};

struct id
{
	string base_name;
	vector<string> members;
	id(const string& s)
		: base_name(), members()
	{
		vector<string> parts;
		string nx = "";
		for (int i = 0; i < s.size(); ++i)
		{
			if(s[i] == '.')
			{
				parts.push_back(nx);
				nx = "";
				continue;
			}
			nx += s[i];
		}
		if (!nx.empty()) parts.push_back(nx);
		if(parts.size() == 0)
		{
			base_name = s;
		}
		else
		{
			base_name = parts[0];
			members = vector<string>(parts.begin() + 1, parts.end());
		}
	}
};

#pragma region expr

struct expr
{
	virtual ~expr(){}
};

struct term : public expr
{
	virtual ~term(){}
};

struct primary : public term
{
	virtual ~primary() {}
};

struct id_primary : public primary
{
	id _id;
	id_primary(id _d)
		: _id(_d) {}
};

struct num_primary : public primary
{
	string num;
	num_primary() {}
	num_primary(const string& s)
		: num(s) {}
};

struct func_invoke_primary : public primary
{
	string func_name;
	vector<expr*> args;
	func_invoke_primary(const string& n, const vector<expr*>& g)
		: func_name(n), args(g)
	{}
};



struct primary_term : public term
{
	primary* p;
	primary_term(): p(nullptr){}
	primary_term(primary*_p) :p(_p){}
};

struct mul_term : public term
{
	term* left;
	primary* right;
	mul_term() {}
	mul_term(term* l, primary* p)
		: left(l), right(p){}
};
struct div_term : public term
{
	term* left;
	primary* right;
	div_term() {}
	div_term(term* l, primary* p)
		: left(l), right(p){}
};


struct add_expr : public expr
{
	expr* left;
	term* right;
	add_expr(expr* l, term* r)
		: left(l), right(r){}
};
struct sub_expr : public expr
{
	expr* left;
	term* right;
	sub_expr(expr* l, term* r)
		: left(l), right(r){}
};

struct bool_expr : public expr
{
};

struct true_bexpr : public bool_expr {};
struct false_bexpr : public bool_expr {};

struct not_bexpr : public bool_expr
{
	bool_expr* xpr;
	not_bexpr(bool_expr* x)
		: xpr(x){}
};

enum bool_op
{
	less, greater, less_equal, greater_equal, equal, not_equal, and, or,
};

struct binary_bexpr : public bool_expr
{
	bool_op op;
	expr* left, *right;
	binary_bexpr(expr* l, bool_op o, expr* r)
		: left(l), right(r), op(o){}
};

#pragma endregion

#pragma region stmt

struct stmt
{
	virtual ~stmt(){}
};

struct multi_stmt : public stmt
{
	stmt* f, *s;
	multi_stmt(stmt* ff, stmt* ss)
		: f(ff), s(ss) {}
};

struct block_stmt : public stmt
{
	stmt* s;
	block_stmt(stmt* _)
		: s(_){}
};

enum class assign_op
{
	equal, plus_equal, minus_equal, incr, deincr, mul_equal, div_equal,
};

struct assign_stmt : public stmt
{
	id name;
	assign_op op;
	expr* xpr;
	assign_stmt(id nm, assign_op o, expr* x)
		: name(nm), op(o), xpr(x){}
};

struct decl_stmt : public stmt
{
	s_type* typ;
	string name; //see BNF grammer, it's not a id, its a #ID
	expr* init_xpr;
	decl_stmt(s_type* t, const string& n, expr* ix = nullptr)
		: typ(t), name(n), init_xpr(ix) {}
};

struct if_stmt : public stmt
{
	bool_expr* xpr;
	stmt* body;
	stmt* else_body;
	if_stmt(bool_expr* x, stmt* b, stmt* e = nullptr)
		: xpr(x), body(b), else_body(e)	 {}
};

struct while_stmt : public stmt
{
	bool_expr* xpr;
	stmt* body;
	while_stmt(bool_expr* x, stmt* b)
		: xpr(x), body(b)	 {}
};

struct for_stmt : public stmt
{
	stmt* init_stmt;
	bool_expr* cond;
	stmt* incr_stmt;
	stmt* body;
	for_stmt(stmt* i, bool_expr* c, stmt* ic, stmt* b)
		: init_stmt(i), cond(c), incr_stmt(ic), body(b)
	{
	}
};

struct func_invoke_stmt : public stmt
{
	string func_name;
	vector<expr*> args;
	func_invoke_stmt(string n, vector<expr*> a)
		: func_name(n), args(a){}
};

struct return_stmt : public stmt
{
	expr* rval;
	return_stmt(expr* r = nullptr)
		: rval(r) {}
};

#pragma endregion

struct sfunction
{
	s_type* return_type;
	string name;
	struct func_arg
	{
		s_type* typ;
		string nmn;
		func_arg(s_type* t, const string& n)
			: typ(t), nmn(n){}
	};
	vector<func_arg> args;
	stmt* body;
	sfunction(s_type* rt, const string& n, const vector<func_arg>& farg, stmt* bd)
		: return_type(rt), name(n), args(farg), body(bd){}
};

struct shader_file
{
public:
	shader_type type;
	input_block* inpblk;
	output_block* outblk;
	vector<cbuffer_block*> cbuffers;
	vector<texture_def*> texturedefs;
	vector<sfunction*> functions;
};