#pragma once
#include "cmmn.h"
#include "tokenizer.h"
#include "ast.h"
#include "parser.h"
#include "code_emitter.h"


class hlsl_code_emmiter : public code_emitter
{
	shader_type mode;

	string translate_semantic(semantic* s)
	{
		if (s->name == "rs_position")
		{
			return string("SV_POSITION");
		}
		else if (s->name == "target")
		{
			ostringstream oss;
			oss << "SV_TARGET" << s->idx;
			return oss.str();
		}
		else if (s->name == "depth")
		{
			ostringstream oss;
			oss << "SV_DEPTH" << s->idx;
			return oss.str();
		}
		else
		{
			ostringstream oss;
			string sem = s->name;
			for (auto& c : sem)c = toupper(c);
			oss << sem;
			if (s->idx != 0xFFFFFFFF) oss << s->idx;
			return oss.str();
		}
	}
	enum special_func_type
	{
		none, vector, sample,
	}; 
	bool is_special_func(const string& n)
	{
		{ //could use regex, but i'm lazy
			string base_type;
			bool seen_x = false;
			string vd, md;
			for (int i = 0; i < n.size(); ++i)
			{
				if (n[i] == 'x') seen_x = true;
				else if (!isdigit(n[i])) base_type += n[i];
				else
				{
					if (seen_x) md += n[i];
					else vd += n[i];
				}
			}
			if (md.empty()) //found vector or square mat
			{
				uint vecd = atoi(vd.c_str());
				if (base_type == "bvec") return true;
				else if (base_type == "ivec") return true;
				else if (base_type == "uvec") return true;
				else if (base_type == "vec") return true;
				else if (base_type == "dvec") return true;
				else if (base_type == "mat") return true;
				else if (base_type == "dmat") return true;
			}
			else
			{
				uint vecd = atoi(vd.c_str());
				uint matd = atoi(md.c_str());
				if (base_type == "mat") return true;
				else if (base_type == "dmat") return true;
			}
		}
		if (n == "sample1D" || n == "sample2D" || n == "sample3D" || n == "sampleCube") return true;
		return false;
	}
	bool is_special_func(const string& n, special_func_type& t, string& base_type, uint& vecd, uint& matd)
	{
		t = special_func_type::vector;
		{ //could use regex, but i'm lazy
			bool seen_x = false;
			string vd, md;
			for (int i = 0; i < n.size(); ++i)
			{
				if (n[i] == 'x') seen_x = true;
				else if (!isdigit(n[i])) base_type += n[i];
				else
				{
					if (seen_x) md += n[i];
					else vd += n[i];
				}
			}
			if (md.empty()) //found vector or square mat
			{
				vecd = atoi(vd.c_str());
				matd = vecd;
				if (base_type == "bvec") return true;
				else if (base_type == "ivec") return true;
				else if (base_type == "uvec") return true;
				else if (base_type == "vec") return true;
				else if (base_type == "dvec") return true;
				else if (base_type == "mat") return true;
				else if (base_type == "dmat") return true;
			}
			else
			{
				vecd = atoi(vd.c_str());
				matd = atoi(md.c_str());
				if (base_type == "mat") return true;
				else if (base_type == "dmat") return true;
			}
		}
		t = special_func_type::sample;
		if (n == "sample1D" || n == "sample2D" || n == "sample3D" || n == "sampleCube") return true;
		t = special_func_type::none;
		return false;
	}

	void emit_special_func(func_invoke_primary* x)
	{
		special_func_type t; string bt; uint vd, md;
		is_special_func(x->func_name, t, bt, vd, md);
		if(t == special_func_type::vector)
		{
			if (bt == "bvec") _out << "bool" << vd;
			else if (bt == "ivec") _out << "int" << vd;
			else if (bt == "uvec") _out << "uint" << vd;
			else if (bt == "vec") _out << "float" << vd;
			else if (bt == "dvec") _out << "double" << vd;
			else if (bt == "mat") _out << "float" << vd << "x" << md;
			else if (bt == "dmat") _out << "double" << vd << "x" << md;
		}
		else if(t == special_func_type::sample)
		{
			x->args[0]->emit(this);
			_out << ".Sample(___smp_" << dynamic_cast<id_primary*>(x->args[0])->_id.base_name << ",";
			x->args[1]->emit(this);
			_out << ")";
		}
	}

	void emit_special_func(func_invoke_stmt* x)
	{
		special_func_type t; string bt; uint vd, md;
		is_special_func(x->func_name, t, bt, vd, md);
		if (t == special_func_type::vector)
		{
			if (bt == "bvec") _out << "bool" << vd;
			else if (bt == "ivec") _out << "int" << vd;
			else if (bt == "uvec") _out << "uint" << vd;
			else if (bt == "vec") _out << "float" << vd;
			else if (bt == "dvec") _out << "double" << vd;
			else if (bt == "mat") _out << "float" << vd << "x" << md;
			else if (bt == "dmat") _out << "double" << vd << "x" << md;
		}
		else if (t == special_func_type::sample)
		{
			x->args[0]->emit(this);
			_out << ".Sample(___smp_" << dynamic_cast<id_primary*>(x->args[0])->_id.base_name << ",";
			x->args[1]->emit(this);
			_out << ")";
		}
	}

