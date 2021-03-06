#include "parser.h"

primary* parse_primary(tokenizer& tk)
{
	auto t = tk.get_token();
	switch (t.tp)
	{
	case token_type::special:
	{
		if (t.s == "(")
		{
			expr* x = parse_expr(tk);
			primary* fi = new expr_in_paren(x);
			t = tk.get_token();
			if (t.s != ")") throw compile_exception("expected )", t.source_file, t.source_line);
			t = tk.get_token();
			if(t.tp == token_type::special && t.s == ".")
			{
				vector<string> m;
				while(true)
				{
					t = tk.get_token();
					if (t.tp != token_type::id)
						break;
					m.push_back(t.s);
					t = tk.get_token();
					if (t.tp == token_type::special && t.s == ".")
						continue;
					else
					{
						tk.put_back(t);
						break;
					}
				}
				fi =  new member_access_primary(fi, m);
			}
			if (t.tp == token_type::special && t.s == "[")
			{
				vector<expr*> idxes;
				while (t.s != "]")
				{
					if (t.s == ",")
					{
						idxes.push_back(parse_expr(tk));
						t = tk.get_token();
						continue;
					}
					idxes.push_back(parse_expr(tk));
					t = tk.get_token();
				}
				fi = new array_index_primary(fi, idxes);
			}
			else tk.put_back(t);
			return fi;
		}
		else if(t.s == "-")
		{
			expr* x = parse_expr(tk);
			primary* fi = new negation_primary(x);
			t = tk.get_token();
			if (t.tp == token_type::special && t.s == ".")
			{
				vector<string> m;
				while (true)
				{
					t = tk.get_token();
					if (t.tp != token_type::id)
						break;
					m.push_back(t.s);
					t = tk.get_token();
					if (t.tp == token_type::special && t.s == ".")
						continue;
					else
					{
						tk.put_back(t);
						break;
					}
				}
				fi = new member_access_primary(fi, m);
			}
			if (t.tp == token_type::special && t.s == "[")
			{
				vector<expr*> idxes;
				while (t.s != "]")
				{
					if (t.s == ",")
					{
						idxes.push_back(parse_expr(tk));
						t = tk.get_token();
						continue;
					}
					idxes.push_back(parse_expr(tk));
					t = tk.get_token();
				}
				fi = new array_index_primary(fi, idxes);
			}
			else tk.put_back(t);
			return fi;
		}
	}
		break;
	case token_type::id:
	{
		auto xt = tk.get_token();
		if (xt.tp == token_type::special && xt.s == "(")
		{
			string n = t.s;
			vector<expr*> args;
			t = xt;
			while (t.s != ")")
			{
				if (t.s == ",")
				{
					args.push_back(parse_expr(tk));
					t = tk.get_token();
					continue;
				}
				args.push_back(parse_expr(tk));
				t = tk.get_token();
			}
			primary* fi = new func_invoke_primary(n, args);
			t = tk.get_token();
			if (t.tp == token_type::special && t.s == ".")
			{
				vector<string> m;
				while (true)
				{
					t = tk.get_token();
					if (t.tp != token_type::id)
						break;
					m.push_back(t.s);
					t = tk.get_token();
					if (t.tp == token_type::special && t.s == ".")
						continue;
					else
					{
						tk.put_back(t);
						break;
					}
				}
				return new member_access_primary(fi, m);
			}
			if (t.tp == token_type::special && t.s == "[")
			{
				vector<expr*> idxes;
				while (t.s != "]")
				{
					if (t.s == ",")
					{
						idxes.push_back(parse_expr(tk));
						t = tk.get_token();
						continue;
					}
					idxes.push_back(parse_expr(tk));
					t = tk.get_token();
				}
				fi = new array_index_primary(fi, idxes);
			}
			else tk.put_back(t);
			return fi;
		}
		else
		{
			tk.put_back(xt);
			string tid = t.s;
			t = tk.get_token();
			primary* fi = new id_primary(tid);
			
			if(t.tp == token_type::special && t.s == "[")
			{
				vector<expr*> idxes;
				while (t.s != "]")
				{
					if (t.s == ",")
					{
						idxes.push_back(parse_expr(tk));
						t = tk.get_token();
						continue;
					}
					idxes.push_back(parse_expr(tk));
					t = tk.get_token();
				}
				fi = new array_index_primary(fi, idxes);
				t = tk.get_token();
			}
			if (t.tp == token_type::special && t.s == ".")
			{
				vector<string> m;
				while (true)
				{
					t = tk.get_token();
					if (t.tp != token_type::id)
						break;
					m.push_back(t.s);
					t = tk.get_token();
					if (t.tp == token_type::special && t.s == ".")
						continue;
					else
					{
						tk.put_back(t);
						break;
					}
				}
				fi = new member_access_primary(fi, m);
			}
			else tk.put_back(t);
			return fi;
		}
	}
		break;
	case token_type::number_lit:
		return new num_primary(t.s);
	}
	return nullptr;
}

