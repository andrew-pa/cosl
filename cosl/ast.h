#pragma once
#include "cmmn.h"

class code_emitter;

class ast_node
{
public:
	virtual void emit(code_emitter* ce) = 0;
	virtual ~ast_node(){}
};

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

struct s_type : public ast_node
{
	base_s_type type;
	uint vecdim;
	uint matdim;
	string* udt_name;
	vector<uint> array_dims;
	s_type(base_s_type t, vector<uint> ad)
		: type(t), vecdim(-1), matdim(-1), array_dims(ad), udt_name(nullptr) {}
	s_type(base_s_type t, uint vd, vector<uint> ad)
		: type(t), vecdim(vd), matdim(-1), array_dims(ad), udt_name(nullptr) {}

	s_type(base_s_type t, uint vd, uint md, vector<uint> ad)
		: type(t), vecdim(vd), matdim(md), array_dims(ad), udt_name(nullptr) {}
	s_type(const string& udt_nm, vector<uint> ad)
		: type(base_s_type::user_def_t), udt_name(new string(udt_nm)), array_dims(ad), vecdim(-1), matdim(-1) {}
	


	void emit(code_emitter* ce) override;
};

struct semantic 
{
	string name;
	uint idx;
	semantic(string n, uint i)
		: name(n), idx(i)
	{}
};

struct decl : public ast_node
{
	s_type* type;
	string name;
	semantic* sem;
	decl(s_type* t, string n, semantic* s)
		: type(t), name(n), sem(s) {}
	void emit(code_emitter* ce) override;
};

struct decl_block : public ast_node
{
	vector<decl> decls;
	decl_block(vector<decl>& d)
		: decls(d)	{}


	void emit(code_emitter* ce) override;

};

struct input_block : public ast_node
{
	decl_block* db;
	input_block(vector<decl>& d)
		: db(new decl_block(d))	{}
	input_block(decl_block* d)
		: db(d)	{}
	void emit(code_emitter* ce) override;
};//typedef decl_block input_block; 
struct output_block  : public ast_node
{
	decl_block* db;
	output_block(vector<decl>& d)
		: db(new decl_block(d))	{}
	output_block(decl_block* d)
		: db(d)	{}
	void emit(code_emitter* ce) override;
};//typedef decl_block output_block; 

struct cbuffer_block : public ast_node
{
public:
	uint reg_idx;
	decl_block* dclbk;
	cbuffer_block(uint i, decl_block* dcl)
		: reg_idx(i), dclbk(dcl)	{}
	void emit(code_emitter* ce) override;
};

struct struct_block : public ast_node
{
	string name;
	decl_block* db;
	struct_block(const string& n, decl_block* d)
		: name(n), db(d){}
	void emit(code_emitter* ce) override;
};


enum class texture_type
{
	texture1D, texture2D, texture3D, textureCube,
};

struct texture_def : public ast_node
{
public:
	texture_type type;
	string name;
	uint reg_idx;
	texture_def(texture_type t, const string& n, uint rx)
		: type(t), name(n), reg_idx(rx){}
	void emit(code_emitter* ce) override;
};

//struct id : public ast_node
//{
//	string base_name;
//	id(const string& s)
//		: base_name(s)
//	{
//	}
//	void emit(code_emitter* ce) override;
//};

#pragma region expr

struct expr : public ast_node
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
	string id_s;
	id_primary(const string& _d)
		: id_s(_d) {}
	void emit(code_emitter* ce) override;
};

struct member_access_primary : public id_primary
{
	primary* val;
	vector<string> members;
	member_access_primary(primary* p, const vector<string>& m)
		: val(p), members(m), id_primary("__invalid_member_access_primary"){}
	void emit(code_emitter* ce) override;
};

struct array_index_primary : public primary
{
	primary* base;
	vector<expr*> indices;
	array_index_primary(primary* b, vector<expr*> ix)
		: base(b), indices(ix){}
	void emit(code_emitter* ce) override;
};

struct num_primary : public primary
{
	string num;
	num_primary() {}
	num_primary(const string& s)
		: num(s) {}
	void emit(code_emitter* ce) override;
};

struct negation_primary : public primary
{
	expr* exp;
	negation_primary() {}
	negation_primary(expr* e)
		: exp(e){}
	void emit(code_emitter* ce) override;
};

struct func_invoke_primary : public primary
{
	string func_name;
	vector<expr*> args;
	func_invoke_primary(const string& n, const vector<expr*>& g)
		: func_name(n), args(g)
	{}
	void emit(code_emitter* ce) override;
};



