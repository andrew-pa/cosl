#include "tokenizer.h"

bool is_whitespace(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}
bool is_delimiter(char c, bool plain)
{
	return is_whitespace(c) || (c == ' ' || c == ';' || c == ',' ||
		c == '(' || c == ')' || c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || (!plain ? (c == '"' || c == '\'') : false)
		|| c == '=' || c == '>' || c == '<' || c == '!' || c == '&' || c == '|' || c == '{' || c == '}' || c == '.' || c == '[' || c == ']');
}

bool is_two_char_special_first(char c)
{
	return c == '&' || c == '!' || c == '|' || c == '+' || c == '-' || c == '*' || c == '/'
			 || c == '<' || c == '>' || c == '=';
}
bool is_two_char_special_second(char c)
{
	return c == '&' || c == '=' || c == '|' || c == '+' || c == '-';
}

bool is_numeric_body(char c)
{
	return isdigit(c) || c == '.';
}



token tokenizer::next_token()
{
	if (idx == s.size()) return end_token();
	for (; idx < s.size(); ++idx)
	{
		if (is_whitespace(s[idx]))
		{
			if (s[idx] == '\n')
				current_line++;
			continue;
		}

		if (isdigit(s[idx]) ||
			(s[idx] == '.')
			&& (idx + 1 < s.size() && is_numeric_body(s[idx + 1])))
		{
			string d;
			d += s[idx];
			idx++;
			if (idx == s.size()) return num_token(d, current_line);

			while (is_numeric_body(s[idx]))
			{
				d += s[idx];
				idx++;
				if (idx == s.size()) return num_token(d, current_line);
			}
			return num_token(d, current_line);
		}
		if (is_delimiter(s[idx], true))
		{
			if(idx + 1 < s.size() && is_two_char_special_first(s[idx]) && is_two_char_special_second(s[idx+1]))
			{
				auto st = special_token(s[idx], s[idx + 1], current_line);
				idx += 2;
				return st;
			}
			return special_token(s[idx++], current_line);
		}
		if (s[idx] == '"' || s[idx] == '\'')
		{
			idx++; //consume opening "
			if (idx == s.size()) return end_token(current_line);
			string d;
			while (s[idx] != '"' && s[idx] != '\'')
			{
				d += s[idx];
				idx++;
				if (idx == s.size()) return end_token(current_line);
			}
			idx++;
			return string_token(d, current_line);
		}

		else
		{
			string d;
			bool hit_end = false;
			while (!is_delimiter(s[idx], false))
			{
				d += s[idx];
				idx++;
				if (idx >= s.size()) {hit_end = true; break;}
			}
			if (d.empty() && hit_end) return end_token(current_line);
			return id_token(d, current_line);
		}
	}
	return end_token(current_line);
}