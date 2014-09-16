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
    : session_(session), msqlResult_(NULL), numberOfRows_(0), currentRow_(-1),
      hasIntoElements_(false), hasVectorIntoElements_(false),
      hasUseElements_(false), hasVectorUseElements_(false), executed_(false)
{
}

void msql_statement_backend::alloc()
{
	SOCI_DEBUG_FUNC
    /* Nothing to do here */
}

void msql_statement_backend::clean_up()
{
	SOCI_DEBUG_FUNC
	/* Clean Up mSQL Stored Result */
	if (msqlResult_ != NULL) {
		msqlFreeResult(msqlResult_);
		msqlResult_ = NULL;
	}
	/* Reset currentRow_, numberOfRows_ */
	numberOfRows_	= 0;
	currentRow_		= -1;
}

void msql_statement_backend::prepare(std::string const & query,
    statement_type /* eType */)
{
	SOCI_DEBUG_FUNC

	/* Borrowed some code from MySQL backend
	 * (If it ain't broke don't fix it!)
	 */
	queryChunks_.clear();
	enum { eNormal, eInQuotes, eInName } state = eNormal;

	std::string name;
	queryChunks_.push_back("");

	bool escaped = false;
	for (	std::string::const_iterator it = query.begin(),
			end = query.end();
			it != end;
			++it ) {
		switch (state) {

			case eNormal:
				if (*it == '\'') {
					queryChunks_.back() += *it;
					state = eInQuotes;
				} else if (*it == ':') {
					const std::string::const_iterator next_it = it + 1;
					// Check whether this is an assignment (e.g. @x:=y)
					// and treat it as a special case, not as a named binding.
					if (next_it != end && *next_it == '=') {
						queryChunks_.back() += ":=";
						++it;
					} else {
						state = eInName;
					}
				} else {
					queryChunks_.back() += *it;
				}
				break;

			case eInQuotes:
				if (*it == '\'' && !escaped) {
					queryChunks_.back() += *it;
					state = eNormal;
				} else {
					/* Regular Quoted Character */
					queryChunks_.back() += *it;
				}
				escaped = *it == '\\' && !escaped;
			break;

			case eInName:
				if (std::isalnum(*it) || *it == '_') {
					name += *it;
				} else {
					/* End of Name */
					names_.push_back(name);
					name.clear();
					queryChunks_.push_back("");
					queryChunks_.back() += *it;
					state = eNormal;
				}
			break;
		}
	}

	if (state == eInName) {
		names_.push_back(name);
	}
}

statement_backend::exec_fetch_result
msql_statement_backend::execute(int number)
{
	SOCI_DEBUG_FUNC
    /* NOTE: Number = number of fields to return to user? 0=nothing to return ?*/
	clean_up();

	std::string query;
	/* Are we using any binds ? */
    if (not useByPosBuffers_.empty() or not useByNameBuffers_.empty()) {

    	std::vector<char *> paramValues;

		if (not useByPosBuffers_.empty()) {
			// use elements bind by position
			// the map of use buffers can be traversed
			// in its natural order

			for (	UseByPosBuffersMap::iterator
					it = useByPosBuffers_.begin(),
					end = useByPosBuffers_.end();
					it != end; ++it ) {
				char **buffers = it->second;
				paramValues.push_back(buffers[0]);
			}
		} else {
			// use elements bind by name

			for (	std::vector<std::string>::iterator
					it = names_.begin(), end = names_.end();
					it != end; ++it ) {
				UseByNameBuffersMap::iterator b = useByNameBuffers_.find(*it);
				if (b == useByNameBuffers_.end()) {
					std::string msg("Missing use element for bind by name (");
					msg += *it;
					msg += ").";
					throw soci_error(msg);
				}
				char **buffers = b->second;
				paramValues.push_back(buffers[0]);
			}
		}

		/* Sanity Check for parameters and query chunks */
        if (queryChunks_.size() != paramValues.size()
            and queryChunks_.size() != paramValues.size() + 1) {
            throw soci_error("Wrong number of parameters.");
        }

        /* Reconstruct Query */
		std::vector<std::string>::const_iterator ci = queryChunks_.begin();
		for (	std::vector<char*>::const_iterator
				pi = paramValues.begin(), end = paramValues.end();
				pi != end; ++ci, ++pi )	{
			query += *ci;
			query += *pi;
		}
		if (ci != queryChunks_.end()) {
			query += *ci;
		}
    } else {
    	/* Nothing to bind, execute raw query */
    	query = queryChunks_.front();
    }

	int msqlQueryRet = msqlQuery(session_.sock_, const_cast<char*>( query.c_str() ));
	executed_ = true;

	if (msqlQueryRet < 0) {
		std::ostringstream ss;
		ss << "msql_statement_backend::execute: "
		   << msqlErrMsg;
		throw soci_error(ss.str());
	} else {
		if (msqlQueryRet == 0)
			return ef_no_data;
		else {
			SOCI_DEBUG("storing result returning fetch()\n")
			numberOfRows_	= msqlQueryRet;
			msqlResult_		= msqlStoreResult();
			// execute(0) not to call fetch
			if (number > 0)
				return fetch(number);
			else
				return ef_success;
		}
	}

    return ef_success;
}

