// aggregation_logical_operator.cpp

#include "sql/operator/aggregate_logical_operator.h"

AggregationLogicalOperator::AggregationLogicalOperator(const std::vector<Field>& fields) : fields_(fields) {}