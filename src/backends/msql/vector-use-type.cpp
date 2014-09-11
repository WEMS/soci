//
// Copyright (C) 2004-2006 Maciej Sobczak, Stephen Hutton
// mSQL Backend Copyright (C) 2014 Wireless Energy Management Systems International Ltd
// Dev: Guy Thouret
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#define SOCI_MSQL_SOURCE
#include "soci-msql.h"

#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

using namespace soci;
using namespace soci::details;


void msql_vector_use_type_backend::bind_by_pos(int & /* position */,
        void * /* data */, exchange_type /* type */)
{
	SOCI_DEBUG_FUNC
    // ...
}

void msql_vector_use_type_backend::bind_by_name(
    std::string const & /* name */, void * /* data */,
    exchange_type /* type */)
{
	SOCI_DEBUG_FUNC
    // ...
}

void msql_vector_use_type_backend::pre_use(indicator const * /* ind */)
{
	SOCI_DEBUG_FUNC
    // ...
}

std::size_t msql_vector_use_type_backend::size()
{
	SOCI_DEBUG_FUNC
    // ...
    return 1;
}

void msql_vector_use_type_backend::clean_up()
{
	SOCI_DEBUG_FUNC
    // ...
}