id_primary* parse_id(tokenizer& tk)
{
	auto t = tk.get_token();
	if (t.tp != token_type::id) throw compile_exception("expected ID", t.source_file, t.source_line);
	string tid = t.s;
	t = tk.get_token();
	if (t.tp == token_type::special && t.s == ".")
	{
		vector<string> m;
		while (true)
		{
			t = tk.get_token();
			if (t.tp != token_type::id)
				break;
			m.push_back(t.s);
			t = tk.get_token();
			if (t.tp == token_type::special && t.s == ".")
				continue;
			else
			{
				tk.put_back(t);
				break;
			}
		}
		return new member_access_primary(new id_primary(tid), m);
	}
	tk.put_back(t);
	return new id_primary(tid);
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
		if(t.s == "*")
		{
			primary* tt = dynamic_cast<primary*>(parse_primary(tk));
			t = tk.get_token();
			x = new mul_term(x, tt);
		}
		else if(t.s == "/")
		{
			primary* tt = dynamic_cast<primary*>(parse_primary(tk));
			t = tk.get_token();
			x = new div_term(x, tt);
		}
		else
		{
			tk.put_back(t);
			return x;
		}
	}
}

expr* parse_bool_expr(tokenizer& tk)
{
	auto w = parse_expr(tk);
	if (dynamic_cast<bool_expr*>(w) == nullptr)
	{
		return new bool_expr_xvr(w);
	}
	return w;


	auto t = tk.get_token();
	if (t.tp == token_type::id)
	{
		if (t.s == "true") return new true_bexpr();
		else if (t.s == "false") return new false_bexpr();
	}
	if (t.tp == token_type::special && t.s == "!")
	{
		bool_expr* x = dynamic_cast<bool_expr*>(parse_bool_expr(tk));
		return new not_bexpr(x);
	}
	tk.put_back(t);
	expr* l = parse_expr(tk);
	t = tk.get_token();
	while (true)
	{
		if (t.tp == token_type::end) return l;
		if (t.tp != token_type::special)
		{
			t = tk.get_token(); continue;
		}
		else if (t.s == "(" || t.s == ")" || t.s == ";")
		{
			tk.put_back(t);
			break;
		}
		bool_op o = bool_op::less;
		if (t.s == "<") 
			o = bool_op::less;
		else if (t.s == ">") o = bool_op::greater;
		else if (t.s == "<=") o = bool_op::less_equal;
		else if (t.s == ">=") o = bool_op::greater_equal;
		else if (t.s == "==") o = bool_op::equal;
		else if (t.s == "!=") o = bool_op::not_equal;
		else if (t.s == "&&")
		{
			o = bool_op::and;
			expr* r = parse_bool_expr(tk);
			t = tk.get_token();
			l = new binary_bexpr(l, o, r);
			continue;
		}
		else if (t.s == "||")
		{
			o = bool_op::or;
			expr* r = parse_bool_expr(tk);
			t = tk.get_token();
			l = new binary_bexpr(l, o, r);
			continue;
		}
		else throw compile_exception("invalid bool operator", t.source_file, t.source_line);
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
	if(dynamic_cast<bool_expr*>(l) == nullptr)
	{
		return new bool_expr_xvr(l);
	}
	return l;
}

expr* parse_expr(tokenizer& tk)
{
	auto t = tk.get_token();
	if (t.tp == token_type::id)
	{
		if (t.s == "true") return new true_bexpr();
		else if (t.s == "false") return new false_bexpr();
	}	
	if (t.tp == token_type::special && t.s == "!")
	{
		return new not_bexpr(parse_expr(tk));
	}
	tk.put_back(t);
	expr* x = parse_term(tk);
	t = tk.get_token();
	while (true)
	{
		if (t.tp == token_type::end) return x;
		if (t.tp != token_type::special)
		{
			t = tk.get_token(); continue;
		}


		if(t.s == "+")
		{
			term* tt = parse_term(tk);
			t = tk.get_token();
			x = new add_expr(x, tt);
		}
		else if(t.s == "-")
		{
			term* tt = parse_term(tk);
			t = tk.get_token();
			x = new sub_expr(x, tt);
		}
		else if(t.s == "<")
		{
			expr* x2 = parse_expr(tk);
			t = tk.get_token();
			x = new binary_bexpr(x, bool_op::less, x2);
		}
		else if (t.s == ">")
		{
			expr* x2 = parse_expr(tk);
			t = tk.get_token();
			x = new binary_bexpr(x, bool_op::greater, x2);
		}
		else if (t.s == "<=")
		{
			expr* x2 = parse_expr(tk);
			t = tk.get_token();
			x = new binary_bexpr(x, bool_op::less_equal, x2);
		}
		else if (t.s == ">=")
		{
			expr* x2 = parse_expr(tk);
			t = tk.get_token();
			x = new binary_bexpr(x, bool_op::greater_equal, x2);
		}
		else if (t.s == "==")
		{
			expr* x2 = parse_expr(tk);
			t = tk.get_token();
			x = new binary_bexpr(x, bool_op::equal, x2);
		}
		else if (t.s == "!=")
		{
			expr* x2 = parse_expr(tk);
			t = tk.get_token();
			x = new binary_bexpr(x, bool_op::not_equal, x2);
		}
		else if (t.s == "&&")
		{
			expr* x2 = parse_expr(tk);
			t = tk.get_token();
			x = new binary_bexpr(x, bool_op::and, x2);
		}
		else if (t.s == "||")
		{
			expr* x2 = parse_expr(tk);
			t = tk.get_token();
			x = new binary_bexpr(x, bool_op::or, x2);
		}
		else
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
	auto at = tk.get_token();
	vector<uint> ad(0);
	if(at.tp == token_type::special && at.s == "[")
	{
		while(at.s != "]")
		{
			at = tk.get_token();
			if (at.tp == token_type::number_lit)
				ad.push_back(atoi(at.s.c_str()));
		}
	}
	else tk.put_back(at);
	if (t.s == "bool") return new s_type(base_s_type::bool_t,ad);
	else if (t.s == "int") return new s_type(base_s_type::int_t, ad);
	else if (t.s == "uint") return new s_type(base_s_type::uint_t, ad);
	else if (t.s == "float") return new s_type(base_s_type::float_t, ad);
	else if (t.s == "double") return new s_type(base_s_type::double_t, ad);
	else if (t.s == "void") return new s_type(base_s_type::void_t, ad);
	else
	{ //could use regex, but i'm lazy
		string base_type;
		bool seen_x = false;
		string vd, md;
		for (int i = 0; i < t.s.size(); ++i)
		{
			if (t.s[i] == 'x') seen_x = true;
			else if (!isdigit(t.s[i])) base_type += t.s[i];
			else
			{
				if (seen_x) md += t.s[i];
				else vd += t.s[i];
			}
		}
		if (md.empty()) //found vector or square mat
		{
			uint vecd = atoi(vd.c_str());
			if (base_type == "bvec") return new s_type(base_s_type::bvec_t, vecd, ad);
			else if (base_type == "ivec") return new s_type(base_s_type::ivec_t, vecd, ad);
			else if (base_type == "uvec") return new s_type(base_s_type::uvec_t, vecd, ad);
			else if (base_type == "vec") return new s_type(base_s_type::vec_t, vecd, ad);
			else if (base_type == "dvec") return new s_type(base_s_type::dvec_t, vecd, ad);
			else if (base_type == "mat") return new s_type(base_s_type::mat_t, vecd, vecd, ad);
			else if (base_type == "dmat") return new s_type(base_s_type::dmat_t, vecd, vecd, ad);
		}
		else
		{
			uint vecd = atoi(vd.c_str());
			uint matd = atoi(md.c_str());
			if (base_type == "mat") return new s_type(base_s_type::mat_t, vecd, matd, ad);
			else if (base_type == "dmat") return new s_type(base_s_type::dmat_t, vecd, matd, ad);
		}
	}
	return new s_type(t.s, ad); //must be udt or invalid
}

stmt* parse_decl_stmt(tokenizer& tk, s_type* ppt)
{
	auto t = tk.get_token();
	if (t.tp != token_type::id) throw compile_exception("invalid decl stmt, missing var name", t.source_file, t.source_line);
	string idn = t.s;
	t = tk.get_token();
	if (t.tp == token_type::special && t.s == "=")
	{
		if (ppt->type == base_s_type::bool_t)
			return new decl_stmt(ppt, idn, parse_bool_expr(tk));
		return new decl_stmt(ppt, idn, parse_expr(tk));
	}
	else
	{
		tk.put_back(t);
		return new decl_stmt(ppt, idn);
	}
	return nullptr;
}

stmt* parse_assign_stmt(tokenizer& tk)
{
	auto t = tk.get_token();
	if (t.tp == token_type::special)
	{
		if (t.s == "++")
		{
			return new assign_stmt(parse_id(tk), assign_op::pre_incr, nullptr);
		}
		else if (t.s == "--")
		{
			return new assign_stmt(parse_id(tk), assign_op::pre_deincr, nullptr);
		}
	}
	if (t.tp != token_type::id) throw compile_exception("invalid assign stmt, missing var name", t.source_file, t.source_line);
	tk.put_back(t);
	auto ddd = parse_id(tk);
	t = tk.get_token();
	if (t.s == "=")
	{
		expr* x = parse_expr(tk);
		return new assign_stmt(ddd, assign_op::equal, x);
	}
	else if (t.s == "+=")
	{
		expr* x = parse_expr(tk);
		return new assign_stmt(ddd, assign_op::plus_equal, x);
	}
	else if (t.s == "-=")
	{
		expr* x = parse_expr(tk);
		return new assign_stmt(ddd, assign_op::minus_equal, x);
	}
	else if (t.s == "*=")	{
		expr* x = parse_expr(tk);
		return new assign_stmt(ddd, assign_op::mul_equal, x);
	}
	else if (t.s == "/=")
	{
		expr* x = parse_expr(tk);
		return new assign_stmt(ddd, assign_op::div_equal, x);
	}
	else if (t.s == "++")
	{
		expr* x = parse_expr(tk);
		return new assign_stmt(ddd, assign_op::post_incr, x);
	}
	else if (t.s == "--")
	{
		expr* x = parse_expr(tk);
		return new assign_stmt(ddd, assign_op::post_deincr, x);
	}
	else
	{
		throw compile_exception("invalid assignment operator \"" + t.s + "\"", t.source_file, t.source_line);
	}
}

stmt* parse_assign_stmt(tokenizer& tk, id_primary* already_parsed_id)
{
	auto t = tk.get_token();
	auto ddd = already_parsed_id;
	if (t.s == "=")
	{
		expr* x = parse_expr(tk);
		return new assign_stmt(ddd, assign_op::equal, x);
	}
	else if (t.s == "+=")
	{
		expr* x = parse_expr(tk);
		return new assign_stmt(ddd, assign_op::plus_equal, x);
	}
	else if (t.s == "-=")
	{
		expr* x = parse_expr(tk);
		return new assign_stmt(ddd, assign_op::minus_equal, x);
	}
	else if (t.s == "*=")	{
		expr* x = parse_expr(tk);
		return new assign_stmt(ddd, assign_op::mul_equal, x);
	}
	else if (t.s == "/=")
	{
		expr* x = parse_expr(tk);
		return new assign_stmt(ddd, assign_op::div_equal, x);
	}
	else if (t.s == "++")
	{
		return new assign_stmt(ddd, assign_op::post_incr, nullptr);
	}
	else if (t.s == "--")
	{
		return new assign_stmt(ddd, assign_op::post_deincr, nullptr);
	}
	else
	{
		throw compile_exception("invalid assignment operator \"" + t.s + "\"", t.source_file, t.source_line);
	}
}

stmt* parse_if_stmt(tokenizer& tk)
{
	auto t = tk.get_token();
	if (t.tp != token_type::special && t.s != "(") 
		throw compile_exception("invalid if stmt, missing opening (", t.source_file, t.source_line); //eat opening (
	bool_expr* cond = dynamic_cast<bool_expr*>(parse_bool_expr(tk));
	t = tk.get_token();
	if (t.tp != token_type::special && t.s != ")") throw compile_exception("invalid if stmt, missing closing )", t.source_file, t.source_line); //eat closing )
	stmt* st = parse_stmt(tk, false);
	t = tk.get_token();
	if (t.tp == token_type::special && t.s == ";") //skip one line stmts + ;
		t = tk.get_token();
	if (t.tp == token_type::id && t.s == "else")
	{
		stmt* et = parse_stmt(tk, false);
		return new if_stmt(cond, st, et);
	}
	else tk.put_back(t);
	return new if_stmt(cond, st);
}

stmt* parse_while_stmt(tokenizer& tk)
{
	auto t = tk.get_token();
	if (t.tp != token_type::special && t.s != "(")
		throw compile_exception("invalid while stmt, missing opening (", t.source_file, t.source_line); //eat opening (
	bool_expr* cond = dynamic_cast<bool_expr*>(parse_bool_expr(tk));
	t = tk.get_token();
	if (t.tp != token_type::special && t.s != ")")
		throw compile_exception("invalid while stmt, missing closing )", t.source_file, t.source_line); //eat closing (
	stmt* st = parse_stmt(tk, false);
	return new while_stmt(cond, st);
}

stmt* parse_for_stmt(tokenizer& tk)
{
	auto t = tk.get_token();
	if (t.tp != token_type::special && t.s != "(")
		throw compile_exception("invalid for stmt, missing opening (", t.source_file, t.source_line); //eat opening (

	stmt* is = parse_stmt(tk, false);

	t = tk.get_token();
	if (t.tp != token_type::special && t.s != ";")
		throw compile_exception("invalid for stmt, missing ; between init stmt and condition", t.source_file, t.source_line); //eat ;

	bool_expr* cond = dynamic_cast<bool_expr*>(parse_bool_expr(tk));

	t = tk.get_token();
	if (t.tp != token_type::special && t.s != ";")
		throw compile_exception("invalid for stmt, missing ; between condition and increment stmt", t.source_file, t.source_line); //eat ;

	stmt* incrs = parse_stmt(tk, false);

	t = tk.get_token();
	if (t.tp != token_type::special && t.s != ")")
		throw compile_exception("invalid for stmt, missing closing )", t.source_file, t.source_line); //eat closing (

	stmt* b = parse_stmt(tk);

	return new for_stmt(is, cond, incrs, b);
}

stmt* parse_return_stmt(tokenizer& tk)
{
	auto t = tk.get_token();
	if (t.tp != token_type::end ||
		(t.tp != token_type::special && t.s != ";"))
	{
		tk.put_back(t);
		return new return_stmt(parse_expr(tk));
	}
	else
	{
		tk.put_back(t);
		return new return_stmt;
	}
}

stmt* parse_stmt(tokenizer& tk, bool allow_multi)
{
	stmt* s = nullptr;
	auto t = token();
	while (true)
	{
		t = tk.get_token();
		if (t.s == "}")
		{
			tk.put_back(t);
			return s;
		}
		if (t.tp == token_type::end) return s;
		if (t.tp == token_type::special && (t.s == "++" || t.s == "--"))
		{
			tk.put_back(t);
			s = parse_assign_stmt(tk);
		}
		else if (t.tp == token_type::id)
		{
			if (t.s == "if")
			{
				s = parse_if_stmt(tk);
				stmt* nx = parse_stmt(tk);
				if (nx != nullptr)
					s = new multi_stmt(s, nx);
			}
			else if (t.s == "while")
			{
				s = parse_while_stmt(tk);
				stmt* nx = parse_stmt(tk);
				if (nx != nullptr)
					s = new multi_stmt(s, nx);
			}
			else if (t.s == "for")
			{
				s = parse_for_stmt(tk);
				stmt* nx = parse_stmt(tk);
				if (nx != nullptr)
					s = new multi_stmt(s, nx);
			}
			else if (t.s == "return") s = parse_return_stmt(tk);
			else
			{
				tk.put_back(t);
				s_type* tp = parse_type(tk);
				t = tk.get_token();
				if(t.tp == token_type::special && t.s == ".")
				{
					vector<string> m;
					while (true)
					{
						t = tk.get_token();
						if (t.tp != token_type::id)
							break;
						m.push_back(t.s);
						t = tk.get_token();
						if (t.tp == token_type::special && t.s == ".")
							continue;
						else
							break;
					}
					member_access_primary* mp = new member_access_primary(new id_primary(*tp->udt_name), m);

					tk.put_back(t);
					s = parse_assign_stmt(tk, mp);
				}
				else if (tp != nullptr) //decl stmt
				{
					if (t.tp == token_type::special && t.s == "(")
					{
						t = tk.get_token();
						string id = *tp->udt_name;
						vector<expr*> args;
						while (t.s != ")")
						{
							if (t.tp == token_type::special && t.s == ",")
							{
								t = tk.get_token();
								continue;
							}

							tk.put_back(t);

							args.push_back(parse_expr(tk));

							t = tk.get_token();
						}
						s = new func_invoke_stmt(id, args);
					}
					else if (t.tp == token_type::id)
					{
						tk.put_back(t);
						s = parse_decl_stmt(tk, tp);
					}
					else
					{
						//assign stmt w/ "udt" name as <id>
						tk.put_back(t);
						s = parse_assign_stmt(tk, new id_primary(*tp->udt_name));
					}
				}
				else
				{
					//assign stmt
					tk.put_back(t);
					s = parse_assign_stmt(tk);
				}
			}
		}
		if (t.tp == token_type::special)
		{
			if (t.s == ";" && allow_multi)
			{
				s = new multi_stmt(s, parse_stmt(tk));
			}
			else if (t.s == "{")
			{
				stmt* ns = parse_stmt(tk);
				t = tk.get_token();
				if (t.s != "}") throw compile_exception("missing closing } of block stmt", t.source_file, t.source_line);
				//stmt* nx = parse_stmt(tk);
				return new block_stmt(ns);
			}
		}
		if (!allow_multi) return s;
	}
}

semantic* parse_semantic(tokenizer& tk)
{
	auto t = tk.get_token();
	if (t.tp != token_type::id) throw compile_exception("invalid semantic, not a ID (got: \"" + t.s + "\"", t.source_file, t.source_line);
	string n = t.s;
	int i = 0;
	string nm;
	for (; i < n.size() && !isdigit(n[i]); ++i) nm += n[i];
	string ix = n.substr(i);
	return new semantic(nm, (ix.length() == 0 ? 0xFFFFFFFF : atoi(ix.c_str())));
}

decl parse_decl(tokenizer& tk)
{
	s_type* tp = parse_type(tk);
	if (tp == nullptr) throw compile_exception("invalid type for decl", "unknown", 0);
	auto t = tk.get_token();
	if (t.tp != token_type::id) throw compile_exception("invalid decl, expected name, got: \"" + t.s + "\"", t.source_file, t.source_line);
	string nm = t.s;
	t = tk.get_token();
	if (t.s != ":")
	{
		tk.put_back(t);
		return decl(tp, nm, nullptr);
	}
	return decl(tp, nm, parse_semantic(tk));
}

decl_block* parse_decl_block(tokenizer& tk)
{
	auto t = tk.get_token();
	if (!(t.tp == token_type::special && t.s == "{"))
		throw compile_exception("decl block missing opening {", t.source_file, t.source_line);

	vector<decl> dcl;

	auto xt = tk.get_token();
	if (xt.tp == token_type::special && xt.s == "}")
	{
		//this decl block is empty so return w/ empty vector
		return new decl_block(dcl); 
	}
	tk.put_back(xt);

	while (!(t.tp == token_type::special && t.s == "}"))
	{
		if (t.tp == token_type::special && t.s == ";")
		{
			t = tk.get_token();
			if (t.tp == token_type::special && t.s == "}")
			{
				continue;
			}
			tk.put_back(t);
			dcl.push_back(parse_decl(tk));
			t = tk.get_token();
			continue;
		}
		dcl.push_back(parse_decl(tk));
		t = tk.get_token();
	}

	return new decl_block(dcl);
}

prim_type parse_prim_type(tokenizer& tk)
{
	auto t = tk.get_token();
	if (t.tp != token_type::id) throw compile_exception("invalid primitive type: " + t.s, t.source_file, t.source_line);
	if (t.s == "point")
		return prim_type::point;
	else if (t.s == "line")
		return prim_type::line;
	else if (t.s == "triangle")
		return prim_type::triangle;
	else if (t.s == "line_adj")
		return prim_type::line_adj;
	else if (t.s == "triangle_adj")
		return prim_type::triangle_adj;
	else
		throw compile_exception("invalid primitive type: " + t.s, t.source_file, t.source_line);
}

input_block* parse_input_block(tokenizer& tk)
{
	auto t = tk.get_token();
	if(t.tp == token_type::special && t.s == "(") //geometry shader 
	{
		prim_type tp = parse_prim_type(tk);
		t = tk.get_token();
		if (t.tp != token_type::special && t.s != ")") throw compile_exception("missing closing )", t.source_file, t.source_line);
		return new input_block(parse_decl_block(tk), tp);
	}
	tk.put_back(t);
	return new input_block(parse_decl_block(tk));
}
output_block* parse_output_block(tokenizer& tk)
{
	auto t = tk.get_token();
	if (t.tp == token_type::special && t.s == "(") //geometry shader 
	{
		prim_type tp = parse_prim_type(tk);
		t = tk.get_token();
		if (t.tp != token_type::special && t.s != ")") throw compile_exception("missing closing )", t.source_file, t.source_line);
		return new output_block(parse_decl_block(tk), tp);
	}
	tk.put_back(t); 
	return new output_block(parse_decl_block(tk));
}
cbuffer_block* parse_cbuffer_block(tokenizer& tk)
{
	auto t = tk.get_token();
	/*if (t.s != "cbuffer") throw compile_exception(t, "invalid cbuffer block");*/
	//t = tk.get_token();
	if (t.s != "(") throw compile_exception("invalid cbuffer block, missing opening ( for register idx", t.source_file, t.source_line);
	t = tk.get_token();
	if (t.tp != token_type::number_lit) throw compile_exception("invalid cbuffer block, expected a numberic register idx, got: \"" + t.s + "\"", t.source_file, t.source_line);
	uint idx = atoi(t.s.c_str());
	t = tk.get_token();
	if (t.s != ")") throw compile_exception("invalid cbuffer block, missing closing ) for register idx", t.source_file, t.source_line);
	decl_block* dcl = parse_decl_block(tk);
	return new cbuffer_block(idx, dcl);
}
struct_block* parse_struct_block(tokenizer& tk)
{
	auto t = tk.get_token();
	if (t.tp != token_type::id) throw compile_exception("invalid struct def, expected a name as a ID, got: \"" + t.s + "\"", t.source_file, t.source_line);
	decl_block* d = parse_decl_block(tk);
	return new struct_block(t.s, d);
}

texture_def* parse_texture_def(tokenizer& tk)
{
	auto t = tk.get_token();
	if (t.tp != token_type::id) throw compile_exception("invalid texture def, missing texture type, got: \""+t.s+"\"", t.source_file, t.source_line);
	texture_type txtp;
	if (t.s == "texture1D") txtp = texture_type::texture1D;
	if (t.s == "texture2D") txtp = texture_type::texture2D;
	if (t.s == "texture3D") txtp = texture_type::texture3D;
	if (t.s == "textureCube") txtp = texture_type::textureCube;
	t = tk.get_token();
	if (t.tp != token_type::id) throw compile_exception("invalid texture def, missing texture name, got: \"" + t.s +"\"", t.source_file, t.source_line);
	string nm = t.s;

	t = tk.get_token();
	if (t.tp != token_type::special && t.s != "(") throw compile_exception("invalid texture def: missing opening (", t.source_file, t.source_line);

	t = tk.get_token();
	if (t.tp != token_type::number_lit) throw compile_exception("invalid texture def: invalid idx", t.source_file, t.source_line);
	uint ix = atoi(t.s.c_str());

	t = tk.get_token();
	if (t.tp != token_type::special && t.s != ")") throw compile_exception("invalid texture def: missing closing )", t.source_file, t.source_line);

	return new texture_def(txtp, nm, ix);
}

sfunction* parse_function_def(tokenizer& tk)
{
	s_type* tp = parse_type(tk);
	auto t = tk.get_token();
	if (t.tp != token_type::id) throw compile_exception("invalid function def, missing name", t.source_file, t.source_line);
	string name = t.s;
	t = tk.get_token();
	if (t.tp != token_type::special && t.s != "(") throw compile_exception("invalid function def, missing opening ( for arguments", t.source_file, t.source_line);

	vector<sfunction::func_arg> args;

	t = tk.get_token();
	if (t.tp != token_type::special && t.s != ")")
	{
		tk.put_back(t);
		while (!(t.tp == token_type::special && t.s == ")"))
		{
			t = tk.get_token();
			if (t.s == ",")
			{
				continue;
			}
			tk.put_back(t);

			s_type* at = parse_type(tk);
			t = tk.get_token();
			if (t.tp != token_type::id) throw compile_exception("invalid function arg, missing argument name", t.source_file, t.source_line);
			args.push_back(sfunction::func_arg(at, t.s));
			t = tk.get_token();
		}
	}

	t = tk.get_token();
	if (t.tp != token_type::special && t.s != "{") throw compile_exception("invalid function def, missing opening { for body", t.source_file, t.source_line);

	auto st = parse_stmt(tk);

	t = tk.get_token();
	if (t.tp != token_type::special && t.s != "}") throw compile_exception("invalid function def, missing closing } for body", t.source_file, t.source_line);

	return new sfunction(tp, name, args, st);
}

shader_file* parse_shader(tokenizer& tk)
{
	shader_file* sf = new shader_file;
	auto t = tk.get_token();
	if (t.tp != token_type::id) throw compile_exception("shader type decl missing", t.source_file, t.source_line);
	if (t.s == "$vertex_shader") sf->type = shader_type::vertex_shader;
	else if (t.s == "$pixel_shader") sf->type = shader_type::pixel_shader;
	else if (t.s == "$geometry_shader")
	{
		sf->type = shader_type::geometry_shader;
		t = tk.get_token();
		if (t.tp != token_type::special || t.s != "(") throw compile_exception("geometry shader type decl missing opening (", t.source_file, t.source_line);
		t = tk.get_token();
		if (t.tp != token_type::number_lit) throw compile_exception("geometry shader type decl missing max vertex count", t.source_file, t.source_line);
		sf->geometry_shader_v.max_vertices = atoi(t.s.c_str());
		t = tk.get_token();
		if (t.tp != token_type::special || t.s != ")") throw compile_exception("geometry shader type decl missing closing )", t.source_file, t.source_line);
	}
	else throw compile_exception("invalid shader type", t.source_file, t.source_line);

	while (t.tp != token_type::end)
	{
		t = tk.get_token();
		if (t.tp == token_type::id)
		{
			if (t.s == "input")
			{
				sf->inpblk = parse_input_block(tk);
				continue;
			}
			else if (t.s == "output")
			{
				sf->outblk = parse_output_block(tk);
				continue;
			}
			else if (t.s == "cbuffer")
			{
				sf->cbuffers.push_back(parse_cbuffer_block(tk));
				continue;
			}
			else if (t.s == "texture1D" || t.s == "texture2D" || t.s == "texture3D" || t.s == "textureCube")
			{
				tk.put_back(t);
				sf->texturedefs.push_back(parse_texture_def(tk));
				continue;
			}
			else if(t.s == "struct")
			{
				sf->structdefs.push_back(parse_struct_block(tk));
				continue;
			}
			else
			{
				tk.put_back(t);
				sf->functions.push_back(parse_function_def(tk));
				continue;
			}
		}
		if (t.tp == token_type::end || t.s == "") break;
	}
	return sf;
}