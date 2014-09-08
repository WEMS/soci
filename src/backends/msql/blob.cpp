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


msql_blob_backend::msql_blob_backend(msql_session_backend &session)
    : session_(session)
{
	throw soci_error("BLOBs are not supported");
}

msql_blob_backend::~msql_blob_backend()
{
	throw soci_error("BLOBs are not supported");
}

std::size_t msql_blob_backend::get_len()
{
	throw soci_error("BLOBs are not supported");
}

std::size_t msql_blob_backend::read(
    std::size_t /* offset */, char * /* buf */, std::size_t /* toRead */)
{
	throw soci_error("BLOBs are not supported");
}

std::size_t msql_blob_backend::write(
    std::size_t /* offset */, char const * /* buf */,
    std::size_t /* toWrite */)
{
	throw soci_error("BLOBs are not supported");
}

std::size_t msql_blob_backend::append(
    char const * /* buf */, std::size_t /* toWrite */)
{
	throw soci_error("BLOBs are not supported");
}

void msql_blob_backend::trim(std::size_t /* newLen */)
{
	throw soci_error("BLOBs are not supported");
}
