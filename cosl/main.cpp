#include "cmmn.h"
#include "tokenizer.h"
#include "ast.h"

expr* parse_expr(tokenizer& tk);

primary* parse_primary(tokenizer& tk)
{
	auto t = tk.get_token();
	switch (t.tp)
	{
	case token_type::special:
		{
			if(t.s == "(")
			{
				expr* x = parse_expr(tk);
				t = tk.get_token();
				if (t.s != ")") throw exception("expected )");
				return (primary*)(x); //nasty hacks
			}
		}
		break;
	case token_type::id:
		{
			auto xt = tk.get_token();
			if(xt.tp == token_type::special && xt.s == "(")
			{
				string n = t.s;
				vector<expr*> args;
				t = xt;
				while(t.s != ")")
				{
					if (t.s == ",")
					{
						t = tk.get_token();
						continue;
					}
					args.push_back(parse_expr(tk));
					t = tk.get_token();
				}
				return new func_invoke_primary(n, args);
			}
			else
			{
				tk.put_back(xt);
				return new id_primary(id(t.s));
			}
		}
		break;
	case token_type::number_lit:
		return new num_primary(t.s);
	}
	return nullptr;
}

term* parse_term(tokenizer& tk)
{
	term* x = dynamic_cast<term*>(parse_primary(tk));
	auto t = tk.get_token();
	while (true)
	{
		if (t.tp == token_type::end) return x;
		if (t.tp != token_type::special)
		{
			t = tk.get_token(); continue;
		}
		auto v = matchv<string, int>(t.s,
		{
			{ "*", [&]
			{
				primary* tt = dynamic_cast<primary*>(parse_primary(tk));
				t = tk.get_token();
				x = new mul_term(x, tt);
				return 0;
			}
			},
			{ "/", [&]
			{
				primary* tt = dynamic_cast<primary*>(parse_primary(tk));
				t = tk.get_token();
				x = new div_term(x, tt);
				return 0;
			}
			},
		}, -1);
		if (v == -1)
		{
			tk.put_back(t);
			return x;
		}
	}
}

expr* parse_bool_expr(tokenizer& tk)
{
	auto t = tk.get_token();
	if (t.tp == token_type::id)
	{
		if (t.s == "true") return new true_bexpr();
		else if (t.s == "false") return new false_bexpr();
	}
	if(t.tp == token_type::special && t.s == "!")
	{
		bool_expr* x = dynamic_cast<bool_expr*>(parse_bool_expr(tk));
		return new not_bexpr(x);
	}
	tk.put_back(t);
	expr* l = parse_expr(tk);
	t = tk.get_token();
	while(true)
	{
		if (t.tp == token_type::end) return l;
		if (t.tp != token_type::special) 
		{
			t = tk.get_token(); continue;
		}
		bool_op o;
		if (t.s == "<") o = bool_op::less;
		else if (t.s == ">") o = bool_op::greater;
		else if (t.s == "<=") o = bool_op::less_equal;
		else if (t.s == ">=") o = bool_op::greater_equal;
		else if (t.s == "==") o = bool_op::equal;
		else if (t.s == "!=") o = bool_op::not_equal;
		else if (t.s == "&&") o = bool_op::and;
		else if (t.s == "||") o = bool_op::or;
		/*bool_op o = matchv<string, bool_op>(t.s,
		{
			{ "<",  [&] { return bool_op::less;			 } },
			{ ">",  [&] { return bool_op::greater;       } },
			{ "<=", [&] { return bool_op::less_equal;	 } },
			{ ">=", [&] { return bool_op::greater_equal; } },
			{ "==", [&] { return bool_op::equal;		 } },
			{ "!=", [&] { return bool_op::not_equal;	 } },
			{ "&&", [&] { return bool_op::and;			 } },
			{ "||", [&] { return bool_op::or;			 } },

		});*/
		expr* r = parse_expr(tk);
		t = tk.get_token();
		l = new binary_bexpr(l, o, r);
	}
	return l;
}

