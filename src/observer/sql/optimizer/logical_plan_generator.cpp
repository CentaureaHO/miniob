/* Copyright (c) 2023 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2023/08/16.
//

#include "sql/optimizer/logical_plan_generator.h"

#include "sql/operator/logical_operator.h"
#include "sql/operator/calc_logical_operator.h"
#include "sql/operator/project_logical_operator.h"
#include "sql/operator/predicate_logical_operator.h"
#include "sql/operator/table_get_logical_operator.h"
#include "sql/operator/insert_logical_operator.h"
#include "sql/operator/delete_logical_operator.h"
#include "sql/operator/join_logical_operator.h"
#include "sql/operator/project_logical_operator.h"
#include "sql/operator/explain_logical_operator.h"
#include "sql/operator/update_logical_operator.h"
#include "sql/operator/aggregate_logical_operator.h"

#include "sql/stmt/stmt.h"
#include "sql/stmt/calc_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "sql/stmt/insert_stmt.h"
#include "sql/stmt/delete_stmt.h"
#include "sql/stmt/explain_stmt.h"
#include "sql/stmt/update_stmt.h"

#include <iostream>

using namespace std;

RC LogicalPlanGenerator::create(Stmt* stmt, unique_ptr<LogicalOperator>& logical_operator)
{
    RC rc = RC::SUCCESS;
    switch (stmt->type())
    {
        case StmtType::CALC:
        {
            CalcStmt* calc_stmt = static_cast<CalcStmt*>(stmt);
            rc                  = create_plan(calc_stmt, logical_operator);
        }
        break;

        case StmtType::SELECT:
        {
            SelectStmt* select_stmt = static_cast<SelectStmt*>(stmt);
            rc                      = create_plan(select_stmt, logical_operator);
        }
        break;

        case StmtType::INSERT:
        {
            InsertStmt* insert_stmt = static_cast<InsertStmt*>(stmt);
            rc                      = create_plan(insert_stmt, logical_operator);
        }
        break;

        case StmtType::DELETE:
        {
            DeleteStmt* delete_stmt = static_cast<DeleteStmt*>(stmt);
            rc                      = create_plan(delete_stmt, logical_operator);
        }
        break;

        case StmtType::EXPLAIN:
        {
            ExplainStmt* explain_stmt = static_cast<ExplainStmt*>(stmt);
            rc                        = create_plan(explain_stmt, logical_operator);
        }
        break;

        case StmtType::UPDATE:
        {
            UpdateStmt* update_stmt = static_cast<UpdateStmt*>(stmt);
            rc                      = create_plan(update_stmt, logical_operator);
        }
        break;
        default:
        {
            rc = RC::UNIMPLENMENT;
        }
    }
    return rc;
}

RC LogicalPlanGenerator::create_plan(CalcStmt* calc_stmt, std::unique_ptr<LogicalOperator>& logical_operator)
{
    logical_operator.reset(new CalcLogicalOperator(std::move(calc_stmt->expressions())));
    return RC::SUCCESS;
}

RC LogicalPlanGenerator::create_plan(SelectStmt* select_stmt, unique_ptr<LogicalOperator>& logical_operator)
{
    unique_ptr<LogicalOperator> table_oper(nullptr);

    const std::vector<Table*>& tables     = select_stmt->tables();
    const std::vector<Field>&  all_fields = select_stmt->query_fields();
    for (Table* table : tables)
    {
        std::vector<Field> fields;
        for (const Field& field : all_fields)
        {
            if (0 == strcmp(field.table_name(), table->name())) { fields.push_back(field); }
        }

        unique_ptr<LogicalOperator> table_get_oper(new TableGetLogicalOperator(table, fields, true /*readonly*/));
        if (table_oper == nullptr) { table_oper = std::move(table_get_oper); }
        else
        {
            JoinLogicalOperator* join_oper = new JoinLogicalOperator;
            join_oper->add_child(std::move(table_oper));
            join_oper->add_child(std::move(table_get_oper));
            table_oper = unique_ptr<LogicalOperator>(join_oper);
        }
    }

    unique_ptr<LogicalOperator> predicate_oper;
    RC                          rc = create_plan(select_stmt->filter_stmt(), predicate_oper);
    if (rc != RC::SUCCESS)
    {
        LOG_WARN("failed to create predicate logical plan. rc=%s", strrc(rc));
        return rc;
    }

    unique_ptr<LogicalOperator> project_oper(new ProjectLogicalOperator(all_fields));
    if (predicate_oper)
    {
        if (table_oper) { predicate_oper->add_child(std::move(table_oper)); }
        project_oper->add_child(std::move(predicate_oper));
    }
    else
    {
        if (table_oper) { project_oper->add_child(std::move(table_oper)); }
    }

    if (select_stmt->is_agg())
    {
        unique_ptr<LogicalOperator> aggregation_oper(new AggregationLogicalOperator(all_fields));
        aggregation_oper->add_child(std::move(project_oper));
        logical_operator.swap(aggregation_oper);
    }
    else
        logical_operator.swap(project_oper);
    return RC::SUCCESS;
}

