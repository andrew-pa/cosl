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
	mat_t, dmat_t, user_def_t,
};

struct s_type
{
	base_s_type type;
	uint vecdim;
	uint matdim;
	string* udt_name;
};

typedef string semantic;

struct decl
{
	s_type type;
	string name;
	semantic sem;
};
struct decl_block
{
	vector<decl> decls;
};

typedef decl_block input_block; 
typedef decl_block output_block; 

struct cbuffer_block : public decl_block
{
public:
	uint reg_idx;
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
};

struct id
{
	string base_name;
	vector<string> members;
};

#pragma region expr
struct expr
{
	virtual ~expr(){}
};

struct id_expr : public expr
{
	id _id;
};

struct add_expr : public expr
{
	expr* left, *right;
};

struct term : public expr
{
};
struct mul_term : public term
{
	expr* left, *right;
};
struct div_term : public term
{
	expr* left, *right;
};

struct bool_expr : public expr
{
};

enum class bool_op
{
	less, greater, less_equal, greater_equal, equal, not_equal, not, and, or
};

struct binary_bool_expr : public expr
{
	expr* left, *right;
	bool_op op;
};

struct true_expr  : public bool_expr {};
struct false_expr : public bool_expr {};

struct func_invoke_expr : public expr
{
	string func_name;
	vector<expr*> args;
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
};

struct block_stmt : public stmt
{
	stmt* s;
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
};

struct decl_stmt : public stmt
{
	id name;
	expr* init_xpr;
};

struct if_stmt : public stmt
{
	bool_expr* xpr;
	stmt* body;
	stmt* else_body;
};

struct while_stmt : public stmt
{
	bool_expr* xpr;
	stmt* body;
};

struct for_stmt : public stmt
{
	stmt* init_stmt;
	bool_expr* cond;
	stmt* incr_stmt;
	stmt* body;
};

struct func_invoke_stmt : public stmt
{
	string func_name;
	vector<expr*> args;
};

struct return_stmt : public stmt
{
	expr* rval;
};

#pragma endregion

struct sfunction
{
	s_type type;
	string name;
	struct func_arg
	{
		s_type typ;
		string nmn;
	};
	vector<func_arg> args;
	stmt* body;
};

struct shader_file
{
public:
	shader_type type;
	input_block inpblk;
	output_block outblk;
	vector<cbuffer_block> cbuffers;
	vector<texture_def> texturedefs;
	vector<sfunction> functions;
};