	ostringstream _out;
public:
	hlsl_code_emmiter()
	{
	}

	string out_string() const override { return _out.str();  }

	void reset() override
	{
		_out = ostringstream();
	}

	void emit(shader_file* sh)override
	{
		_out << "#pragma pack_matrix( row_major )" << endl;
		emit(sh->type);
		sh->inpblk->emit(this);
		sh->outblk->emit(this);
		for (auto& cb : sh->cbuffers)
			cb->emit(this);
		for (auto& tx : sh->texturedefs)
			tx->emit(this);
		for (auto& fn : sh->functions)
			fn->emit(this);
	}

	void emit(shader_type x)override
	{
		mode = x;
	}

	void emit(s_type* x)override
	{
		switch (x->type)
		{
		case base_s_type::bvec_t:
			_out << "bool";
			_out << x->vecdim;
			break;
		case base_s_type::bool_t:
			_out << "bool";
			break;
		case base_s_type::ivec_t:
			_out << "int";
			_out << x->vecdim;
			break;
		case base_s_type::int_t:
			_out << "int";
			break;
		case base_s_type::uvec_t:
			_out << "uint";
			_out << x->vecdim;
			break;
		case base_s_type::uint_t:
			_out << "uint";
			break;
		case base_s_type::vec_t:
			_out << "float";
			_out << x->vecdim;
			break;
		case base_s_type::float_t:
			_out << "float";
			break;
		case base_s_type::dvec_t:
			_out << "double";
			_out << x->vecdim;
			break;
		case base_s_type::double_t:
			_out << "double";
			break;
		case base_s_type::mat_t:
			_out << "float";
			_out << x->vecdim;
			_out << "x";
			_out << x->matdim;
			break;
		case base_s_type::dmat_t:
			_out << "double";
			_out << x->vecdim;
			_out << "x";
			_out << x->matdim;
			break;
		case base_s_type::user_def_t:
			_out << *x->udt_name;
			break;
		case base_s_type::void_t:
			_out << "void";
			break;
		}
	}

	void emit(decl& x)override
	{
		x.type->emit(this);
		_out << " " << x.name;
		if (x.sem != nullptr)
			_out << " : " << translate_semantic(x.sem);
	}

	void emit(decl_block* x)override
	{
		_out << "{" << endl;
		for (auto& d : x->decls)
		{
			d.emit(this);
			_out << ";" << endl;
		}
		_out << "}";
	}

	void emit(input_block* x)override
	{
		_out << "struct __shader_input_t" << endl;
		x->db->emit(this);
		_out << ";" << endl;
	}

	void emit(output_block* x)override
	{
		_out << "struct __shader_output_t" << endl;
		x->db->emit(this);
		_out << ";" << endl;
	}

	void emit(cbuffer_block* x)override
	{
		_out << "cbuffer ___reg_" << x->reg_idx
			<< "___" << " : register(b"
			<< x->reg_idx << ")";
		x->dclbk->emit(this);
		_out << ";" << endl;
	}

	void emit(texture_def* x)override
	{
		switch (x->type)
		{
		case texture_type::texture1D:
			_out << "Texture1D " << x->name
				<< " : register(t" << x->reg_idx << ");";
			break;
		case texture_type::texture2D:
			_out << "Texture2D " << x->name
				<< " : register(t" << x->reg_idx << ");";
			break;
		case texture_type::texture3D:
			_out << "Texture3D " << x->name
				<< " : register(t" << x->reg_idx << ");";
			break;
		case texture_type::textureCube:
			_out << "TextureCube " << x->name
				<< " : register(t" << x->reg_idx << ");";
			break;
		}
		_out << endl << "SamplerState ___smp_" << x->name << " : register(s" << x->reg_idx << ");" << endl;
	}

	void emit(const id& x)override
	{
		_out << x.base_name;
		for (auto& n : x.members)
		{
			_out << "." << n;
		}
	}

	// void emit(expr* x)= 0;
	// void emit(term* x)= 0;
	// void emit(primary* x)= 0;
	void emit(id_primary* x)override
	{
		x->_id.emit(this);
	}

	void emit(num_primary* x)override
	{
		_out << x->num;
	}

	void emit(func_invoke_primary* x)override
	{
		if (is_special_func(x->func_name))
		{
			emit_special_func(x);
			return;
		}
		_out << x->func_name;
		_out << "(";
		for (auto& a : x->args)
			a->emit(this);
		_out << ")";
	}

	void emit(primary_term* x)override
	{
		x->p->emit(this);
	}

	void emit(expr_in_paren* x)override
	{
		_out << "(";
		x->x->emit(this);
		_out << ")";
	}

	void emit(mul_term* x)override
	{
		x->left->emit(this);
		_out << " * ";
		x->right->emit(this);
	}

