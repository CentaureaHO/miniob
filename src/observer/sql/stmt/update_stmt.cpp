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

#include "sql/stmt/update_stmt.h"
#include <string>
#include "common/log/log.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "common/lang/string.h"

UpdateStmt::UpdateStmt(
    Table* table, Value* values, int value_amount, FilterStmt* filter_stmt, const std::string& attribute_name)
    : table_(table),
      values_(values),
      value_amount_(value_amount),
      filter_stmt_(filter_stmt),
      attribute_name_(attribute_name)
{}

RC UpdateStmt::create(Db* db, const UpdateSqlNode& update, Stmt*& stmt)
{
    if (!db || update.relation_name.empty() || update.attribute_names[0].empty())
    {
        LOG_WARN("Invalid argument. db=%p, table_name=%s", db, update.relation_name.c_str());
        return RC::INVALID_ARGUMENT;
    }

    Table* table = db->find_table(update.relation_name.c_str());
    if (!table)
    {
        LOG_WARN("No such table. db=%s, table_name=%s", db->name(), update.relation_name.c_str());
        return RC::SCHEMA_TABLE_NOT_EXIST;
    }

    std::unordered_map<std::string, Table*> table_map;
    table_map[update.relation_name] = table;
    FilterStmt* filter_stmt         = nullptr;
    RC rc = FilterStmt::create(db, table, &table_map, update.conditions.data(), update.conditions.size(), filter_stmt);
    if (rc != RC::SUCCESS)
    {
        LOG_WARN("Failed to create filter statement. rc=%d:%s", rc, strrc(rc));
        return rc;
    }

    const TableMeta& table_meta = table->table_meta();
    const FieldMeta* field_meta = nullptr;
    for (int i = 0; i < table_meta.field_num(); ++i)
    {
        const FieldMeta* current = table_meta.field(i);
        if (current->name() == update.attribute_names[0])
        {
            field_meta = current;
            break;
        }
    }

    if (!field_meta)
    {
        LOG_WARN("Field '%s' not found in table '%s'.", update.attribute_names[0].c_str(), update.relation_name.c_str());
        return RC::SCHEMA_FIELD_MISSING;
    }

    if (field_meta->type() != update.values[0].attr_type())
    {
        LOG_WARN("Type mismatch. Cannot update a %s field with a %s value.", 
                 attr_type_to_string(field_meta->type()), attr_type_to_string(update.values[0].attr_type()));
        return RC::INVALID_ARGUMENT;
    }

    stmt = new UpdateStmt(table, const_cast<Value*>(&update.values[0]), 1, filter_stmt, update.attribute_names[0]);
    return RC::SUCCESS;
}
