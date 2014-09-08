###############################################################################
#
# This file is part of CMake configuration for SOCI library
#
# Copyright (C) 2010 Mateusz Loskot
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#
###############################################################################
soci_backend(mSQL
  DEPENDS mSQL
  HEADERS soci-msql.h common.h
  DESCRIPTION "SOCI backend for mSQL database engine"
  AUTHORS "Guy Thouret"
  MAINTAINERS "Guy Thouret")

add_subdirectory(test)