expr* parse_expr(tokenizer& tk)
{
	//auto t = tk.get_token();
	//if (t.tp == token_type::end) return nullptr;
	//if(t.tp == token_type::special && t.s == "(")
	//{
	//	cout << "(";
	//	auto x = parse_expr(tk);
	//	t = tk.get_token();
	//	if (t.tp != token_type::special || t.s != ")") throw exception("no closing )");
	//	cout << ")";
	//	return x;
	//}
	//else*/ if (t.tp == token_type::id || t.tp == token_type::number_lit || (t.tp == token_type::special && t.s == "("))
	//{
	//	cout << t.s << "_";
	//	if (t.s == "true") 
	//		return new true_expr();
	//	if (t.s == "false") 
	//		return new false_expr();
	//	expr* f;
	//	string ots = t.s;
	//	if (t.tp == token_type::id) 
	//	{
	//		t = tk.get_token();
	//		if(t.tp == token_type::special && t.s == "(")
	//		{
	//			t = tk.get_token();
	//			vector<expr*> ags;
	//			while (t.s != ")" && t.tp != token_type::end)
	//			{
	//				if (t.tp == token_type::special && t.s == ",")
	//				{
	//					t = tk.get_token();
	//					continue;
	//				}
	//				ags.push_back(parse_expr(tk));
	//				t = tk.get_token();
	//			}
	//			cout << "$";
	//			f = new func_invoke_expr(ots, ags);
	//		}
	//		else
	//		{
	//			tk.put_back(t);
	//			f = new id_expr(t.s);
	//		}
	//	}
	//	else if (t.tp == token_type::number_lit) f = new num_expr(t.s);
	//	else if (t.tp == token_type::special && t.s == "(")
	//	{
	//		auto x = parse_expr(tk);
	//		t = tk.get_token();
	//		if (t.tp != token_type::special || t.s != ")") throw exception("no closing )");
	//		f = x;
	//	}
	//	t = tk.get_token();
	//	if (t.tp == token_type::end) return nullptr;
	//	cout << t.s << "_";
	//	if(t.tp == token_type::special)
	//	{
	//		auto v = matchv<string, expr*>(t.s, 
	//		{
	//			{ "*", [&] {
	//				return new mul_term(f, parse_expr(tk));
	//				}
	//			},
	//			{ "/", [&] {
	//				return new div_term(f, parse_expr(tk));
	//				}
	//			},
	//			{ "+", [&] {
	//					return new add_expr(f, parse_expr(tk)); 
	//				} 
	//			},
	//			{ "-", [&] {
	//					return new sub_expr(f, parse_expr(tk));
	//				}
	//			},
	//			
	//			{ "<=", [&] {
	//					return new binary_bool_expr(f, bool_op::less_equal, parse_expr(tk));
	//				}
	//			},
	//			{ ">=", [&] {
	//					return new binary_bool_expr(f, bool_op::greater_equal, parse_expr(tk));
	//				}
	//			},
	//			{ "<", [&] {
	//					return new binary_bool_expr(f, bool_op::less, parse_expr(tk));
	//				}
	//			},
	//			{ ">", [&] {
	//					return new binary_bool_expr(f, bool_op::greater, parse_expr(tk));
	//				}
	//			},
	//			{ "==", [&] {
	//					return new binary_bool_expr(f, bool_op::equal, parse_expr(tk));
	//				}
	//			},
	//			{ "!=", [&] {
	//					return new binary_bool_expr(f, bool_op::not_equal, parse_expr(tk));
	//				}
	//			},
	//			{ "&&", [&] {
	//					return new binary_bool_expr(f, bool_op::and, parse_expr(tk));
	//				}
	//			},
	//			{ "||", [&] {
	//					return new binary_bool_expr(f, bool_op::or, parse_expr(tk));
	//				}
	//			},
	//		}, nullptr);
	//		if(v == nullptr)
	//		{
	//			tk.put_back(t);
	//			return f;
	//		}
	//		return v;
	//	}
	//}
	//else if(t.tp == token_type::special)
	//{
	//	if(t.s == "!")
	//	{
	//		cout << "!" << "_";
	//		return new not_bool_expr(parse_expr(tk));
	//	}
	//}
	//tk.put_back(t);
	//return nullptr;
	auto t = tk.get_token();
	if (t.tp == token_type::id)
	{
		if (t.s == "true") return new true_bexpr();
		else if (t.s == "false") return new false_bexpr();
	}
	else tk.put_back(t);
	expr* x = parse_term(tk);
	t = tk.get_token();
	while(true)
	{
		if (t.tp == token_type::end) return x;
		if (t.tp != token_type::special) 
		{
			t = tk.get_token(); continue;
		}
		auto v = matchv<string, int>(t.s,
		{
			{ "+", [&]
				{ 
					term* tt = parse_term(tk);
					t = tk.get_token();
					x = new add_expr(x,  tt);
					return 0; 
				} 
			},
			{ "-", [&]
				{
					term* tt = parse_term(tk);
					t = tk.get_token();
					x = new sub_expr(x, tt);
					return 0;
				} 
			},
		}, -1);
		if(v == -1)
		{
			tk.put_back(t);
			return x;
		}
	}
}

