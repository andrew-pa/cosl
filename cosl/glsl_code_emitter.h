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
	shader_file* shfile;
	string vs_str;
	map<string, uint> txaliases;
	map<string, uint> ftxidims;
	map<semantic, string> vars_for_sem;

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
			//emit multiply that causes the y coord to flip to match DX/HLSL - turns out his is unnecessary
			//if (ftxidims[txname] == 2) _out << "*vec2(1., 1.)";
			//else if (ftxidims[txname] == 3) _out << "*vec3(1.,1.,1.)";
			//else if (ftxidims[txname] == 7 /* cubemap sets this to 7 */) _out << "*vec3(1.,1.,1.)";
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
		else if(shmode == shader_type::geometry_shader)
		{ 
			if(name == "emit_vertex")
			{
				_out << "EmitVertex()"; //EmitVertex has no params so emit_vertex just ignores it's params
				return true;
			}
			else if(name == "end_primitive")
			{
				_out << "EndPrimitive()"; //EndPrimitive same as EmitVertex
				return true;
			}
		}
		return false; //function wasn't really special
	}

	string string_for_prim_type(prim_type pt)
	{
		switch (pt)
		{
		case prim_type::point:
			return "points";
		case prim_type::line:
			return "lines";
		case prim_type::triangle:
			return "triangles";
		case prim_type::line_adj:
			return "lines_adjacency";
		case prim_type::triangle_adj:
			return "triangles_adjacency";
		default:
			throw exception("invalid primitive type");
		}
	}
	string na;
	string na_input = "i_";
	string na_output = "o_";
public:
	glsl_code_emitter(const string& version_string) : vs_str(version_string), na(string("")) { }

	void emit(shader_file* sh) override
	{
		shfile = sh;
		_out << "#version " << vs_str << " core" << endl;
		_out << "~~~~" << endl;
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
		switch (x)
		{
		case shader_type::vertex_shader:
			na_input = "vsi_";
			na_output = "vso_";
			break;
		case shader_type::pixel_shader:
			na_input = "i_";
			na_output = "pso_";
			break;
		case shader_type::geometry_shader:
			na_input = "vso_";
			na_output = "gso_";
			break;
		}
	}

	void emit(decl& x) override
	{
		x.type->emit(this);
		_out << " " << na+x.name;		
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
		if(shmode == shader_type::geometry_shader)
		{
			_out << "layout(" << string_for_prim_type(x->in_prim_type) << ") in;" << endl;
		}

		if (shmode == shader_type::pixel_shader)
		{
#pragma region Pixel Shader input blocks
			_out << "#ifndef IN_FROM_GS" << endl;
			
			int layout_idx = 0; //write out input vars for VS->PS path
			for (auto& d : x->db->decls)
			{
				if (d.sem != nullptr)
				{
					vars_for_sem[*d.sem] = d.name;
					if (d.sem->name == "rs_position")
						continue;
				}

				_out << "#define i_" << d.name << " " << "vso_" << d.name << endl;
				_out << "in ";
				d.type->emit(this);
				_out << " " << "vso_" + d.name;
				if (d.type->array_dims.size() > 0)
				{
					for (uint d : d.type->array_dims)
					{
						_out << "[";
						_out << d;
						_out << "]";
					}
				}
				_out << ";" << endl;
				layout_idx++;
			}

			_out << "#else" << endl;

			layout_idx = 0; //write out input vars for VS->GS->PS path
			for (auto& d : x->db->decls)
			{
				if (d.sem != nullptr)
				{
					vars_for_sem[*d.sem] = d.name;
					if (d.sem->name == "rs_position")
						continue;
				}

				_out << "#define i_" << d.name << " " << "gso_" << d.name << endl;
				_out << "in ";
				d.type->emit(this);
				_out << " " << "gso_" + d.name;
				if (d.type->array_dims.size() > 0)
				{
					for (uint d : d.type->array_dims)
					{
						_out << "[";
						_out << d;
						_out << "]";
					}
				}
				_out << ";" << endl;
				layout_idx++;
			}

			_out << "#endif" << endl;
			return;
#pragma endregion
		}

		na = na_input;

		int layout_idx = 0;
		for(auto& d : x->db->decls)
		{
			if (d.sem != nullptr)
			{
				vars_for_sem[*d.sem] = d.name;
				if(d.sem->name == "rs_position") 
					continue;
			}

			//this is iffy, because i don't know if geometry shaders and vertex shaders are the same for this
			if (shmode == shader_type::vertex_shader)
				_out << "layout(location = " << layout_idx << ") ";
			
			_out << "in ";
			d.emit(this);
			if (shmode == shader_type::geometry_shader) _out << "[]";
			_out << ";" << endl; 
			layout_idx++;
		}


		na = "";
	}

	void emit(output_block* x) override
	{
		if(shmode == shader_type::geometry_shader)
		{
			string pts;
			if (x->out_prim_type == prim_type::point) pts = "points";
			else if (x->out_prim_type == prim_type::line) pts = "line_strip";
			else if (x->out_prim_type == prim_type::triangle) pts = "triangle_strip";
			else throw exception("invalid output primitive type");
			_out << "layout(" << pts << 
				", max_vertices = " << shfile->geometry_shader_v.max_vertices << ") out;" << endl;
		}
		
		na = na_output;

		for (auto& d : x->db->decls)
		{
			if(d.sem != nullptr)
			{
				if(d.sem->name == "target")
				{
					_out << "layout(location = " << d.sem->idx << ") ";
				}
				else if(d.sem->name == "rs_position")
				{
					vars_for_sem[*d.sem] = d.name;
					continue;
				}
			}
			_out << "out ";
			d.emit(this);
			_out << ";" << endl;
		}

		na = "";
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
				if(!x->members.empty() && x->members[0] == vars_for_sem[semantic("rs_position", 0)])
				{
					_out << "gl_Position";
					return;
				}
			
				_out << na_output;
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
				_out << na_input;
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
		auto avaid = dynamic_cast<array_index_primary*>(x->val);
		if(avaid != nullptr)
		{
			vaid = dynamic_cast<id_primary*>(avaid->base);
			if(vaid != nullptr)
			{
				if(vaid->id_s == "inputs" && shmode == shader_type::geometry_shader)
				{
					if (x->members[0] == vars_for_sem[semantic("rs_position", 0)])
					{
						_out << "gl_in[";
						avaid->indices[0]->emit(this); //gl_in only 
						_out << "]";
						for (auto& m : x->members)
						{
							if (vars_for_sem[semantic("rs_position",0)] == m)
								_out << ".gl_Position";
							else
								_out << "." << m;
						}
					}
					else
					{
						_out << na_input;
						int i = 0;
						for (auto& n : x->members)
						{
							_out << n;
							if(i == 0)
							{

								_out << "[";
								avaid->indices[0]->emit(this);
								_out << "]";
							}
							if (i != x->members.size() - 1)
								_out << ".";
							i++;
						}
					}
					return;
				}
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