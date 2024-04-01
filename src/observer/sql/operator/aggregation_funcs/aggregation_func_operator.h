#pragma once

#include "sql/parser/parse_defs.h"
#include "common/rc.h"

class AggregationOperator
{
  public:
    AggregationOperator() = default;
    AggregationOperator(AggregationType func_type) : func_type_(func_type) {}
    AggregationOperator(const AggregationOperator& AO) : func_type_(AO.func_type_) {}

    static RC run();

  private:
    AggregationType func_type_;
};