#include "sql/operator/aggregation_func_operator.h"
#include "sql/parser/value.h"
#include "common/log/log.h"

#include <cstring>
#include <string>
#include <vector>

RC AggregationOperator::init(const std::string& func_name)
{
    func_name_ = func_name;
    func_type_ = aggregation_type_from_string(func_name.c_str());
    if (func_type_ == AggregationType::UNKNOWN)
    {
        LOG_INFO("Unknown aggregation function: %s", func_name.c_str());
        return RC::UNKNOWN_AGGREGATION_FUNC;
    }
    else if (func_type_ == AggregationType::MULATTRS)
    {
        LOG_INFO("Aggregation function %s can't be applied to multiple attributes", func_name.c_str());
        return RC::AGGREGATION_UNMATCHED;
    }
    return RC::SUCCESS;
}

RC AggregationOperator::init(AggregationType type)
{
    func_type_ = type;
    if (func_type_ == AggregationType::UNKNOWN)
    {
        LOG_INFO("Unknown aggregation function");
        return RC::UNKNOWN_AGGREGATION_FUNC;
    }
    else if (func_type_ == AggregationType::MULATTRS)
    {
        LOG_INFO("Aggregation function can't be applied to multiple attributes");
        return RC::AGGREGATION_UNMATCHED;
    }
    return RC::SUCCESS;
}

RC AggregationOperator::run(std::vector<Value>& values, const char* field_name)
{
    if (values.empty())
    {
        LOG_INFO("No values to aggregate for field %s", field_name);
        return RC::EMPTY;
    }
    switch (func_type_)
    {
        case F_AVG: return Run_AVG(values);
        case F_COUNT:
        case F_COUNT_ALL: return Run_COUNT(values);
        case F_MAX: return Run_MAX(values);
        case F_MIN: return Run_MIN(values);
        case F_SUM: return Run_SUM(values);
        case NOTAGG: return Run_NOTAGG(values);
        default: LOG_INFO("Unsupported aggregation function: %s", func_name_.c_str()); return RC::AGGREGATION_UNMATCHED;
    }
}

RC AggregationOperator::Run_AVG(std::vector<Value>& values) const
{
    double   Sum       = 0;
    AttrType ValueType = UNDEFINED;
    for (auto& value : values)
    {
        AttrType ValueType = value.attr_type();
        if (ValueType != AttrType::INTS && ValueType != AttrType::FLOATS)
        {
            LOG_INFO("Invalid type for Aggregation func: AVG: %s", attr_type_to_string(value.attr_type()));
            return RC::AGGREGATION_UNMATCHED;
        }

        Sum += (ValueType == AttrType::INTS) ? value.get_int() : value.get_float();
    }

    values[0].set_float(Sum / values.size());
    values.erase(values.begin() + 1, values.end());
    return RC::SUCCESS;
}

RC AggregationOperator::Run_COUNT(std::vector<Value>& values) const
{
    values[0].set_int(values.size());
    values.erase(values.begin() + 1, values.end());
    return RC::SUCCESS;
}

RC AggregationOperator::Run_MAX(std::vector<Value>& values) const
{
    Value* MaxValue = &values[0];
    for (auto& value : values)
        if (MaxValue->compare(value) < 0) MaxValue = &value;
    values[0].set_value(*MaxValue);
    values.erase(values.begin() + 1, values.end());
    return RC::SUCCESS;
}

RC AggregationOperator::Run_MIN(std::vector<Value>& values) const
{
    Value* MinValue = &values[0];
    for (auto& value : values)
        if (MinValue->compare(value) > 0) MinValue = &value;
    values[0].set_value(*MinValue);
    values.erase(values.begin() + 1, values.end());
    return RC::SUCCESS;
}

RC AggregationOperator::Run_SUM(std::vector<Value>& values) const
{
    double   Sum       = 0;
    AttrType ValueType = UNDEFINED;
    for (auto& value : values)
    {
        AttrType ValueType = value.attr_type();
        if (ValueType != AttrType::INTS && ValueType != AttrType::FLOATS)
        {
            LOG_INFO("Invalid type for Aggregation func: SUM: %s", attr_type_to_string(value.attr_type()));
            return RC::AGGREGATION_UNMATCHED;
        }

        Sum += (ValueType == AttrType::INTS) ? value.get_int() : value.get_float();
    }

    values[0].set_float(Sum);
    values.erase(values.begin() + 1, values.end());
    return RC::SUCCESS;
}

RC AggregationOperator::Run_NOTAGG(std::vector<Value>& values) const
{
    // values.erase(values.begin() + 1, values.end());
    return RC::SUCCESS;
}