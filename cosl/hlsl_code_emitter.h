#pragma once
#include "cmmn.h"
#include "tokenizer.h"
#include "ast.h"
#include "parser.h"
#include "code_emitter.h"
#include "c_style_code_emitter.h"

class hlsl_code_emitter : public c_style_code_emitter
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

	bool is_vector_ctor(const string& name)
	{
		auto v = name.find("vec", 0);
		auto m = name.find("mat", 0);
		return v != name.npos ||
			m != name.npos; //hax!
	}

	tuple<base_s_type, uint, uint> get_vector_type(const string& name)
	{
		string base_type;
		bool seen_x = false;
		string vd, md;
		for (int i = 0; i < name.size(); ++i)
		{
			if (name[i] == 'x') seen_x = true;
			else if (!isdigit(name[i])) base_type += name[i];
			else
			{
				if (seen_x) md += name[i];
				else vd += name[i];
			}
		}
		if(md.empty())
		{
			uint vecd = atoi(vd.c_str());
			base_s_type bt = base_s_type::void_t;
			if (base_type == "bvec") bt = base_s_type::bvec_t;
			else if (base_type == "ivec") bt = base_s_type::ivec_t;
			else if (base_type == "uvec") bt = base_s_type::uvec_t;
			else if (base_type == "vec") bt = base_s_type::vec_t;
			else if (base_type == "dvec") bt = base_s_type::dvec_t;
			else if (base_type == "mat") bt = base_s_type::mat_t;
			else if (base_type == "dmat") bt = base_s_type::dmat_t;
			tuple<base_s_type, uint, uint> vt{bt, vecd, -1};
			return vt;
		}
		else
		{
			uint vecd = atoi(vd.c_str());
			uint matd = atoi(md.c_str());
			if (base_type == "mat") return tuple < base_s_type, uint, uint > {base_s_type::mat_t, vecd, matd};
			else if (base_type == "dmat") return tuple < base_s_type, uint, uint > {base_s_type::dmat_t, vecd, matd};
		}
	}

	string translate_vec_ctor(const tuple<base_s_type, uint, uint>& vt)
	{
		ostringstream oss;

		auto bt = get<0>(vt);
		auto vd = get<1>(vt);
		auto md = get<2>(vt);

		if (bt == base_s_type::bvec_t) oss << "bool" << vd;
		else if (bt == base_s_type::ivec_t) oss << "int" << vd;
		else if (bt == base_s_type::uvec_t) oss << "uint" << vd;
		else if (bt == base_s_type::vec_t) oss << "float" << vd;
		else if (bt == base_s_type::dvec_t) oss << "double" << vd;
		else if (bt == base_s_type::mat_t) oss << "float" << vd << "x" << md;
		else if (bt == base_s_type::dmat_t) oss << "double" << vd << "x" << md;

		return oss.str();
	}

	bool emit_special_function(const string& name, const vector<expr*>& args) override
	{
		if(is_vector_ctor(name))
		{
			auto vt = get_vector_type(name);
			auto ctor_func_name = translate_vec_ctor(vt);
			_out << ctor_func_name << "(";
			if(args.size() == 1) //replicate scalar value because HLSL is stupid
			{
				args[0]->emit(this);
				_out << ", ";
				args[0]->emit(this);
				_out << ", ";
				args[0]->emit(this);
			}
			else
			{
				for (int i = 0; i < args.size(); ++i)
				{
					args[i]->emit(this);
					if (i != args.size() - 1)
						_out << ", ";
				}
			}
			_out << ")";
			return true;
		}
		else if(name == "sample_texture")
		{
			if (args.size() != 2)
				throw new exception("too many or too few arguments to sample_texture");
			args[0]->emit(this);
			_out << ".Sample(___smp_" << dynamic_cast<id_primary*>(args[0])->id_s; //TODO: this prohibits passing a texture to another function
			_out << ", ";
			args[1]->emit(this);
			_out << ")";
			return true;
		}
		else if(name == "mix")
		{
			if (args.size() != 3) throw exception("too many or not enough arguments to mix()");
			_out << "lerp(";
			args[0]->emit(this);
			_out << ", ";
			args[1]->emit(this);
			_out << ", ";
			args[2]->emit(this);
			_out << ")";
			return true;
		}
		else if(name == "fract")
		{
			if (args.size() != 1) throw exception("too many or not enough arguments to fract()");
			_out << "frac(";
			args[0]->emit(this);
			_out << ")";
			return true;
		}
		else if(name == "discard")
		{
			_out << "clip(-1.f)";
			return true;
		}
		return false;
	}
public:
	hlsl_code_emitter()
	{
	}

	void emit(shader_file* sh)override
	{
		_out << "#pragma pack_matrix( row_major )" << endl;
		emit(sh->type);
		sh->inpblk->emit(this);
		sh->outblk->emit(this);
		for (auto& s : sh->structdefs)
			s->emit(this);
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
		if (x.type->array_dims.size() > 0)
		{
			for (uint d : x.type->array_dims)
			{
				_out << "[";
				_out << d;
				_out << "]";
			}
		}
		if (x.sem != nullptr)
			_out << " : " << translate_semantic(x.sem);
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

	void emit(id_primary* x)override
	{
		_out << x->id_s;
	}

	void emit(member_access_primary* x) override
	{
		x->val->emit(this);
		for (auto& m : x->members)
			_out << "." << m;
	}

	void emit(sfunction* x)override
	{
		if (x->name == "main")
		{
			_out << "__shader_output_t"; //x->return_type->emit(this);
			_out << " ";
			_out << x->name;
			_out << "(__shader_input_t input";
			bool first = true;
			for (auto& ag : x->args)
			{
				if(!first)
				{
					_out << ",";
					first = false;
				}
				ag.typ->emit(this);
				_out << " " << ag.nmn;
			}
			_out << ")";
			_out << "{" << endl;
			_out << "__shader_output_t output;" << endl;
			x->body->emit(this);
			_out << "return output;" << endl;
			_out << "}" << endl;
		}
		else
		{
			x->return_type->emit(this);
			_out << " ";
			_out << x->name;
			_out << "(";
			bool first = true;
			for (auto& ag : x->args)
			{
				if (!first)
				{
					_out << ",";
				}
				ag.typ->emit(this);
				_out << " " << ag.nmn;
				first = false;
			}
			_out << ")";
			_out << "{" << endl;
			x->body->emit(this);
			_out << "}" << endl;
		}
	}
};