//
// Copyright (C) 2004-2006 Maciej Sobczak, Stephen Hutton
// mSQL Backend Copyright (C) 2014 Wireless Energy Management Systems International Ltd
// Dev: Guy Thouret
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SOCI_MSQL_H_INCLUDED
#define SOCI_MSQL_H_INCLUDED

#ifdef _WIN32
# ifdef SOCI_DLL
#  ifdef SOCI_MSQL_SOURCE
#   define SOCI_MSQL_DECL __declspec(dllexport)
#  else
#   define SOCI_MSQL_DECL __declspec(dllimport)
#  endif // SOCI_MSQL_SOURCE
# endif // SOCI_DLL
#endif // _WIN32
//
// If SOCI_MSQL_DECL isn't defined yet define it now
#ifndef SOCI_MSQL_DECL
# define SOCI_MSQL_DECL
#endif

#include "soci-backend.h"
#ifdef _WIN32
#include <winsock.h> // SOCKET
#endif // _WIN32
#include <msql.h> // mSQL C API
#include <cstddef>
#include <string>

namespace soci
{

struct msql_statement_backend;

struct SOCI_MSQL_DECL msql_standard_into_type_backend : details::standard_into_type_backend
{
    msql_standard_into_type_backend(msql_statement_backend &st)
        : statement_(st)
    {}

    void define_by_pos(int& position, void* data, details::exchange_type type);

    void pre_fetch();
    void post_fetch(bool gotData, bool calledFromFetch, indicator* ind);

    void clean_up();

    msql_statement_backend& statement_;

	void *data_;
	details::exchange_type type_;
	int position_;
};

struct SOCI_MSQL_DECL msql_vector_into_type_backend : details::vector_into_type_backend
{
    msql_vector_into_type_backend(msql_statement_backend &st)
        : statement_(st)
    {}

    void define_by_pos(int& position, void* data, details::exchange_type type);

    void pre_fetch();
    void post_fetch(bool gotData, indicator* ind);

    void resize(std::size_t sz);
    std::size_t size();

    void clean_up();

    msql_statement_backend& statement_;
};

struct SOCI_MSQL_DECL msql_standard_use_type_backend : details::standard_use_type_backend
{
    msql_standard_use_type_backend(msql_statement_backend &st)
        : statement_(st)
    {}

    void bind_by_pos(int& position, void* data, details::exchange_type type, bool readOnly);
    void bind_by_name(std::string const& name, void* data, details::exchange_type type, bool readOnly);

    void pre_use(indicator const* ind);
    void post_use(bool gotData, indicator* ind);

    void clean_up();

    msql_statement_backend& statement_;

    void *data_;
    details::exchange_type type_;
    int position_;
    std::string name_;
    char *buf_;
};

struct SOCI_MSQL_DECL msql_vector_use_type_backend : details::vector_use_type_backend
{
    msql_vector_use_type_backend(msql_statement_backend &st)
        : statement_(st) {}

    void bind_by_pos(int& position, void* data, details::exchange_type type);
    void bind_by_name(std::string const& name, void* data, details::exchange_type type);

    void pre_use(indicator const* ind);

    std::size_t size();

    void clean_up();

    msql_statement_backend& statement_;
};

struct msql_session_backend;
struct SOCI_MSQL_DECL msql_statement_backend : details::statement_backend
{
    msql_statement_backend(msql_session_backend &session);

    void alloc();
    void clean_up();
    void prepare(std::string const& query, details::statement_type eType);

    exec_fetch_result execute(int number);
    exec_fetch_result fetch(int number);

    long long get_affected_rows();
    int get_number_of_rows();

    std::string rewrite_for_procedure_call(std::string const& query);

    int prepare_for_describe();
    void describe_column(int colNum, data_type& dtype, std::string& columnName);

    msql_standard_into_type_backend* make_into_type_backend();
    msql_standard_use_type_backend* make_use_type_backend();
    msql_vector_into_type_backend* make_vector_into_type_backend();
    msql_vector_use_type_backend* make_vector_use_type_backend();

    msql_session_backend& session_;

    /* mSQL specific members */
    m_result* msqlResult_;
    int numberOfRows_;
    int currentRow_;
    std::string rawQuery_;
};

struct msql_rowid_backend : details::rowid_backend
{
    msql_rowid_backend(msql_session_backend &session);

    ~msql_rowid_backend();
};

struct msql_blob_backend : details::blob_backend
{
    msql_blob_backend(msql_session_backend& session);

    ~msql_blob_backend();

    std::size_t get_len();
    std::size_t read(std::size_t offset, char* buf, std::size_t toRead);
    std::size_t write(std::size_t offset, char const* buf, std::size_t toWrite);
    std::size_t append(char const* buf, std::size_t toWrite);
    void trim(std::size_t newLen);

    msql_session_backend& session_;
};

struct msql_session_backend : details::session_backend
{
    msql_session_backend(connection_parameters const& parameters);

    ~msql_session_backend();

    void begin();
    void commit();
    void rollback();

    std::string get_backend_name() const { return "msql"; }

    void clean_up();

    msql_statement_backend* make_statement_backend();
    msql_rowid_backend* make_rowid_backend();
    msql_blob_backend* make_blob_backend();

    int sock_;
};

struct SOCI_MSQL_DECL msql_backend_factory : backend_factory
{
    msql_backend_factory() {}
    msql_session_backend* make_session(connection_parameters const& parameters) const;
};

extern SOCI_MSQL_DECL msql_backend_factory const msql;

extern "C"
{

// for dynamic backend loading
SOCI_MSQL_DECL backend_factory const* factory_msql();
SOCI_MSQL_DECL void register_factory_msql();

} // extern "C"

} // namespace soci

#endif // SOCI_MSQL_H_INCLUDED
