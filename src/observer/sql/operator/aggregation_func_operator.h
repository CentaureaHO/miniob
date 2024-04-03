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

class AggregationOperator
{
  public:
    AggregationOperator() : func_type_(NOTAGG) {}

    RC init(const std::string& func_name);
    RC init(AggregationType type);
    RC run(std::vector<Value>& values, const char* field_name);

    AggregationType type() const { return func_type_; }

  private:
    AggregationType func_type_;
    std::string     func_name_;

    RC Run_AVG(std::vector<Value>& values) const;
    RC Run_COUNT(std::vector<Value>& values) const;
    RC Run_MAX(std::vector<Value>& values) const;
    RC Run_MIN(std::vector<Value>& values) const;
    RC Run_SUM(std::vector<Value>& values) const;

    RC Run_NOTAGG(std::vector<Value>& values) const;
};