// aggregation_logical_operator.h

#pragma once

#include "sql/operator/logical_operator.h"
#include "sql/parser/parse_defs.h"
#include "storage/field/field.h"
#include <vector>

/**
 * @brief 逻辑算子，用于执行聚合操作
 * @ingroup LogicalOperator
 */
class AggregationLogicalOperator : public LogicalOperator
{
  public:
    AggregationLogicalOperator(const std::vector<Field>& fields);
    virtual ~AggregationLogicalOperator() = default;

    LogicalOperatorType       type() const override { return LogicalOperatorType::AGGREGATE; }
    const std::vector<Field>& fields() const { return fields_; }

  private:
    std::vector<Field> fields_;
};