s_type* parse_type(tokenizer& tk)
{
	auto t = tk.get_token();
	if (t.tp != token_type::id) return nullptr;
	if (t.s == "bool") return new s_type(base_s_type::bool_t);
	else if (t.s == "int") return new s_type(base_s_type::int_t);
	else if (t.s == "uint") return new s_type(base_s_type::uint_t);
	else if (t.s == "float") return new s_type(base_s_type::float_t);
	else if (t.s == "double") return new s_type(base_s_type::double_t);
	else
	{ //could use regex, but i'm lazy
		string base_type;
		bool seen_x = false;
		string vd, md;
		for (int i = 0; i < t.s.size(); ++i)
		{
			if (!isdigit(t.s[i])) base_type += t.s[i];
			else
			{
				if (seen_x) md += t.s[i];
				else vd += t.s[i];
			}
		}
		if(md.empty()) //found vector or square mat
		{
			uint vecd = atoi(vd.c_str());
			if (base_type == "bvec") return new s_type(base_s_type::bvec_t, vecd);
			else if (base_type == "ivec") return new s_type(base_s_type::ivec_t, vecd);
			else if (base_type == "uvec") return new s_type(base_s_type::uvec_t, vecd);
			else if (base_type == "vec") return new s_type(base_s_type::vec_t, vecd);
			else if (base_type == "dvec") return new s_type(base_s_type::dvec_t, vecd);
			else if (base_type == "mat") return new s_type(base_s_type::mat_t, vecd, vecd);
			else if (base_type == "dmat") return new s_type(base_s_type::dmat_t, vecd, vecd);
		}
		else
		{
			uint vecd = atoi(vd.c_str());
			uint matd = atoi(md.c_str());
			if (base_type == "mat") return new s_type(base_s_type::mat_t, vecd, matd);
			else if (base_type == "dmat") return new s_type(base_s_type::dmat_t, vecd, matd);
		}
	}
	return new s_type(t.s); //must be udt or invalid
}

stmt* parse_decl_stmt(tokenizer& tk, s_type* ppt)
{
	auto t = tk.get_token();
	if (t.tp != token_type::id) throw exception("invalid decl stmt");
	string idn = t.s;
	t = tk.get_token();
	if(t.tp == token_type::special && t.s == "=")
	{
		return new decl_stmt(ppt, idn, parse_expr(tk));
	}
	else
	{
		tk.put_back(t);
		return new decl_stmt(ppt, idn);
	}
}

stmt* parse_assign_stmt(tokenizer& tk)
{
	auto t = tk.get_token();
	if (t.tp != token_type::id) throw exception("invalid assign stmt");
	id ddd(t.s);
	t = tk.get_token();
	return matchv<string, stmt*>(t.s,
	{
		{ "=", [&] 
		{
			expr* x = parse_expr(tk);
			return new assign_stmt(ddd, assign_op::equal, x);
		},
	}
	}, [&] { throw exception("invalid assignment operator"); });
}

stmt* parse_assign_stmt(tokenizer& tk, string* already_parsed_id)
{
}

stmt* parse_if_stmt(tokenizer& tk)
{
	return nullptr;
}
stmt* parse_while_stmt(tokenizer& tk)
{
	return nullptr;
}
stmt* parse_for_stmt(tokenizer& tk)
{
	return nullptr;
}
stmt* parse_return_stmt(tokenizer& tk)
{
	return nullptr;
}

stmt* parse_stmt(tokenizer& tk)
{
	auto t = tk.get_token();
	if(t.tp == token_type::id)
	{
		return matchv<string, stmt*>(t.s,
		{
			{ "if", [&] { return parse_if_stmt(tk); } },
			{ "while", [&] { return parse_while_stmt(tk); }  },
			{ "for", [&] { return parse_for_stmt(tk); }  },
			{ "return", [&] { return parse_return_stmt(tk); }  },
		}, [&] 
		{
			tk.put_back(t);
			s_type* tp = parse_type(tk);
			t = tk.get_token();
			if(tp != nullptr) //decl stmt
			{
				if (t.tp == token_type::id)
				{
					tk.put_back(t);
					return parse_decl_stmt(tk, tp);
				}
				else
				{
					//assign stmt w/ "udt" name as <id>
					tk.put_back(t);
					return parse_assign_stmt(tk, tp->udt_name);
				}
			}
			else
			{
				if (t.tp == token_type::id)
				{
					//assign stmt
					tk.put_back(t);
					return parse_assign_stmt(tk);
				}
			}
			throw exception("invalid stmt");
		});
	}
}


int main()
{
	tokenizer tkn("");
	auto x = parse_bool_expr(tkn);
	getchar();
}