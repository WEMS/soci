//
// Copyright (C) 2004-2006 Maciej Sobczak, Stephen Hutton
// MSQL Backend Copyright (C) 2014 Wireless Energy Management Systems International Ltd
// Dev: Guy Thouret
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#define SOCI_MSQL_SOURCE
#include "soci-msql.h"
#include <backend-loader.h>

#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

using namespace soci;
using namespace soci::details;

// concrete factory for Empty concrete strategies
msql_session_backend* msql_backend_factory::make_session(
     connection_parameters const& parameters) const
{
     return new msql_session_backend(parameters);
}

msql_backend_factory const soci::msql;

extern "C"
{

// for dynamic backend loading
SOCI_MSQL_DECL backend_factory const* factory_msql()
{
    return &soci::msql;
}

SOCI_MSQL_DECL void register_factory_msql()
{
    soci::dynamic_backends::register_backend("msql", soci::msql);
}

} // extern "C"
