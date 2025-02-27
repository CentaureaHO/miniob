/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2022/5/22.
//

#pragma once

#include "common/rc.h"
#include "sql/stmt/stmt.h"
#include <string>

class Table;
class FilterStmt;
class Db;
struct UpdateSqlNode;
class Value;

/**
 * @brief 更新语句
 * @ingroup Statement
 */
class UpdateStmt : public Stmt
{
  public:
    UpdateStmt() = default;
    UpdateStmt(
        Table* table, std::vector<Value> values, FilterStmt* filter_stmt, std::vector<std::string> attribute_names);

    StmtType type() const override { return StmtType::UPDATE; }

    static RC create(Db* db, const UpdateSqlNode& update_sql, Stmt*& stmt);

    Table*                          table() const { return table_; }
    const std::vector<Value>&       values() const { return values_; }
    FilterStmt*                     filter_stmt() const { return filter_stmt_; }
    const std::vector<std::string>& attribute_names() const { return attribute_names_; }

  private:
    Table*                   table_ = nullptr;
    std::vector<Value>       values_;
    FilterStmt*              filter_stmt_ = nullptr;
    std::vector<std::string> attribute_names_;
};