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
#include "common.h"

#include <ctime>

#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

using namespace soci;
using namespace soci::details;
using namespace soci::details::msql;

void msql_standard_into_type_backend::define_by_pos(
    int &position, void *data, exchange_type type)
{
	data_		= data;
	type_		= type;
	position_	= position++;
}

void msql_standard_into_type_backend::pre_fetch()
{
    // ...
}

void msql_standard_into_type_backend::post_fetch(
    bool gotData, bool calledFromFetch, indicator *ind)
{
	/* 3.2 doc states this is end-of-rowset condition */
	if (calledFromFetch == true && gotData == false) {
		return;
	}

    if (gotData) {
    	int pos = position_ - 1;

    	msqlDataSeek(statement_.msqlResult_,statement_.currentRow_);
    	m_row row = msqlFetchRow(statement_.msqlResult_);

    	/* !!! Check this: mSQL returns a NULL pointer for a NULL value in database !!! */
    	if ( row[pos] == NULL ) {
    		if ( ind == NULL ) {
    			throw soci_error("NULL field and NULL indicator");
    		}
    		*ind = i_null;
    		return;
    	} else {
    		if ( ind != NULL )
    			*ind = i_ok;
    	}

    	const char *buf = row[pos] != NULL ? row[pos] : "";

    	switch (type_) {
			case x_char: {
				char *dest = static_cast<char*>(data_);
				*dest = *buf;
			}
			break;
			case x_stdstring: {
				std::string *dest = static_cast<std::string *>(data_);
				dest->assign(buf);
			}
			break;
			case x_short: {
				short *dest = static_cast<short*>(data_);
				parse_num(buf, *dest);
			}
			break;
			case x_integer: {
				int *dest = static_cast<int*>(data_);
				parse_num(buf, *dest);
			}
			break;
			case x_long_long: {
				long long *dest = static_cast<long long *>(data_);
				parse_num(buf, *dest);
			}
			break;
			case x_unsigned_long_long: {
				unsigned long long *dest =
				static_cast<unsigned long long*>(data_);
				parse_num(buf, *dest);
			}
			break;
			case x_double: {
				double *dest = static_cast<double*>(data_);
				parse_num(buf, *dest);
			}
			break;
			case x_stdtm: {
				// attempt to parse the string and convert to std::tm
				std::tm *dest = static_cast<std::tm *>(data_);
				parse_std_tm(buf, *dest);
			}
			break;
			default:
			throw soci_error("Into element used with non-supported type.");
    	}
    }
}

void msql_standard_into_type_backend::clean_up()
{
    // ...
}
