#pragma once
#include <cstring>
#include <string>

enum AggregationType
{
    NOTAGG,
    F_AVG,
    F_COUNT,
    F_COUNT_ALL,
    F_MAX,
    F_MIN,
    F_SUM,
    MULATTRS,
    COMPOSITE,
    UNKNOWN,
};

extern const char*     aggregation_type_to_string(AggregationType type);
extern AggregationType aggregation_type_from_string(const char* s);