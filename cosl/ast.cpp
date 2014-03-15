#include "ast.h"
#include "code_emitter.h"

#define dea(type) void type::emit(code_emitter* ce) { ce->emit(this); }
dea(s_type)
dea(decl_block)
dea(input_block)
dea(output_block)
dea(cbuffer_block)
dea(texture_def)
dea(id_primary)
dea(num_primary)
dea(func_invoke_primary)
dea(member_access_primary)
dea(mul_term)
dea(div_term)
dea(add_expr)
dea(sub_expr)
dea(true_bexpr)
dea(false_bexpr)
dea(binary_bexpr)
dea(not_bexpr)
dea(multi_stmt)
dea(block_stmt)
dea(assign_stmt)
dea(decl_stmt)
dea(if_stmt)
dea(while_stmt)
dea(for_stmt)
dea(func_invoke_stmt)
dea(return_stmt)
dea(sfunction)
dea(expr_in_paren)

void decl::emit(code_emitter* ce)
{
	ce->emit(*this);
}
