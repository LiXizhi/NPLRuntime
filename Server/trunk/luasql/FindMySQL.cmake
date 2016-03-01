# - Find MySQL
# Find the MySQL includes and client library
# This module defines
#  MYSQL_INCLUDE_DIR, where to find mysql.h
#  MYSQL_LIBRARIES, the libraries needed to use MySQL.
#  MYSQL_FOUND, If false, do not try to use MySQL.
#  MYSQL_DLL, only set under win32 in case one needs to copy library
#
# Copyright (c) 2006, Jaroslaw Staniek, <js@iidea.pl>
# Copyright 2016, LiXizhi@yeah.net
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# LiXizhi: uncomment to find dir again
unset(MYSQL_INCLUDE_DIR CACHE)
unset(MYSQL_LIBRARIES CACHE)

if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
   set(MYSQL_FOUND TRUE)

else(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)

  # read environment variables and change \ to /
  SET(PROGRAM_FILES_32 $ENV{ProgramFiles})
  if (${PROGRAM_FILES_32})
    STRING(REPLACE "\\\\" "/" PROGRAM_FILES_32 ${PROGRAM_FILES_32})
  endif(${PROGRAM_FILES_32})

  SET(PROGRAM_FILES_64 $ENV{ProgramW6432})
  if (${PROGRAM_FILES_64})
     STRING(REPLACE "\\\\" "/" PROGRAM_FILES_64 ${PROGRAM_FILES_64})
  endif(${PROGRAM_FILES_64})

  find_path(MYSQL_INCLUDE_DIR mysql.h
      /usr/include/mysql
      /usr/local/include/mysql
      /usr/local/mysql/include
      /opt/local/include/mysql*/mysql
      $ENV{ProgramFiles}/MySQL/*/include
      $ENV{SystemDrive}/MySQL/*/include
      ${PROGRAM_FILES_32}/MySQL/*/include
      ${PROGRAM_FILES_64}/MySQL/*/include
      )

if(WIN32 AND MSVC)
  find_library(MYSQL_LIBRARIES NAMES libmysql
      PATHS
      $ENV{ProgramFiles}/MySQL/*/lib/opt
      $ENV{SystemDrive}/MySQL/*/lib/opt
      $ENV{SystemDrive}/MySQL/*/lib
      ${PROGRAM_FILES_32}/MySQL/*/lib
      ${PROGRAM_FILES_64}/MySQL/*/lib
      )
  GET_FILENAME_COMPONENT(MYSQL_LIBRARY_DIR ${MYSQL_LIBRARIES} PATH)
  FIND_FILE(MYSQL_DLL libmysql.dll PATHS ${MYSQL_LIBRARY_DIR})

else(WIN32 AND MSVC)
  find_library(MYSQL_LIBRARIES NAMES mysqlclient
      PATHS
      /usr/lib/mysql
      /usr/local/lib/mysql
      /usr/local/mysql/lib
      /opt/local/lib/mysql*/mysql
      )
endif(WIN32 AND MSVC)

  if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
    set(MYSQL_FOUND TRUE)
    message(STATUS "Found MySQL: ${MYSQL_INCLUDE_DIR}, ${MYSQL_LIBRARIES}")
  else(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
    set(MYSQL_FOUND FALSE)
    message(STATUS "MySQL not found.")
  endif(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)

  mark_as_advanced(MYSQL_INCLUDE_DIR MYSQL_LIBRARIES)

endif(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)