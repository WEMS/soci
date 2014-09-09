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

#include <sstream>

#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

using namespace soci;
using namespace soci::details;


msql_statement_backend::msql_statement_backend(msql_session_backend &session)
    : session_(session), msqlResult_(NULL), numberOfRows_(0), currentRow_(-1)
{
}

void msql_statement_backend::alloc()
{
	SOCI_MSQL_DEBUG_FUNC
    /* Nothing to do here */
}

void msql_statement_backend::clean_up()
{
	SOCI_MSQL_DEBUG_FUNC
	/* Clean Up mSQL Stored Result */
	if (msqlResult_ != NULL) {
		msqlFreeResult(msqlResult_);
		msqlResult_ = NULL;
	}
}

void msql_statement_backend::prepare(std::string const & query,
    statement_type /* eType */)
{
	SOCI_MSQL_DEBUG_FUNC
	/* Could borrow the MySQL code where the passed query string is
	 * scanned and parsed to support binds
	 *
	 * Keeping it simple for now by storing the raw query string
	 */
	rawQuery_ = query;
}

statement_backend::exec_fetch_result
msql_statement_backend::execute(int number)
{
	SOCI_MSQL_DEBUG_FUNC
    /* NOTE: Number = number of fields to return to user? 0=nothing to return ?*/
	clean_up();

	int msqlQueryRet = msqlQuery(session_.sock_, const_cast<char*>( rawQuery_.c_str() ));

	if (msqlQueryRet < 0) {
		std::ostringstream ss;
		ss << "msql_statement_backend::execute: "
		   << msqlErrMsg;
		throw soci_error(ss.str());
	} else {
		if (msqlQueryRet == 0)
			return ef_no_data;
		else {
			numberOfRows_	= msqlQueryRet;
			msqlResult_		= msqlStoreResult();
			return fetch(number);
		}
	}

    return ef_success;
}

statement_backend::exec_fetch_result
msql_statement_backend::fetch(int number)
{
	SOCI_MSQL_DEBUG_FUNC
	if (++currentRow_ >= numberOfRows_) {
		/* End of Result Set */
		return ef_no_data;
	} else {
		return ef_success;
	}
}

long long msql_statement_backend::get_affected_rows()
{
	SOCI_MSQL_DEBUG_FUNC
	return static_cast<long long>(numberOfRows_);
}

int msql_statement_backend::get_number_of_rows()
{
	SOCI_MSQL_DEBUG_FUNC
	/* Assuming fetch moved cursor forward so return number of rows left in result */
    return numberOfRows_ - currentRow_;
}

std::string msql_statement_backend::rewrite_for_procedure_call(
    std::string const &query)
{
	SOCI_MSQL_DEBUG_FUNC
	/* Not supported */
    return query;
}

int msql_statement_backend::prepare_for_describe()
{
	SOCI_MSQL_DEBUG_FUNC
    /* Return number of columns in result set */
	if (msqlResult_ != NULL)
		return msqlNumFields(msqlResult_);
	else
		return 0;
}

void msql_statement_backend::describe_column(int colNum,
    data_type & type, std::string & /* columnName */)
{
	SOCI_MSQL_DEBUG_FUNC
    msqlFieldSeek(msqlResult_,colNum -1);

    m_field *field = msqlFetchField(msqlResult_);

    switch (field->type) {
    	case REAL_TYPE:
    	case MONEY_TYPE:
    		type = dt_double;
    		break;
    	case INT_TYPE:
    	case UINT_TYPE:
    	case INT16_TYPE:
    	case UINT16_TYPE:
    	case INT8_TYPE:
    	case UINT8_TYPE:
    	case CHAR_TYPE:
    	case BYTE_TYPE:
    		type = dt_integer;
    		break;
    	case INT64_TYPE:
    		type = dt_long_long;
    		break;
    	case UINT64_TYPE:
    		type = dt_unsigned_long_long;
    		break;
    	case TEXT_TYPE:
    		type = dt_string;
    		break;
    	case MILLITIME_TYPE:
    	case DATETIME_TYPE:
    	case MILLIDATETIME_TYPE:
    		type = dt_date;
    		break;
    	case IPV4_TYPE:
    	case CIDR4_TYPE:
    	case CIDR6_TYPE:
    		throw new soci_error("Unsupported mSQL Network Types");
    		break;
    	default:
    		throw new soci_error("Unknown mSQL Type");
    }
}

msql_standard_into_type_backend * msql_statement_backend::make_into_type_backend()
{
	SOCI_MSQL_DEBUG_FUNC
    return new msql_standard_into_type_backend(*this);
}

msql_standard_use_type_backend * msql_statement_backend::make_use_type_backend()
{
	SOCI_MSQL_DEBUG_FUNC
    return new msql_standard_use_type_backend(*this);
}

msql_vector_into_type_backend *
msql_statement_backend::make_vector_into_type_backend()
{
	SOCI_MSQL_DEBUG_FUNC
    return new msql_vector_into_type_backend(*this);
}

msql_vector_use_type_backend * msql_statement_backend::make_vector_use_type_backend()
{
	SOCI_MSQL_DEBUG_FUNC
    return new msql_vector_use_type_backend(*this);
}
