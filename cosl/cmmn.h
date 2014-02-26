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
