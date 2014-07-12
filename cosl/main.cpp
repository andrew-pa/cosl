#include "cmmn.h"
#include "tokenizer.h"
#include "ast.h"
#include "parser.h"
#include "code_emitter.h"
#include "hlsl_code_emitter.h"
#include "glsl_code_emitter.h"
#include "preprocesser.h"
#include <set>

/*
Usage (to generate HLSL):	-hlsl <in_file> <out_file>
Usage (to generate & compile HLSL):
							-hlsl <in_file> <interm_file> -c <fxc_options including out file>
Usage (to generate GLSL):	-glsl 440 <in_file> <out_file>
*/

enum class shader_target_language
{
	HLSL, GLSL
};

set<string> defines;
vector<string> incl_paths;
set<string> incls;

int main(int argc, char* argv[])
{
	vector<string> args;
	for (int i = 1; i < argc; ++i) args.push_back(string(argv[i]));

	incl_paths.push_back("");

	shader_target_language target_lang;
	string in_file_path, out_file_path;
	string hlsl__fxc_options; bool hlsl__use_fxc = false;
	
	code_emitter* ce = nullptr;

	for (int i = 0; i < args.size(); ++i)
	{
		string arg = args[i];
		if(arg == "-hlsl")
		{
			i++;
			target_lang = shader_target_language::HLSL;
			in_file_path = args[i++];
			out_file_path = args[i++];
			if (i < args.size() && args[i] == "-c")
			{
				i++;
				hlsl__use_fxc = true;
				ostringstream oss;
				while (args[i] != ";")
				{
					oss << args[i++] << " ";
				}
				hlsl__fxc_options = oss.str();
			}
			defines.insert("HLSL");
			ce = new hlsl_code_emitter;
		}
		else if (arg == "-glsl")
		{
			i++;
			target_lang = shader_target_language::GLSL;
			ce = new glsl_code_emitter(args[i++]);
			in_file_path = args[i++];
			out_file_path = args[i++];
			defines.insert("GLSL");
		}

		if(arg == "-I")
		{
			i++;
			incl_paths.push_back(args[i++]);
		}
		if(arg == "-D")
		{
			i++;
			defines.insert(args[i++]);
		}
	}
	incl_paths.push_back(in_file_path.substr(0, in_file_path.find_last_of('\\')));


	try
	{
		//string in_file = read_file_and_preprocess(in_file_path);

		//tokenizer tkn(in_file);
		
		tokenizer tkn;
		preprocesser p(incl_paths, defines);

		p.preprocess_file(in_file_path, tkn); //automatically tokenizes each line
		
		auto shf = parse_shader(tkn);
		ce->emit(shf);
		ofstream ofs(out_file_path);
		ofs.write(ce->out_string().c_str(), ce->out_string().size());
		ofs.flush();
		ofs.close();

		if (target_lang == shader_target_language::HLSL && hlsl__use_fxc) //ONLY for HLSL
		{
			ostringstream oss;
			oss << "fxc ";
			oss << out_file_path << " " << hlsl__fxc_options;
			//cout << "cmdline: " << oss.str() << endl;
			return system(oss.str().c_str()); //super bad hack
		}
	}
#ifndef _DEBUG
	catch(const exception& ex)
	{
		cout << "error: " << ex.what() << endl;
		return -1;
	}
#else
	catch(int z){}
#endif
	return 0;
}



















































//////////////////////////////////// ref only!
//////////////////////////////////////tokenizer tkn("	vec4 p = vec4(input.pos, 1.0);"
//////////////////////////////////////	"output.pos = p*wvp;"
//////////////////////////////////////"output.posW = input.pos;"
//////////////////////////////////////"output.normW = input.norm * inw;"
//////////////////////////////////////"output.texc = input.tex; ");
//////////////////////////////////////auto x = parse_stmt(tkn);
////////////////////////////////////
//////////////////////////////////////tokenizer tkn2("{ float depth : depth32; }");
//////////////////////////////////////auto x2 = parse_decl_block(tkn2);
//////////////////////////////////////getchar();
////////////////////////////////////
//////////////////////////////////////tokenizer tkn("$vertex_shader //this should be considered white space\n"
//////////////////////////////////////"	input /*so should this be*/"
//////////////////////////////////////"{"
//////////////////////////////////////"	vec3 pos : position;"
//////////////////////////////////////"	vec3 norm : normal;"
//////////////////////////////////////"	vec3 tex : texcoord;"
//////////////////////////////////////"}"
//////////////////////////////////////
//////////////////////////////////////"	output"
//////////////////////////////////////"{"
//////////////////////////////////////"	vec3 pos : rs_position;"
//////////////////////////////////////"	vec3 posW;"
//////////////////////////////////////"	vec3 normW;"
//////////////////////////////////////"	vec3 texc;"
//////////////////////////////////////"}"
////////////////////////////////////
//////////////////////////////////////"	cbuffer(0)"
//////////////////////////////////////"{"
//////////////////////////////////////"		mat4 wvp;"
//////////////////////////////////////"		mat4 inw;"
//////////////////////////////////////"		vec4 t;"
//////////////////////////////////////"}"
////////////////////////////////////
//////////////////////////////////////"void main()"
//////////////////////////////////////"{"
//////////////////////////////////////"	vec4 p = vec4(input.pos, 1.0);"
//////////////////////////////////////"	output.pos = p*wvp;"
//////////////////////////////////////"	output.posW = input.pos;"
//////////////////////////////////////"	output.normW = input.norm * inw;"
//////////////////////////////////////"	output.texc = input.tex;"
//////////////////////////////////////"}");
////////////////////////////////////tokenizer tkn("$pixel_shader "
////////////////////////////////////	"input"
////////////////////////////////////	"{"
////////////////////////////////////	"	vec3 posW;"
////////////////////////////////////	"	vec3 normW;"
////////////////////////////////////	"	vec2 texc;"
////////////////////////////////////	"}"
////////////////////////////////////
////////////////////////////////////	"output"
////////////////////////////////////	"{"
////////////////////////////////////	"	vec4 col : target0;"
////////////////////////////////////	"}"
////////////////////////////////////
////////////////////////////////////	"texture2D tex(0)"
////////////////////////////////////
////////////////////////////////////	"void main()"
////////////////////////////////////	"{"
////////////////////////////////////	"	output.col = sample2D(tex, input.texc);"
////////////////////////////////////	"}");
////////////////////////////////////auto sh = parse_shader(tkn);
////////////////////////////////////auto ce = new hlsl_code_emmiter;
////////////////////////////////////ce->emit(sh);
/////////////////////////////////////*ce->emit(sh->type);
////////////////////////////////////sh->inpblk->emit(ce);
////////////////////////////////////sh->outblk->emit(ce);
////////////////////////////////////for (auto& cb : sh->cbuffers)
////////////////////////////////////cb->emit(ce);
////////////////////////////////////for (auto& tx : sh->texturedefs)
////////////////////////////////////tx->emit(ce);
////////////////////////////////////for (auto& fn : sh->functions)
////////////////////////////////////fn->emit(ce);*/
////////////////////////////////////cout << ce->out_string();
////////////////////////////////////getchar();