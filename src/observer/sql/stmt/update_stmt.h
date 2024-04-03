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
        Table* table, Value* values, int value_amount, FilterStmt* filter_stmt, const std::string& attribute_name);
    StmtType type() const override { return StmtType::UPDATE; }

  public:
    static RC create(Db* db, const UpdateSqlNode& update_sql, Stmt*& stmt);

  public:
    Table*      table() const { return table_; }
    Value*      values() const { return values_; }
    int         value_amount() const { return value_amount_; }
    FilterStmt* filter_stmt() const { return filter_stmt_; }
    std::string attribute_name() const { return attribute_name_; }

  private:
    /*
     * table_: 表对象指针
     * values_: 待更新的值
     * value_amount_: 待更新的值的数量
     * filter_stmt_: 过滤语句指针
     * attribute_name_: 待更新的属性名
     */
    Table*      table_        = nullptr;
    Value*      values_       = nullptr;
    int         value_amount_ = 0;
    FilterStmt* filter_stmt_  = nullptr;
    std::string attribute_name_;
};