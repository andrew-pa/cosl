#pragma once
#include "cmmn.h"
#include "tokenizer.h"
#include "ast.h"
#include "parser.h"
#include "code_emitter.h"
#include "c_style_code_emitter.h"

class glsl_code_emitter : public c_style_code_emitter
{
	shader_type shmode;
	string vs_str;
	map<string, uint> txaliases;
	map<string, uint> ftxidims;
	string rspos;

	bool emit_special_function(const string& name, const vector<expr*>& args) override
	{
		if(name == "sample_texture")
		{
			if (args.size() != 2)
				throw new exception("too many or too few arguments to sample_texture");
			string txname = dynamic_cast<id_primary*>(args[0])->id_s;
			_out << "texture(";
			if (shmode == shader_type::vertex_shader) _out << "vs_";
			else if (shmode == shader_type::pixel_shader) _out << "ps_";
			_out << "tex_" << txaliases[txname] << ", ";
			args[1]->emit(this);
			//emit multiply that causes the y coord to flip to match DX/HLSL
			if (ftxidims[txname] == 2) _out << "*vec2(1., -1.)";
			else if (ftxidims[txname] == 3) _out << "*vec3(1.,-1.,1.)";
			_out << ")";
			return true;
		}
		if(name == "mul")
		{
			if(args.size() != 2)
			{
				throw exception("invalid number of arguments to mul");
			}
			_out << "(";
			args[1]->emit(this);
			_out << " * ";
			args[0]->emit(this);
			_out << ")";
			return true;
		}
		else if(name == "discard")
		{
			_out << " { discard; }";
			return true;
		}
		return false; //function wasn't really special
	}
public:
	glsl_code_emitter(const string& version_string) : vs_str(version_string) { }

	void emit(shader_file* sh) override
	{
		_out << "#version " << vs_str << " core" << endl;
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

	void emit(shader_type x) override
	{
		shmode = x;
	}

	void emit(decl& x) override
	{
		if(x.sem != nullptr)
		{
			if(x.sem->name == "rs_position" && shmode == shader_type::vertex_shader)
			{
				rspos = x.name;
				return;
			}
		}
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
		//ignore other semantics because GLSL doesn't have it. could be translated to layout idx
	}

	void emit(input_block* x) override
	{
		int layout_idx = 0;
		for(auto& d : x->db->decls)
		{
			if (shmode != shader_type::pixel_shader)
				_out << "layout(location = " << layout_idx << ") ";
			else
			{
				if (d.sem != nullptr)
				{
					if (d.sem->name == "rs_position" && shmode == shader_type::vertex_shader)
					{
						rspos = d.name;
						continue;
					}
				}
			}
			_out << "in ";
			d.emit(this);
			_out << ";" << endl;
			layout_idx++;
		}
	}

	void emit(output_block* x) override
	{
		for (auto& d : x->db->decls)
		{
			if(d.sem != nullptr)
			{
				if(d.sem->name == "target")
				{
					_out << "layout(location = " << d.sem->idx << ") ";
				}
				if (d.sem->name == "rs_position" && shmode == shader_type::vertex_shader)
				{
					rspos = d.name;
					continue;
				}
			}
			_out << "out ";
			d.emit(this);
			_out << ";" << endl;
		}
	}

	void emit(cbuffer_block* x) override
	{
		_out << "layout(std140) uniform ";
		if (shmode == shader_type::vertex_shader) _out << "vs_";
		else if (shmode == shader_type::pixel_shader) _out << "ps_";
		_out << "block_" << x->reg_idx << " ";
		x->dclbk->emit(this);
		_out << ";" << endl;
	}

	void emit(texture_def* x) override
	{
		switch (x->type)
		{
		case texture_type::texture1D:
			_out << "uniform sampler1D ";
			ftxidims[x->name] = 1;
			break;
		case texture_type::texture2D:
			_out << "uniform sampler2D ";
			ftxidims[x->name] = 2;
			break;
		case texture_type::texture3D:
			_out << "uniform sampler3D ";
			ftxidims[x->name] = 3;
			break;
		case texture_type::textureCube:
			_out << "uniform samplerCube ";
			ftxidims[x->name] = 7;
			break;
		}
		if (shmode == shader_type::vertex_shader) _out << "vs_";
		else if (shmode == shader_type::pixel_shader) _out << "ps_";
		_out << "tex_" << x->reg_idx << ";" << endl;
		txaliases[x->name] = x->reg_idx;
	}

	void emit(id_primary* x) override
	{
		if(txaliases.find(x->id_s) != txaliases.end())
		{
			if (shmode == shader_type::vertex_shader) _out << "vs_";
			else if (shmode == shader_type::pixel_shader) _out << "ps_";
			_out << "tex_" << txaliases[x->id_s];
			return;
		}
		//if(x.base_name == "output")
		//{
		//	if(!x.members.empty() && x.members[0] == rspos)
		//	{
		//		_out << "gl_Position";
		//		return;
		//	}
		//	
		//	int i = 0;
		//	for (auto& n : x.members)
		//	{
		//		_out << n;
		//		if (i != x.members.size() - 1)
		//			_out << ".";
		//		i++;
		//	}
		//	return;
		//}
		//if (x.base_name == "input")
		//{
		//	int i = 0;
		//	for (auto& n : x.members)
		//	{
		//		_out << n;
		//		if (i != x.members.size() - 1)
		//			_out << ".";
		//		i++;
		//	}
		//	return;
		//}

		_out << x->id_s;
	}

	void emit(member_access_primary* x) override
	{
		auto vaid = dynamic_cast<id_primary*>(x->val);
		if(vaid != nullptr)
		{
			if(vaid->id_s == "output")
			{
				if(!x->members.empty() && x->members[0] == rspos)
				{
					_out << "gl_Position";
					return;
				}
				
				int i = 0;
				for (auto& n : x->members)
				{
					_out << n;
					if (i != x->members.size() - 1)
						_out << ".";
					i++;
				}
				return;
			}
			if (vaid->id_s == "input")
			{
				int i = 0;
				for (auto& n : x->members)
				{
					_out << n;
					if (i != x->members.size() - 1)
						_out << ".";
					i++;
				}
				return;
			}
		}
		x->val->emit(this);
		for (auto& m : x->members)
			_out << "." << m;
		
	}
	
	void emit(sfunction* x) override
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

	void emit(s_type* x)override
	{
		switch (x->type)
		{
		case base_s_type::bvec_t:
			_out << "bvec";
			_out << x->vecdim;
			break;
		case base_s_type::bool_t:
			_out << "bool";
			break;
		case base_s_type::ivec_t:
			_out << "ivec";
			_out << x->vecdim;
			break;
		case base_s_type::int_t:
			_out << "int";
			break;
		case base_s_type::uvec_t:
			_out << "uvec";
			_out << x->vecdim;
			break;
		case base_s_type::uint_t:
			_out << "uint";
			break;
		case base_s_type::vec_t:
			_out << "vec";
			_out << x->vecdim;
			break;
		case base_s_type::float_t:
			_out << "float";
			break;
		case base_s_type::dvec_t:
			_out << "dvec";
			_out << x->vecdim;
			break;
		case base_s_type::double_t:
			_out << "double";
			break;
		case base_s_type::mat_t:
			_out << "mat";
			_out << x->vecdim;
			_out << "x";
			_out << x->matdim;
			break;
		case base_s_type::dmat_t:
			_out << "dmat";
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
};