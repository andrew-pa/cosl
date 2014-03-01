#pragma once
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <array>
#include <stack>
#include <map>
#include <iostream>
#include <fstream>
#include <functional>
#include <exception>
#include <algorithm>
using namespace std;

#define proprw(t, n, gc) inline t& n() gc
#define propr(t, n, gc) inline t n() const gc

typedef unsigned int uint;

template <typename A, typename T>
inline bool is_a(T* v)
{
	return (dynamic_cast<A*>(v) != nullptr);
}

template <typename T, typename F, typename _Compare = equal_to<T>>
inline const F& matchv(const T& v, vector<pair<T, function<const F&()>>> pmes, const F& def = F())
{
	_Compare c;
	for (auto pm : pmes)
	{
		if(c(v, pm.first))
		{
			return pm.second();
		}
	}
	return def;
}
template <typename T, typename F, typename _Compare = equal_to<T>>
inline const F& matchv(const T& v, vector<pair<T, function<const F&()>>> pmes, 
	function<const F&()> def = [&]{ return F(); })
{
	_Compare c;
	for (auto pm : pmes)
	{
		if (c(v, pm.first))
		{
			return pm.second();
		}
	}
	return def();
}