	void emit(div_term* x)override
	{
		x->left->emit(this);
		_out << " / ";
		x->right->emit(this);
	}

	void emit(add_expr* x)override
	{
		x->left->emit(this);
		_out << " + ";
		x->right->emit(this);
	}

	void emit(sub_expr* x)override
	{
		x->left->emit(this);
		_out << " - ";
		x->right->emit(this);
	}

	void emit(true_bexpr* x)override
	{
		_out << "true";
	}

	void emit(false_bexpr* x)override
	{
		_out << "false";
	}

	void emit(binary_bexpr* x)override
	{
		x->left->emit(this);
		switch (x->op)
		{
		case bool_op::and:
			_out << " && ";
			break;
		case bool_op::or:
			_out << " || ";
			break;
		case bool_op::equal:
			_out << " == ";
			break;
		case bool_op::less:
			_out << " != ";
			break;
		case bool_op::less_equal:
			_out << " <= ";
			break;
		case bool_op::greater:
			_out << " > ";
			break;
		case bool_op::greater_equal:
			_out << " >= ";
			break;
		case bool_op::not_equal:
			_out << " != ";
			break;
		}
		x->right->emit(this);
	}

	void emit(not_bexpr* x)override
	{
		_out << "!";
		x->xpr->emit(this);
	}

	void emit(multi_stmt* x)override
	{
		if (x->f == nullptr && x->s == nullptr) return;
		if (x->f != nullptr);
		{ x->f->emit(this);  }
		_out << ";" << endl;
		if (x->s != nullptr)
		{
			x->s->emit(this);
		}
	}

	void emit(block_stmt* x)override
	{
		_out << "{" << endl;
		x->s->emit(this);
		_out << "}" << endl;
	}

	void emit(assign_stmt* x)override
	{
		if (x->op == assign_op::pre_incr)
		{
			_out << "++";
			x->name.emit(this);
			return;
		}
		else if (x->op == assign_op::pre_deincr)
		{
			_out << "--";
			x->name.emit(this);
			return;
		}

		x->name.emit(this);
		switch (x->op)
		{
		case assign_op::equal:
			_out << " = ";
			break;
		case assign_op::plus_equal:
			_out << " += ";
			break;
		case assign_op::minus_equal:
			_out << " -= ";
			break;
		case assign_op::mul_equal:
			_out << " *= ";
			break;
		case assign_op::div_equal:
			_out << " /= ";
			break;

		case assign_op::post_incr:
			_out << "++";
			return;
		case assign_op::post_deincr:
			_out << "--";
			return;
		}
		if (x->xpr != nullptr)
			x->xpr->emit(this);
	}

	void emit(decl_stmt* x)override
	{
		x->typ->emit(this);
		_out << " " << x->name;
		if (x->init_xpr != nullptr)
		{
			_out << " = ";
			x->init_xpr->emit(this);
		}
	}

	void emit(if_stmt* x)override
	{
		_out << "if(";
		x->xpr->emit(this);
		_out << ")";
		x->body->emit(this);
		if (x->else_body != nullptr)
		{
			_out << endl << "else ";
			x->else_body->emit(this);
		}
	}

	void emit(while_stmt* x)override
	{
		_out << "while(";
		x->xpr->emit(this);
		_out << ")";
		x->body->emit(this);
	}

	void emit(for_stmt* x)override
	{
		_out << "for(";
		x->init_stmt->emit(this);
		_out << "; ";
		x->cond->emit(this);
		_out << "; ";
		x->incr_stmt->emit(this);
		_out << ")";
		x->body->emit(this);
	}

	void emit(func_invoke_stmt* x)override
	{
		if (is_special_func(x->func_name))
		{
			emit_special_func(x);
			return;
		}
		_out << x->func_name;
		_out << "(";
		for (auto& a : x->args)
			a->emit(this);
		_out << ")";
	}

	void emit(return_stmt* x)override
	{
		if (x->rval != nullptr)
		{
			_out << "return ";
			x->rval->emit(this);
		}
		else
		{
			_out << "return";
		}
	}

	void emit(sfunction* x)override
	{
		if (x->name == "main")
		{
			_out << "__shader_output_t"; //x->return_type->emit(this);
			_out << " ";
			_out << x->name;
			_out << "(__shader_input_t input";
			for (auto& ag : x->args)
			{
				_out << ",";
				ag.typ->emit(this);
				_out << " " << ag.nmn;
			}
			_out << ")";
			_out << "{" << endl;
			_out << "__shader_output_t output;" << endl;
			x->body->emit(this);
			_out << "return output;" << endl;
			_out << "}";
		}
		else
		{
			x->return_type->emit(this);
			_out << " ";
			_out << x->name;
			_out << "(";
			for (auto& ag : x->args)
			{
				_out << ",";
				ag.typ->emit(this);
				_out << " " << ag.nmn;
			}
			_out << ")";
			_out << "{" << endl;
			x->body->emit(this);
			_out << "}";
		}
	}
};