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
#include <soci-platform.h>

#include <cstdio>
#include <cstring>
#include <ctime>
#include <limits>

#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

using namespace soci;
using namespace soci::details;
using namespace soci::details::msql;


void msql_standard_use_type_backend::bind_by_pos(
    int &position, void *data, exchange_type type, bool /* readOnly */)
{
    data_ = data;
    type_ = type;
    position_ = position++;
}

void msql_standard_use_type_backend::bind_by_name(
    std::string const &name, void *data, exchange_type type, bool /* readOnly */)
{
    data_ = data;
    type_ = type;
    name_ = name;
}

void msql_standard_use_type_backend::pre_use(indicator const *ind)
{
    if (ind != NULL && *ind == i_null)
    {
        buf_ = new char[5];
        std::strcpy(buf_, "NULL");
    }
    else
    {
        // allocate and fill the buffer with text-formatted client data
        switch (type_)
        {
        case x_char:
            {
                char buf[] = { *static_cast<char*>(data_), '\0' };
                buf_ = quote(buf, 1);
            }
            break;
        case x_stdstring:
            {
                std::string *s = static_cast<std::string *>(data_);
                buf_ = quote(s->c_str(), s->size());
            }
            break;
        case x_short:
            {
                std::size_t const bufSize
                    = std::numeric_limits<short>::digits10 + 3;
                buf_ = new char[bufSize];
                snprintf(buf_, bufSize, "%d",
                    static_cast<int>(*static_cast<short*>(data_)));
            }
            break;
        case x_integer:
            {
                std::size_t const bufSize
                    = std::numeric_limits<int>::digits10 + 3;
                buf_ = new char[bufSize];
                snprintf(buf_, bufSize, "%d", *static_cast<int*>(data_));
            }
            break;
        case x_long_long:
            {
                std::size_t const bufSize
                    = std::numeric_limits<long long>::digits10 + 3;
                buf_ = new char[bufSize];
                snprintf(buf_, bufSize, "%" LL_FMT_FLAGS "d", *static_cast<long long *>(data_));
            }
            break;
        case x_unsigned_long_long:
            {
                std::size_t const bufSize
                    = std::numeric_limits<unsigned long long>::digits10 + 3;
                buf_ = new char[bufSize];
                snprintf(buf_, bufSize, "%" LL_FMT_FLAGS "u",
                         *static_cast<unsigned long long *>(data_));
            }
            break;

        case x_double:
            {
                if (is_infinity_or_nan(*static_cast<double*>(data_))) {
                    throw soci_error(
                        "Use element used with infinity or NaN, which are "
                        "not supported by the MySQL server.");
                }

                std::size_t const bufSize = 100;
                buf_ = new char[bufSize];

                snprintf(buf_, bufSize, "%.20g",
                    *static_cast<double*>(data_));
            }
            break;
        case x_stdtm:
            {
                std::size_t const bufSize = 24;
                buf_ = new char[bufSize];

            	/* mSQL Date / Time Format:
            	 * date		30-Jun-2014
            	 * time		10:06:47
            	 */

                std::tm *t = static_cast<std::tm *>(data_);
                snprintf(buf_, bufSize,
                    "\'%d-%02d-%02d %02d:%02d:%02d\'",
                    t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                    t->tm_hour, t->tm_min, t->tm_sec);
                strftime(buf_, (unsigned long int) bufSize, "\'%d-%b-%Y %T\'", t);
            }
            break;
        default:
            throw soci_error("Use element used with non-supported type.");
        }
    }
}

void msql_standard_use_type_backend::post_use(
    bool /* gotData */, indicator * /* ind */)
{
    clean_up();
}

void msql_standard_use_type_backend::clean_up()
{
    if (buf_ != NULL)
    {
        delete [] buf_;
        buf_ = NULL;
    }
}