struct primary_term : public term
{
	primary* p;
	primary_term(): p(nullptr){}
	primary_term(primary*_p) :p(_p){}
	void emit(code_emitter* ce) override;
};

struct expr_in_paren : public primary
{
	expr* x;
	expr_in_paren() : x(nullptr){}
	expr_in_paren(expr* xp) : x(xp){}
	void emit(code_emitter* ce) override;
};

struct mul_term : public term
{
	term* left;
	primary* right;
	mul_term() {}
	mul_term(term* l, primary* p)
		: left(l), right(p){}
	void emit(code_emitter* ce) override;
};
struct div_term : public term
{
	term* left;
	primary* right;
	div_term() {}
	div_term(term* l, primary* p)
		: left(l), right(p){}
	void emit(code_emitter* ce) override;
};


struct add_expr : public expr
{
	expr* left;
	term* right;
	add_expr(expr* l, term* r)
		: left(l), right(r){}
	void emit(code_emitter* ce) override;
};
struct sub_expr : public expr
{
	expr* left;
	term* right;
	sub_expr(expr* l, term* r)
		: left(l), right(r){}
	void emit(code_emitter* ce) override;
};

struct bool_expr : public expr
{
};

struct true_bexpr : public bool_expr {
	void emit(code_emitter* ce) override;
};
struct false_bexpr : public bool_expr {
	void emit(code_emitter* ce) override;
};

struct not_bexpr : public bool_expr
{
	expr* xpr;
	not_bexpr(expr* x)
		: xpr(x){}
	void emit(code_emitter* ce) override;
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
	void emit(code_emitter* ce) override;
};

struct bool_expr_xvr : public bool_expr
{
	expr* xpr;
	bool_expr_xvr(expr* _xpr)
		: xpr(_xpr) {}
	inline void emit(code_emitter* ce) override { xpr->emit(ce); }
};

#pragma endregion

#pragma region stmt

struct stmt : public ast_node
{
	virtual ~stmt(){}
};

struct multi_stmt : public stmt
{
	stmt* f, *s;
	multi_stmt(stmt* ff, stmt* ss)
		: f(ff), s(ss) {}
	void emit(code_emitter* ce) override;
};

struct block_stmt : public stmt
{
	stmt* s;
	block_stmt(stmt* _)
		: s(_){}
	void emit(code_emitter* ce) override;
};

enum class assign_op
{
	equal, plus_equal, minus_equal, pre_incr, pre_deincr, post_incr, post_deincr, mul_equal, div_equal,
};

struct assign_stmt : public stmt
{
	id_primary* name;
	assign_op op;
	expr* xpr;
	assign_stmt(id_primary* nm, assign_op o, expr* x)
		: name(nm), op(o), xpr(x){}
	void emit(code_emitter* ce) override;
};

struct decl_stmt : public stmt
{
	s_type* typ;
	string name; //see BNF grammer, it's not a id, its a #ID
	expr* init_xpr;
	decl_stmt(s_type* t, const string& n, expr* ix = nullptr)
		: typ(t), name(n), init_xpr(ix) {}
	void emit(code_emitter* ce) override;
};

struct if_stmt : public stmt
{
	bool_expr* xpr;
	stmt* body;
	stmt* else_body;
	if_stmt(bool_expr* x, stmt* b, stmt* e = nullptr)
		: xpr(x), body(b), else_body(e)	 {}
	void emit(code_emitter* ce) override;
};

struct while_stmt : public stmt
{
	bool_expr* xpr;
	stmt* body;
	while_stmt(bool_expr* x, stmt* b)
		: xpr(x), body(b)	 {}
	void emit(code_emitter* ce) override;
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
	void emit(code_emitter* ce) override;
};

struct func_invoke_stmt : public stmt
{
	string func_name;
	vector<expr*> args;
	func_invoke_stmt(string n, vector<expr*> a)
		: func_name(n), args(a){}
	void emit(code_emitter* ce) override;
};

struct return_stmt : public stmt
{
	expr* rval;
	return_stmt(expr* r = nullptr)
		: rval(r) {}
	void emit(code_emitter* ce) override;
};

#pragma endregion

struct sfunction : public ast_node
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
	void emit(code_emitter* ce) override;
};

struct shader_file
{
public:
	shader_type type;
	input_block* inpblk;
	output_block* outblk;
	vector<cbuffer_block*> cbuffers;
	vector<texture_def*> texturedefs;
	vector<struct_block*> structdefs;
	vector<sfunction*> functions;
};