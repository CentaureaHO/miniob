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
// Created by WangYunlai on 2022/07/05.
//

#include <sstream>
#include "sql/expr/tuple_cell.h"
#include "storage/field/field.h"
#include "common/log/log.h"
#include "common/lang/comparator.h"
#include "common/lang/string.h"

TupleCellSpec::TupleCellSpec(
    const char* table_name, const char* field_name, const char* alias, AggregationType agg_type)
{
    if (table_name) { table_name_ = table_name; }
    if (field_name) { field_name_ = field_name; }
    if (alias) { alias_ = alias; }
    else
    {
        if (table_name_.empty()) { alias_ = field_name_; }
        else { alias_ = table_name_ + "." + field_name_; }
    }

    if (agg_type != AggregationType::NOTAGG)
    {
        if (agg_type == AggregationType::F_COUNT_ALL)
            alias_ = "COUNT(*)";
        else
        {
            agg_   = std::string(aggregation_type_to_string(agg_type));
            alias_ = agg_ + "(" + alias_ + ")";
            for (char& c : alias_) { c = std::toupper(c); }
        }
    }
}

TupleCellSpec::TupleCellSpec(const char* alias, AggregationType agg_type)
{
    if (alias) { alias_ = alias; }
    if (agg_type != AggregationType::NOTAGG)
    {
        if (agg_type == AggregationType::F_COUNT_ALL)
            alias_ = "COUNT(*)";
        else
        {
            agg_   = std::string(aggregation_type_to_string(agg_type));
            alias_ = agg_ + "(" + alias_ + ")";
            for (char& c : alias_) { c = std::toupper(c); }
        }
    }
}
