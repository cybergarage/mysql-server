/* Copyright (c) 2017, 2024, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is designed to work with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have either included with
   the program or referenced in the documentation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef SQL_CMD_DDL_INCLUDED
#define SQL_CMD_DDL_INCLUDED

#include "lex_string.h"
#include "my_sqlcommand.h"
#include "sql/sql_cmd.h"

class THD;

class Sql_cmd_ddl : public Sql_cmd {
 public:
  enum enum_sql_cmd_type sql_cmd_type() const override {
    /*
      Somewhat unsurprisingly, anything sub-classed to Sql_cmd_ddl
      identifies as DDL by default.
    */
    return SQL_CMD_DDL;
  }
};

/**
  This is a dummy class for old-style commands whose code is in sql_parse.cc,
  not in the execute() function. This Sql_cmd sub-class presently exists
  solely to provide a correct sql_cmd_type() for the command; it does nothing
  else.
*/
class Sql_cmd_ddl_dummy final : public Sql_cmd_ddl {
 private:
  enum_sql_command my_sql_command{SQLCOM_END};

 public:
  void set_sql_command_code(enum_sql_command scc) {
    assert(my_sql_command == SQLCOM_END);  // ensure value was not set up yet
    my_sql_command = scc;
  }

  enum_sql_command sql_command_code() const override {
    assert(my_sql_command != SQLCOM_END);  // ensure value was set up
    return my_sql_command;
  }

  // Error: we should never get here! (see explanation above)
  bool execute(THD *thd [[maybe_unused]]) override {
    assert(false);
    return false;
  }
};

class sp_name;

class Sql_cmd_create_library final : public Sql_cmd_ddl {
 public:
  Sql_cmd_create_library(THD *thd, bool if_not_exists, sp_name *lib_name,
                         LEX_STRING language, LEX_STRING source_code);

  enum_sql_command sql_command_code() const override {
    return SQLCOM_CREATE_LIBRARY;
  }

  bool execute(THD *thd) override;

 private:
  bool m_if_not_exists;
  sp_name *m_name;
  LEX_STRING m_language;
  // In order to support prepare of routines that contain CREATE LIBRARY
  // statements, we need to keep a copy of the source code.
  char *m_source;
};

class Sql_cmd_drop_library final : public Sql_cmd_ddl {
 public:
  Sql_cmd_drop_library(bool if_exists, sp_name *lib_name)
      : m_if_exists(if_exists), m_name(lib_name) {}

  enum_sql_command sql_command_code() const override {
    return SQLCOM_DROP_LIBRARY;
  }

  bool execute(THD *thd) override;

 private:
  bool m_if_exists;
  sp_name *m_name;
};

#endif  // SQL_CMD_DDL_INCLUDED
