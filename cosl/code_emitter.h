#pragma once
#include "cmmn.h"
#include "ast.h"

class code_emitter
{
public:
	virtual string out_string() const = 0;
	virtual void reset() = 0;

	virtual void emit(shader_type x)= 0;
	virtual void emit(s_type* x)= 0;
	virtual void emit(decl& x)= 0;
	virtual void emit(decl_block* x)= 0;
	virtual void emit(input_block* x)= 0;
	virtual void emit(output_block* x)= 0;
	virtual void emit(cbuffer_block* x)= 0;
	virtual void emit(struct_block* x) = 0;
	virtual void emit(texture_def* x)= 0;

	//virtual void emit(expr* x)= 0;
	//virtual void emit(term* x)= 0;
	//virtual void emit(primary* x)= 0;
	virtual void emit(id_primary* x)= 0;
	virtual void emit(num_primary* x)= 0;
	virtual void emit(func_invoke_primary* x)= 0;
	virtual void emit(member_access_primary* x) = 0;
	virtual void emit(array_index_primary* x) = 0;
	virtual void emit(negation_primary* x) = 0;
	virtual void emit(primary_term* x)= 0;
	virtual void emit(expr_in_paren* x) = 0;
	virtual void emit(mul_term* x) = 0;
	virtual void emit(div_term* x)= 0;
	virtual void emit(add_expr* x)= 0;
	virtual void emit(sub_expr* x)= 0;
	virtual void emit(true_bexpr* x)= 0;
	virtual void emit(false_bexpr* x)= 0;
	virtual void emit(binary_bexpr* x)= 0;
	virtual void emit(not_bexpr* x)= 0;

	virtual void emit(multi_stmt* x)= 0;
	virtual void emit(block_stmt* x)= 0;
	virtual void emit(assign_stmt* x)= 0;
	virtual void emit(decl_stmt* x)= 0;
	virtual void emit(if_stmt* x)= 0;
	virtual void emit(while_stmt* x)= 0;
	virtual void emit(for_stmt* x)= 0;
	virtual void emit(func_invoke_stmt* x)= 0;
	virtual void emit(return_stmt* x)= 0;

	virtual void emit(sfunction* x)= 0;

	virtual void emit(shader_file* sh) = 0;
};