RC LogicalPlanGenerator::create_plan(FilterStmt* filter_stmt, unique_ptr<LogicalOperator>& logical_operator)
{
    std::vector<unique_ptr<Expression>> cmp_exprs;
    const std::vector<FilterUnit*>&     filter_units = filter_stmt->filter_units();
    for (const FilterUnit* filter_unit : filter_units)
    {
        const FilterObj& filter_obj_left  = filter_unit->left();
        const FilterObj& filter_obj_right = filter_unit->right();

        unique_ptr<Expression> left(filter_obj_left.is_attr
                                        ? static_cast<Expression*>(new FieldExpr(filter_obj_left.field))
                                        : static_cast<Expression*>(new ValueExpr(filter_obj_left.value)));

        unique_ptr<Expression> right(filter_obj_right.is_attr
                                         ? static_cast<Expression*>(new FieldExpr(filter_obj_right.field))
                                         : static_cast<Expression*>(new ValueExpr(filter_obj_right.value)));

        ComparisonExpr* cmp_expr = new ComparisonExpr(filter_unit->comp(), std::move(left), std::move(right));
        cmp_exprs.emplace_back(cmp_expr);
    }

    unique_ptr<PredicateLogicalOperator> predicate_oper;
    if (!cmp_exprs.empty())
    {
        unique_ptr<ConjunctionExpr> conjunction_expr(new ConjunctionExpr(ConjunctionExpr::Type::AND, cmp_exprs));
        predicate_oper =
            unique_ptr<PredicateLogicalOperator>(new PredicateLogicalOperator(std::move(conjunction_expr)));
    }

    logical_operator = std::move(predicate_oper);
    return RC::SUCCESS;
}

RC LogicalPlanGenerator::create_plan(InsertStmt* insert_stmt, unique_ptr<LogicalOperator>& logical_operator)
{
    Table*        table = insert_stmt->table();
    vector<Value> values(insert_stmt->values(), insert_stmt->values() + insert_stmt->value_amount());

    InsertLogicalOperator* insert_operator = new InsertLogicalOperator(table, values);
    logical_operator.reset(insert_operator);
    return RC::SUCCESS;
}

RC LogicalPlanGenerator::create_plan(DeleteStmt* delete_stmt, unique_ptr<LogicalOperator>& logical_operator)
{
    Table*             table       = delete_stmt->table();
    FilterStmt*        filter_stmt = delete_stmt->filter_stmt();
    std::vector<Field> fields;
    for (int i = table->table_meta().sys_field_num(); i < table->table_meta().field_num(); i++)
    {
        const FieldMeta* field_meta = table->table_meta().field(i);
        fields.push_back(Field(table, field_meta));
    }
    unique_ptr<LogicalOperator> table_get_oper(new TableGetLogicalOperator(table, fields, false /*readonly*/));

    unique_ptr<LogicalOperator> predicate_oper;
    RC                          rc = create_plan(filter_stmt, predicate_oper);
    if (rc != RC::SUCCESS) { return rc; }

    unique_ptr<LogicalOperator> delete_oper(new DeleteLogicalOperator(table));

    if (predicate_oper)
    {
        predicate_oper->add_child(std::move(table_get_oper));
        delete_oper->add_child(std::move(predicate_oper));
    }
    else { delete_oper->add_child(std::move(table_get_oper)); }

    logical_operator = std::move(delete_oper);
    return rc;
}

RC LogicalPlanGenerator::create_plan(ExplainStmt* explain_stmt, unique_ptr<LogicalOperator>& logical_operator)
{
    Stmt*                       child_stmt = explain_stmt->child();
    unique_ptr<LogicalOperator> child_oper;
    RC                          rc = create(child_stmt, child_oper);
    if (rc != RC::SUCCESS)
    {
        LOG_WARN("failed to create explain's child operator. rc=%s", strrc(rc));
        return rc;
    }

    logical_operator = unique_ptr<LogicalOperator>(new ExplainLogicalOperator);
    logical_operator->add_child(std::move(child_oper));
    return rc;
}

RC LogicalPlanGenerator::create_plan(UpdateStmt* update_stmt, unique_ptr<LogicalOperator>& logical_operator)
{
    // 从更新语句中获取表对象； 通过表对象获取表的元数据; 定义一个字段向量，用于存储表的所有字段
    Table*             table      = update_stmt->table();
    const TableMeta&   table_meta = table->table_meta();
    std::vector<Field> fields;

    // 遍历表的所有字段元数据，并将它们添加到fields向量中
    for (int i = 0; i < table_meta.field_num(); ++i)
    {
        const FieldMeta* field_meta = table_meta.field(i);
        fields.emplace_back(table, field_meta);
    }

    // 创建一个用于获取表数据的逻辑操作符，它将用于执行更新操作之前的数据检索
    auto table_get_oper = std::make_unique<TableGetLogicalOperator>(table, fields, false /* readonly */);

    unique_ptr<LogicalOperator> filter_oper;
    if (update_stmt->filter_stmt())
    {
        RC rc = create_plan(update_stmt->filter_stmt(), filter_oper);
        if (rc != RC::SUCCESS) return rc;
    }
    // 创建一个逻辑更新操作符，用于执行实际的更新操作
    auto update_oper =
        std::make_unique<UpdateLogicalOperator>(table, *update_stmt->values(), update_stmt->attribute_name().c_str());

    // 如果存在过滤操作符，将获取表数据的操作符设置为过滤操作符的子操作符，
    // 然后将过滤操作符设置为更新操作符的子操作符。
    // 这样，数据首先被获取，然后根据过滤条件进行筛选，最后执行更新。
    if (filter_oper)
    {
        filter_oper->add_child(std::move(table_get_oper));
        update_oper->add_child(std::move(filter_oper));
    }
    else
        // 如果不存在过滤条件，直接将获取表数据的操作符设置为更新操作符的子操作符
        update_oper->add_child(std::move(table_get_oper));

    // 将构建的更新操作符赋值给传入的引用，完成逻辑计划的创建
    logical_operator = std::move(update_oper);
    return RC::SUCCESS;
}
