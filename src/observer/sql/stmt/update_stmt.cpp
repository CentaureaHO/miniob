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
    // 检查输入参数的有效性：数据库指针不能为空，表名和属性名不能为空
    if (!db || update.relation_name.empty() || update.attribute_name.empty())
    {
        LOG_WARN("Invalid argument. db=%p, table_name=%s", db, update.relation_name.c_str());
        return RC::INVALID_ARGUMENT;
    }

    // 在数据库中查找对应的表
    Table* table = db->find_table(update.relation_name.c_str());
    if (!table)
    {
        LOG_WARN("No such table. db=%s, table_name=%s", db->name(), update.relation_name.c_str());
        return RC::SCHEMA_TABLE_NOT_EXIST;
    }

    // 创建一个映射，用于后续的过滤语句创建，其中包含表名到表对象的映射
    std::unordered_map<std::string, Table*> table_map;
    table_map[update.relation_name] = table;

    // 初始化过滤语句指针; 尝试根据给定的条件创建过滤语句
    FilterStmt* filter_stmt = nullptr;
    RC rc = FilterStmt::create(db, table, &table_map, update.conditions.data(), update.conditions.size(), filter_stmt);
    if (rc != RC::SUCCESS)
    {
        LOG_WARN("Failed to create filter statement. rc=%d:%s", rc, strrc(rc));
        return rc;
    }

    // 日期非法输入检查特判定
    if (update.value.attr_type() == DATES && !common::CheckDate(update.value.get_int()))
    {
        LOG_WARN("Invalid date input. Correct format: YYYY-MM-DD");
        return RC::INVALID_ARGUMENT;
    }

    // 获取表的元数据; 初始化字段元数据指针
    const TableMeta& table_meta = table->table_meta();
    const FieldMeta* field_meta = nullptr;
    // 遍历表的所有字段，查找对应的字段元数据
    for (int i = 0; i < table_meta.field_num(); ++i)
    {
        const FieldMeta* current = table_meta.field(i);
        if (current->name() == update.attribute_name)
        {
            field_meta = current;
            break;
        }
    }

    // 如果字段未找到，记录警告并返回字段缺失错误
    if (!field_meta)
    {
        LOG_WARN("Field '%s' not found in table '%s'.", update.attribute_name.c_str(), update.relation_name.c_str());
        return RC::SCHEMA_FIELD_MISSING;
    }

    // 如果字段类型与更新值的类型不匹配，记录警告并返回无效参数错误
    if (field_meta->type() != update.value.attr_type())
    {
        LOG_WARN("Type mismatch. Cannot update a %s field with a %s value.", 
                 attr_type_to_string(field_meta->type()), attr_type_to_string(update.value.attr_type()));
        return RC::INVALID_ARGUMENT;
    }

    stmt = new UpdateStmt(table, const_cast<Value*>(&update.value), 1, filter_stmt, update.attribute_name);
    return RC::SUCCESS;
}
