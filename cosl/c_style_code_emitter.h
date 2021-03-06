#pragma once
#include "cmmn.h"
#include "tokenizer.h"
#include "ast.h"
#include "parser.h"
#include "code_emitter.h"


class c_style_code_emitter : public code_emitter
{
protected:
	ostringstream _out;

	virtual bool emit_special_function(const string& name, const vector<expr*>& args) = 0;

	virtual void emit_regular_function(const string& name, const vector<expr*>& args)
	{
		_out << name << "(";
		if (args.size() == 1)
		{
			args[0]->emit(this);
		}
		else
		{
			int i = 0;
			for (auto& a : args)
			{
				a->emit(this);
				if (i != args.size() - 1)
					_out << ", ";
				i++;
			}
		}
		_out << ")";
	}
public:
	string out_string() const override { return _out.str(); }

	void reset() override
	{
		_out = ostringstream();
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

	void emit(struct_block* x)override
	{
		_out << "struct " << x->name << endl;
		x->db->emit(this);
		_out << ";" << endl;
	}

	void emit(num_primary* x)override
	{
		_out << x->num;
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

	void emit(array_index_primary* x) override
	{
		x->base->emit(this);
		for (auto* i : x->indices)
		{
			_out << "[";
			i->emit(this);
			_out << "]";
		}
	}

	void emit(negation_primary* x) override 
	{
		_out << "-(";
		x->exp->emit(this);
		_out << ")";
	}

	void emit(func_invoke_primary* x) override
	{
		if (!emit_special_function(x->func_name, x->args))
			emit_regular_function(x->func_name, x->args);
	}

	void emit(func_invoke_stmt* x) override
	{
		if (!emit_special_function(x->func_name, x->args))
			emit_regular_function(x->func_name, x->args);
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
			_out << " < ";
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
		if (x->f != nullptr)
		{ 
			x->f->emit(this); 

			if(!(is_a<if_stmt, stmt>(x->f) || is_a<while_stmt, stmt>(x->f) || is_a<for_stmt, stmt>(x->f)))
				_out << ";" << endl; 
		}
		
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
			x->name->emit(this);
			return;
		}
		else if (x->op == assign_op::pre_deincr)
		{
			_out << "--";
			x->name->emit(this);
			return;
		}

		x->name->emit(this);
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
		if (x->typ->array_dims.size() > 0)
		{
			for (uint d : x->typ->array_dims)
			{
				_out << "[";
				_out << d;
				_out << "]";
			}
		}
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
		if (dynamic_cast<block_stmt*>(x->body) == nullptr) _out << ";";
		if (x->else_body != nullptr)
		{
			_out << endl << "else ";
			x->else_body->emit(this);
			if (dynamic_cast<block_stmt*>(x->else_body) == nullptr) _out << ";" << endl;
		}
		else _out << endl;
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
};