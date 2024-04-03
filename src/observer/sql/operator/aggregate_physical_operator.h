// aggregation_physical_operator.h

#pragma once

#include "sql/operator/physical_operator.h"
#include "sql/operator/aggregation_func_operator.h"
#include "sql/expr/tuple.h"
#include <vector>

class Trx;

/**
 * @brief 物理算子，用于执行具体的聚合操作
 * @ingroup PhysicalOperator
 */
class AggregationPhysicalOperator : public PhysicalOperator
{
  public:
    AggregationPhysicalOperator() {}
    AggregationPhysicalOperator(const std::vector<AggregationType> aggs) : aggs_(aggs) {}
    virtual ~AggregationPhysicalOperator() = default;

    PhysicalOperatorType type() const override { return PhysicalOperatorType::AGGREGATE; }

    void add_aggregation(AggregationType agg);

    RC open(Trx* trx) override;
    RC next() override;
    RC close() override;

    Tuple* current_tuple() override { return &result_tuple_; }

  private:
    std::vector<AggregationType> aggs_;
    ValueListTuple               result_tuple_;
};
