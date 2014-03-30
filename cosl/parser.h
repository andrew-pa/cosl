#pragma once
#include "cmmn.h"
#include "tokenizer.h"
#include "ast.h"

struct parser_exception : public exception
{
	token tkat;

	parser_exception(token t, const string& m)
		: exception(m.c_str()), tkat(t)
	{
	}

	const char* what() const override
	{
		ostringstream oss;
		oss << exception::what();
		oss << " @ token ";
		tkat.print(oss);
		char* c = new char[oss.str().size()];
		strcpy(c, oss.str().c_str());
		return c;
	}
};

expr* parse_expr(tokenizer& tk);
primary* parse_primary(tokenizer& tk);
term* parse_term(tokenizer& tk);
expr* parse_bool_expr(tokenizer& tk);

s_type* parse_type(tokenizer& tk);

stmt* parse_stmt(tokenizer& tk, bool allow_multi = true);
//stmt* parse_decl_stmt(tokenizer& tk, s_type* ppt);
//stmt* parse_assign_stmt(tokenizer& tk);
//stmt* parse_assign_stmt(tokenizer& tk, string* already_parsed_id);
//stmt* parse_if_stmt(tokenizer& tk);
//stmt* parse_while_stmt(tokenizer& tk);
//stmt* parse_for_stmt(tokenizer& tk);
//stmt* parse_return_stmt(tokenizer& tk);
semantic* parse_semantic(tokenizer& tk);

decl parse_decl(tokenizer& tk);
decl_block* parse_decl_block(tokenizer& tk);
input_block* parse_input_block(tokenizer& tk);
output_block* parse_output_block(tokenizer& tk);
cbuffer_block* parse_cbuffer_block(tokenizer& tk);
texture_def* parse_texture_def(tokenizer& tk);
sfunction* parse_function_def(tokenizer& tk);

shader_file* parse_shader(tokenizer& tk);