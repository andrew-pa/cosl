#ifdef scratch
enum class special_func_type
{
	none, vector, sample, math, special_op
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
	if (n == "sample_texture") return true;
	if (n == "fract") return true;
	if (n == "mix") return true;
	if (n == "discard") return true;
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
	if (n == "sample_texture") return true;
	t = special_func_type::math;
	if (n == "fract") return true;
	if (n == "mix") return true;
	t = special_func_type::special_op;
	if (n == "discard")
		return true;
	t = special_func_type::none;
	return false;
}

void emit_special_func(func_invoke_primary* x)
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

		_out << "(";
		if (x->args.size() == 1)
		{
			x->args[0]->emit(this);
		}
		else
		{
			int i = 0;
			for (auto& a : x->args)
			{
				a->emit(this);
				if (i != x->args.size() - 1)
					_out << ", ";
				i++;
			}
		}
		_out << ")";
	}
	else if (t == special_func_type::sample)
	{
		x->args[0]->emit(this);
		_out << ".Sample(___smp_" << dynamic_cast<id_primary*>(x->args[0])->id_s;
		for (int i = 1; i < x->args.size(); ++i)
		{
			_out << ", ";
			x->args[i]->emit(this);
		}
		_out << ")";
	}
	else if (t == special_func_type::math)
	{
		if (x->func_name == "fract")
		{
			_out << "frac(";
			if (x->args.size() == 1)
			{
				x->args[0]->emit(this);
			}
			else
			{
				int i = 0;
				for (auto& a : x->args)
				{
					a->emit(this);
					if (i != x->args.size() - 1)
						_out << ", ";
					i++;
				}
			}
			_out << ")";
		}
		else if (x->func_name == "mix")
		{
			_out << "lerp(";
			if (x->args.size() == 1)
			{
				x->args[0]->emit(this);
			}
			else
			{
				int i = 0;
				for (auto& a : x->args)
				{
					a->emit(this);
					if (i != x->args.size() - 1)
						_out << ", ";
					i++;
				}
			}
			_out << ")";
		}
	}
	else if (t == special_func_type::special_op)
	{
		if (x->func_name == "discard")
		{
			_out << "clip(-1.f)";
		}
	}
}
#endif