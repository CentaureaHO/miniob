#pragma once

#include "sql/parser/parse_defs.h"
#include "common/rc.h"

/*
 * AggregationType
 *   NOTAGG,
 *   F_AVG,
 *   F_COUNT,
 *   F_MAX,
 *   F_MIN,
 *   F_SUM,
 *   MULATTRS,
 *   UNKNOWN,
 */

const char* aggregation_type_to_string(AggregationType type);
AggregationType aggregation_type_from_string(const char* s);

class AggregationOperator
{
  public:
    AggregationOperator(): func_type_(UNKNOWN) {}

    RC init(const std::string& func_name);
    RC run(std::vector<Value>& values, const char* field_name);

  private:
    AggregationType func_type_;
    std::string func_name_;

    RC Run_AVG(std::vector<Value>& values) const;
    RC Run_COUNT(std::vector<Value>& values) const;
    RC Run_MAX(std::vector<Value>& values) const;
    RC Run_MIN(std::vector<Value>& values) const;
    RC Run_SUM(std::vector<Value>& values) const;
};