statement_backend::exec_fetch_result
msql_statement_backend::fetch(int number)
{
	SOCI_DEBUG_FUNC
	if (++currentRow_ >= numberOfRows_) {
		/* End of Result Set */
		SOCI_DEBUG("currentRow_=%d,numberOfRows_=%d return ef_no_data\n",currentRow_,numberOfRows_)
		return ef_no_data;
	} else {
		SOCI_DEBUG("return ef_success\n")
		return ef_success;
	}
}

long long msql_statement_backend::get_affected_rows()
{
	SOCI_DEBUG_FUNC
	return static_cast<long long>(numberOfRows_);
}

int msql_statement_backend::get_number_of_rows()
{
	SOCI_DEBUG_FUNC
	/* Assuming fetch moved cursor forward so return number of rows left in result */
    int nRows = numberOfRows_ - currentRow_;
	SOCI_DEBUG("nRows = %d\n",nRows)
	return nRows;
}

std::string msql_statement_backend::rewrite_for_procedure_call(
    std::string const &query)
{
	SOCI_DEBUG_FUNC
	/* Not supported */
    return query;
}

int msql_statement_backend::prepare_for_describe()
{
	SOCI_DEBUG_FUNC
	/* Need to execute the query now to get the result set */
	if (!executed_)
		execute(1);

	SOCI_DEBUG("In prepare_for_describe after execute()...\n")
    /* Return number of columns in result set */
	if (msqlResult_ != NULL) {
		int nFields = msqlNumFields(msqlResult_);
		SOCI_DEBUG("nFields returned = %d\n",nFields)
		return nFields;
	} else {
		SOCI_DEBUG("NULL Result\n")
		return 0;
	}
}

void msql_statement_backend::describe_column(int colNum,
    data_type & type, std::string &columnName)
{
	SOCI_DEBUG_FUNC
    msqlFieldSeek(msqlResult_,colNum -1);

    m_field *field = msqlFetchField(msqlResult_);

    switch (field->type) {
    	case REAL_TYPE:
    	case MONEY_TYPE: {
    		type = dt_double;
    		SOCI_DEBUG("dt_double\n");
    		break;
    	}
    	case INT_TYPE:
    	case UINT_TYPE:
    	case INT16_TYPE:
    	case UINT16_TYPE:
    	case INT8_TYPE:
    	case UINT8_TYPE:
    	case BYTE_TYPE: {
    		type = dt_integer;
    		SOCI_DEBUG("dt_integer\n");
    	}
    	break;
    	case INT64_TYPE: {
    		type = dt_long_long;
    		SOCI_DEBUG("dt_long_long\n");
    	}
    	break;
    	case UINT64_TYPE: {
    		type = dt_unsigned_long_long;
    		SOCI_DEBUG("dt_unsigned_long_long\n");
    	}
    	break;
    	case CHAR_TYPE:
    	case TEXT_TYPE: {
    		type = dt_string;
    		SOCI_DEBUG("dt_string\n");
    	}
    	break;
    	case MILLITIME_TYPE:
    	case DATETIME_TYPE:
    	case MILLIDATETIME_TYPE: {
    		type = dt_date;
    		SOCI_DEBUG("dt_date\n");
    	}
    	break;
    	case IPV4_TYPE:
    	case CIDR4_TYPE:
    	case CIDR6_TYPE:
    		throw new soci_error("Unsupported mSQL Network Types");
    		break;
    	default:
    		throw new soci_error("Unknown mSQL Type");
    }
    /* Set the Column Name */
    columnName = field->name;
}

msql_standard_into_type_backend * msql_statement_backend::make_into_type_backend()
{
	SOCI_DEBUG_FUNC
	hasIntoElements_ = true;
    return new msql_standard_into_type_backend(*this);
}

msql_standard_use_type_backend * msql_statement_backend::make_use_type_backend()
{
	SOCI_DEBUG_FUNC
	hasUseElements_ = true;
    return new msql_standard_use_type_backend(*this);
}

msql_vector_into_type_backend *
msql_statement_backend::make_vector_into_type_backend()
{
	SOCI_DEBUG_FUNC
	hasVectorIntoElements_ = true;
    return new msql_vector_into_type_backend(*this);
}

msql_vector_use_type_backend * msql_statement_backend::make_vector_use_type_backend()
{
	SOCI_DEBUG_FUNC
	hasVectorUseElements_ = true;
    return new msql_vector_use_type_backend(*this);
}
