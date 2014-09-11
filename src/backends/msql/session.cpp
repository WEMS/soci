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
#include <connection-parameters.h>
#include <iostream>


#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

using namespace soci;
using namespace soci::details;
using std::string;

namespace {

void skip_white(std::string::const_iterator *i,
		std::string::const_iterator const & end, bool endok) {
	for (;;) {
		if (*i == end) {
			if (endok) {
				return;
			} else {
				throw soci_error("Unexpected end of connection string");
			}
		}
		if (std::isspace(**i)) {
			++*i;
		} else {
			return;
		}
	}
}

std::string param_name(std::string::const_iterator *i,
		std::string::const_iterator const & end) {
	std::string val("");
	for (;;) {
		if (*i == end or (not std::isalpha(**i) and **i != '_')) {
			break;
		}
		val += **i;
		++*i;
	}
	return val;
}

string param_value(string::const_iterator *i,
		string::const_iterator const & end) {

	string err = "Malformed connection string";
	bool quot;

	if (**i == '\'') {
		quot = true;
		++*i;
	} else {
		quot = false;
	}

	string val("");
	for (;;) {

		if (*i == end) {
			if (quot)
				throw soci_error(err);
			else
				break;
		}

		if (**i == '\'') {
			if (quot) {
				++*i;
				break;
			} else {
				throw soci_error(err);
			}
		}

		if (not quot and std::isspace(**i))
			break;

		if (**i == '\\') {
			++*i;
			if (*i == end)
				throw soci_error(err);
		}

		val += **i;
		++*i;
	}
	return val;
}

void parse_connect_string(const string &connect_string,
							string *dbname, bool *dbname_exists,
							string *host, bool *host_exists) {
	string err = "Malformed connection string";
	string::const_iterator i = connect_string.begin(), end = connect_string.end();

	while (i != end) {
		/* connect_string format is <white><name><white>=<white><value>... */
		skip_white(&i, end, true);
		if (i == end)
			return;
        string par = param_name(&i, end);
        skip_white(&i, end, false);
		if (*i == '=')
			++i;
		else
			throw soci_error(err);
		skip_white(&i, end, false);
		string val = param_value(&i, end);

		/* Set dbname and host parameters */
		if (par == "dbname") {
			*dbname = val;
			*dbname_exists = true;
		} else if (par == "host") {
			*host = val;
			*host_exists = true;
		} else {
			throw soci_error(err);
		}
	}
}

} //namespace anonymous


msql_session_backend::msql_session_backend(
    connection_parameters const & parameters)
{
	SOCI_DEBUG_FUNC
    /* Parse db and host parameters */
	std::string host,dbname;
	bool host_exists = false;
	bool dbname_exists = false;

	parse_connect_string(parameters.get_connect_string(), &dbname, &dbname_exists, &host, &host_exists);

	if ( dbname_exists ) {
		if ( host_exists ) {
			std::cout << "using host" << std::endl;
			sock_ = msqlConnect( const_cast<char*>( host.c_str() ) );
		} else {
			SOCI_DEBUG("Passing NULL host\n")
			/* Passing NULL as msqlConnect parameter does not work! */
			char *nullhost = NULL;
			sock_ = msqlConnect(nullhost);
		}

		if (sock_ == -1) {
			std::string errMsg = msqlErrMsg;
			throw soci_error(errMsg);
		} else {
			msqlSelectDB(sock_, const_cast<char*>( dbname.c_str() ));
		}
	} else {
		throw soci_error("dbname not set in connection");
	}
}

msql_session_backend::~msql_session_backend()
{
	SOCI_DEBUG_FUNC
    clean_up();
}

void msql_session_backend::begin()
{
	SOCI_DEBUG_FUNC
    /* Unsupported */
}

void msql_session_backend::commit()
{
	SOCI_DEBUG_FUNC
	/* Unsupported */
}

void msql_session_backend::rollback()
{
	/* Unsupported */
}

void msql_session_backend::clean_up()
{
	SOCI_DEBUG_FUNC
    sock_ = NULL;
}

msql_statement_backend * msql_session_backend::make_statement_backend()
{
	SOCI_DEBUG_FUNC
    return new msql_statement_backend(*this);
}

msql_rowid_backend * msql_session_backend::make_rowid_backend()
{
	SOCI_DEBUG_FUNC
    return new msql_rowid_backend(*this);
}

msql_blob_backend * msql_session_backend::make_blob_backend()
{
	SOCI_DEBUG_FUNC
    return new msql_blob_backend(*this);
}
