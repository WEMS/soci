###############################################################################
# CMake module to search for mSQL library
#
# On success, the macro sets the following variables:
# MSQL_FOUND = if the library found
# MSQL_LIBRARY = full path to the library
# MSQL_LIBRARIES = full path to the library
# MSQL_INCLUDE_DIR = where to find the library headers
#
# Copyright (c) 2009 Mateusz Loskot <mateusz@loskot.net>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
###############################################################################

find_path(MSQL_INCLUDE_DIR
  NAMES msql.h
  PATH_PREFIXES msql
  PATHS
  $ENV{BUILDROOT_DIR}/output/target/usr/local/msql3/include)

set(MSQL_NAMES msql)
find_library(MSQL_LIBRARY
  NAMES ${MSQL_NAMES}
  PATHS
  $ENV{BUILDROOT_DIR}/output/target/usr/local/msql3/lib)

set(MSQL_LIBRARIES 
  ${MSQL_LIBRARIES}
  ${MSQL_LIBRARY})

#message(STATUS ${MSQL_LIBRARY})
# Handle the QUIETLY and REQUIRED arguments and set MSQL_FOUND to TRUE
# if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MSQL
  DEFAULT_MSG
  MSQL_LIBRARIES
  MSQL_INCLUDE_DIR)

mark_as_advanced(MSQL_LIBRARY MSQL_INCLUDE_DIR MSQL_LIBRARIES)
