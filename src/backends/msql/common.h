//
// Copyright (C) 2004-2006 Maciej Sobczak, Stephen Hutton
// mSQL Backend Copyright (C) 2014 Wireless Energy Management Systems International Ltd
// Dev: Guy Thouret
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SOCI_MSQL_COMMON_H_INCLUDED
#define SOCI_MSQL_COMMON_H_INCLUDED

#include "soci-msql.h"
#include <ctime>
#include <sstream>
#include <vector>

namespace soci {
namespace details {
namespace msql {

// helper function for parsing datetime values
void parse_std_tm(char const *buf, std::tm &t);

// The idea is that infinity - infinity gives NaN, and NaN != NaN is true.
//
// This should work on any IEEE-754-compliant implementation, which is
// another way of saying that it does not always work (in particular,
// according to stackoverflow, it won't work with gcc with the --fast-math
// option), but I know of no better way of testing this portably in C++ prior
// to C++11.  When soci moves to C++11 this should be replaced
// with std::isfinite().
template <typename T>
bool is_infinity_or_nan(T x) {
	T y = x - x;
	return (y != y);
}

template <typename T>
void parse_num(char const *buf, T &x) {
	std::istringstream iss(buf);
	iss >> x;
	if (iss.fail() || (iss.eof() == false)) {
		throw soci_error("Cannot convert data");
	}
	if (is_infinity_or_nan(x)) {
		throw soci_error("Cannot convert data");
	}
}

// helper for quoting strings
char * quote(const char *s, int len);

} // namespace msql
} // namespace details
} // namespace soci

#endif // SOCI_MSQL_COMMON_H_INCLUDED
