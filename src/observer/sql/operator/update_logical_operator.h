#pragma once

#include "sql/operator/logical_operator.h"

/**
 * @brief 逻辑算子，用于执行 update 语句
 * @ingroup LogicalOperator
 */
class UpdateLogicalOperator : public LogicalOperator
{
  public:
    UpdateLogicalOperator(Table* table, const std::vector<Value>& values, const std::vector<std::string>& field_names);
    virtual ~UpdateLogicalOperator() = default;

    LogicalOperatorType             type() const override { return LogicalOperatorType::UPDATE; }
    Table*                          table() const { return table_; }
    const std::vector<Value>&       values() const { return values_; }
    const std::vector<std::string>& field_names() const { return field_names_; }

  private:
    Table*                   table_ = nullptr;
    std::vector<Value>       values_;
    std::vector<std::string> field_names_;
};