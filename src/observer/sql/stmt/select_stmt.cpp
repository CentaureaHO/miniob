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
// Created by Wangyunlai on 2022/6/6.
//

#include "sql/stmt/select_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "common/log/log.h"
#include "common/lang/string.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "sql/operator/aggregation_func_operator.h"
#include <iostream>

SelectStmt::~SelectStmt()
{
    if (nullptr != filter_stmt_)
    {
        delete filter_stmt_;
        filter_stmt_ = nullptr;
    }
}

static void wildcard_fields(Table* table, std::vector<Field>& field_metas, AggregationType aggr_type)
{
    const TableMeta& table_meta = table->table_meta();
    const int        field_num  = table_meta.field_num();
    if (aggr_type == AggregationType::F_COUNT_ALL)
    {
        field_metas.push_back(Field(table, table_meta.field(0), aggr_type));
        return;
    }
    for (int i = table_meta.sys_field_num(); i < field_num; i++)
    {
        field_metas.push_back(Field(table, table_meta.field(i), aggr_type));
    }
}

RC SelectStmt::create(Db* db, const SelectSqlNode& select_sql, Stmt*& stmt)
{
    if (nullptr == db)
    {
        LOG_WARN("invalid argument. db is null");
        return RC::INVALID_ARGUMENT;
    }

    // collect tables in `from` statement
    std::vector<Table*>                     tables;
    std::unordered_map<std::string, Table*> table_map;
    for (size_t i = 0; i < select_sql.relations.size(); i++)
    {
        const char* table_name = select_sql.relations[i].c_str();
        // std::cout << "table_name: " << table_name << std::endl;
        if (nullptr == table_name)
        {
            LOG_WARN("invalid argument. relation name is null. index=%d", i);
            return RC::INVALID_ARGUMENT;
        }

        Table* table = db->find_table(table_name);
        if (nullptr == table)
        {
            LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
            return RC::SCHEMA_TABLE_NOT_EXIST;
        }

        tables.push_back(table);
        table_map.insert(std::pair<std::string, Table*>(table_name, table));
    }

    // collect query fields in `select` statement
    std::vector<Field> query_fields;
    bool               IsAgg = 0, IsCommon = 0;
    for (int i = static_cast<int>(select_sql.attributes.size()) - 1; i >= 0; i--)
    {
        const RelAttrSqlNode& relation_attr = select_sql.attributes[i];
        AggregationType aggr_type = relation_attr.aggr_type();
        if (!relation_attr.ValidAgg)
        {
            if (aggr_type == AggregationType::COMPOSITE)
            {
                LOG_WARN("Nested aggregation functions are not allowed.");
                return RC::NESTED_AGGREGATION;
            }
            else if (aggr_type == AggregationType::MULATTRS)
            {
                LOG_WARN("Multiple attributes are not allowed in aggregation functions.");
                return RC::AGGREGATION_UNMATCHED;
            }
            return RC::INVALID_ARGUMENT;
        }
        if (aggr_type != AggregationType::NOTAGG)
            IsAgg = 1;
        else
            IsCommon = 1;
        if (IsAgg && IsCommon)
        {
            LOG_WARN("Aggregate functions and attributes cannot be queried simultaneously.");
            return RC::AGGREGATION_UNMATCHED;
        }
        if (common::is_blank(relation_attr.relation_name.c_str()) &&
            0 == strcmp(relation_attr.attribute_name.c_str(), "*"))
        {
            if (aggr_type != AggregationType::F_COUNT_ALL && aggr_type != AggregationType::NOTAGG)
            {
                LOG_WARN("Invalid aggregation type for field \'*\'. aggr: %s", aggregation_type_to_string(aggr_type));
                return RC::INVALID_ARGUMENT;
            }
            for (Table* table : tables) { wildcard_fields(table, query_fields, aggr_type); }
        }
        else if (!common::is_blank(relation_attr.relation_name.c_str()))
        {
            const char* table_name = relation_attr.relation_name.c_str();
            const char* field_name = relation_attr.attribute_name.c_str();

            if (0 == strcmp(table_name, "*"))
            {
                if (0 != strcmp(field_name, "*"))
                {
                    LOG_WARN("invalid field name while table is *. attr=%s", field_name);
                    return RC::SCHEMA_FIELD_MISSING;
                }
                if (aggr_type != AggregationType::F_COUNT_ALL && aggr_type != AggregationType::NOTAGG)
                {
                    return RC::INVALID_ARGUMENT;
                }
                for (Table* table : tables) { wildcard_fields(table, query_fields, aggr_type); }
            }
            else
            {
                auto iter = table_map.find(table_name);
                if (iter == table_map.end())
                {
                    LOG_WARN("no such table in from list: %s", table_name);
                    return RC::SCHEMA_FIELD_MISSING;
                }

                Table* table = iter->second;
                if (0 == strcmp(field_name, "*"))
                {
                    if (aggr_type != AggregationType::F_COUNT_ALL && aggr_type != AggregationType::NOTAGG)
                    {
                        LOG_WARN("invalid aggregation type. field=%s.%s.%s", db->name(), table->name(), field_name);
                        return RC::INVALID_ARGUMENT;
                    }
                    wildcard_fields(table, query_fields, aggr_type);
                }
                else
                {
                    const FieldMeta* field_meta = table->table_meta().field(field_name);
                    if (nullptr == field_meta)
                    {
                        LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), field_name);
                        return RC::SCHEMA_FIELD_MISSING;
                    }
                    query_fields.push_back(Field(table, field_meta, aggr_type));
                }
            }
        }
        else
        {
            if (tables.size() != 1)
            {
                LOG_WARN("invalid. I do not know the attr's table. attr=%s", relation_attr.attribute_name.c_str());
                return RC::SCHEMA_FIELD_MISSING;
            }

            Table*           table      = tables[0];
            const FieldMeta* field_meta = table->table_meta().field(relation_attr.attribute_name.c_str());
            if (nullptr == field_meta)
            {
                LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), relation_attr.attribute_name.c_str());
                return RC::SCHEMA_FIELD_MISSING;
            }
            query_fields.push_back(Field(table, field_meta, aggr_type));
        }
    }

    LOG_INFO("got %d tables in from stmt and %d fields in query stmt", tables.size(), query_fields.size());

    Table* default_table = nullptr;
    if (tables.size() == 1) { default_table = tables[0]; }

    // create filter statement in `where` statement

    FilterStmt* filter_stmt = nullptr;
    RC          rc          = FilterStmt::create(db,
        default_table,
        &table_map,
        select_sql.conditions.data(),
        static_cast<int>(select_sql.conditions.size()),
        filter_stmt);
    if (rc != RC::SUCCESS)
    {
        LOG_WARN("cannot construct filter stmt");
        return rc;
    }
    // everything alright
    SelectStmt* select_stmt = new SelectStmt();
    // TODO add expression copy
    select_stmt->tables_.swap(tables);
    select_stmt->query_fields_.swap(query_fields);
    select_stmt->filter_stmt_ = filter_stmt;
    select_stmt->is_aggr_     = IsAgg;
    stmt                      = select_stmt;
    return RC::SUCCESS;
}
