#pragma once
#include "cmmn.h"
#include "tokenizer.h"

class preprocesser
{
	set<string> defines;
	vector<string> incl_paths;
	set<string> incls;

	bool intrp_line_w_ppc(string& line, ifstream& ifs, uint& line_idx, const string& file, tokenizer& tk)
	{
		istringstream lis(line);
		string cmd; lis >> cmd;

		if(cmd == "#ifdef")
		{
			string def;
			lis >> def;
			if(defines.find(def) != defines.end())
			{
				//take the if/true branch
				bool seen_else = false;
				while(line != "#endif")
				{
					getline(ifs, line); line_idx++;
					if (line == "#else" || line.find("#elif") != line.npos)
					{
						seen_else = true;
						continue;
					}
					if (line == "#endif") break;
					if(!seen_else) tk.tokenize_line(line, file, line_idx);
				}
			}
			else
			{
				//look for a else branch while skipping to #endif
				//take the if/true branch
				bool seen_else = false;
				while (line != "#endif")
				{
					getline(ifs, line); line_idx++;
					if (line == "#else")
					{
						seen_else = !seen_else;
						continue;
					}
					if(line.find("#elif") != line.npos)
					{
						lis = istringstream(line);
						string def;
						lis >> def >> def;
						if(defines.find(def) != defines.end())
						{
							seen_else = true;
							continue;
						}
						else seen_else = false;
					}
					if (line == "#endif") break;
					if (seen_else) tk.tokenize_line(line, file, line_idx);
				}
			}
			return true;
		}
		else if(cmd == "#define")
		{
			string def;
			lis >> def;
			defines.insert(def);
			return true;
		}
		else if(cmd == "#undef")
		{
			string d;
			lis >> d;
			defines.erase(d);
			return true;
		}
		else if(cmd == "#include")
		{
			string ip;
			lis >> ip;
			ip = ip.substr(1, ip.size() - 2); //remove < > || " "
			if(incls.find(ip) == incls.end())	
			{
				ifstream test_stream;
				for(const auto& incl_path : incl_paths)
				{
					string p = incl_path + "\\" + ip;
					test_stream.open(p);
					if(test_stream.good())
					{
						test_stream.close();
						preprocess_file(p, tk);
						break;
					}
				}
			}
			return true;
		}

		return false;
	}

public:
	inline void define(const string& d) { defines.insert(d); }
	inline void add_include_path(const string& p) { incl_paths.push_back(p); }

	preprocesser(vector<string> inp = vector<string>(), set<string> defs = set<string>()) : incl_paths(inp), defines(defs) {}

	void preprocess_file(const string& path, tokenizer& tk)
	{
		ifstream ifs(path);
	//	ifs.exceptions(ios_base::badbit | ios_base::failbit);

		string line;
		uint line_idx = 0;

		while(ifs)
		{
			getline(ifs, line);
			line_idx++;
			
			if (line.empty()) continue;

#pragma region find & skip comments
			auto slsh_pos = line.find('/');
			if(slsh_pos != line.npos)
			{
				if(line[slsh_pos+1] == '/') //comment that starts with '//'
				{
					if(slsh_pos != 0) //comment isn't the whole line
					{
						line = string(line.begin(), line.begin() + slsh_pos); //set line to be the line up to the comment
						tk.tokenize_line(line, path, line_idx);
					}
					continue;
				}
				else if(line[slsh_pos+1] == '*') //multiline / C-style comment
				{
					auto last_star = line.find_last_of('*'); //see if this multiline comment is really a comment embeded in a line
					if(last_star != line.npos && last_star != slsh_pos+1)
					{
						line.erase(line.begin() + (slsh_pos - 1), line.begin() + (last_star + 2));
						tk.tokenize_line(line, path, line_idx);
						continue;
					}
					if(slsh_pos != 0) //deal with multiline comments that don't start at the beginning
					{
						line = string(line.begin(), line.begin() + slsh_pos); //set line to be the line up to the comment
						tk.tokenize_line(line, path, line_idx);
					}
					while(true)
					{
						if (ifs.eof()) throw compile_exception("multiline comment never ends", path, line_idx);
						getline(ifs, line);
						line_idx++;
						auto ep = line.find('*'); //find ending */
						if(ep != line.npos && ep+1 < line.size() && line[ep+1] == '/')
						{
							line = string(line.begin() + ep + 2, line.end()); //set line to be the rest of the line
							if(line.size() > 0 && !line.empty()) //is there code on this line?
							{
								tk.tokenize_line(line, path, line_idx);
							}
							break;
						}
					}
					continue;
				}
			}
#pragma endregion

			
#pragma region interpret preprocesser commands
			if (line[0] == '#')
			{
				if (intrp_line_w_ppc(line, ifs, line_idx, path, tk))
					continue;
				//istringstream liness(line);
				//string cmd;
				//liness >> cmd;
				//if(cmd == "#ifdef")
				//{
				//	string req_def;
				//	liness >> req_def;
				//	if(defines.find(req_def) == defines.end())
				//	{
				//		while(ifs)
				//		{
				//			getline(ifs, line);
				//			line_idx++;
				//			if (line == "#endif") break;
				//			if (line == "#else") break;
				//		}	
				//	}
				//	 //process lines until #endif or #else
				//	{
				//		while(ifs)
				//		{
				//			getline(ifs, line);
				//			line_idx++;
				//			if (line == "#endif") break;
				//			else if (line == "#else")
				//			{
				//				while (ifs)
				//				{
				//					getline(ifs, line);
				//					line_idx++;
				//					if (line == "#endif") break;
				//				}
				//			}
				//			else if(line == "#elif")
				//		}
				//	}
				//	continue;
				//}
				//else if(cmd == "#endif") 
				//{
				//	//endif from a #else || #elif
				//	continue;
				//}
				//else if(cmd == "#define")
				//{
				//	string d;
				//	liness >> d;
				//	defines.insert(d);
				//	continue;
				//}
				//else if(cmd == "#undef")
				//{
				//	string d;
				//	liness >> d;
				//	defines.erase(d);
				//	continue;
				//}
				//else if(cmd == "#include")
				//{
				//	string ip;
				//	liness >> ip;
				//	ip = ip.substr(1, ip.size() - 2); //remove < > || " "
				//	if(incls.find(ip) == incls.end())	
				//	{
				//		ifstream test_stream;
				//		for(const auto& incl_path : incl_paths)
				//		{
				//			string p = incl_path + "\\" + ip;
				//			test_stream.open(p);
				//			if(test_stream.good())
				//			{
				//				test_stream.close();
				//				preprocess_file(p, tk);
				//				break;
				//			}
				//		}
				//	}
				//	continue;
				//}
			}
#pragma endregion
		
			//line must not be special, tokenize it as usual
			tk.tokenize_line(line, path, line_idx);
			if (ifs.eof() || ifs.bad() || ifs.fail()) break;
		}
	}
};