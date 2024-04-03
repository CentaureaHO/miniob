#include "sql/parser/aggregation_parser.h"

const char* AGGREGATION_TYPE_NAME[] = {
    "not_func",
    "avg",
    "count",
    "count_all",
    "max",
    "min",
    "sum",
    "mulattrs",
    "composite",
    "unknown",
};

const char* aggregation_type_to_string(AggregationType type)
{
    if (type >= NOTAGG && type < UNKNOWN) return AGGREGATION_TYPE_NAME[type];
    return AGGREGATION_TYPE_NAME[UNKNOWN];
}

AggregationType aggregation_type_from_string(const char* s)
{
    for (unsigned int i = NOTAGG; i < UNKNOWN; i++)
        if (strcmp(s, AGGREGATION_TYPE_NAME[i]) == 0) return (AggregationType)i;
    return UNKNOWN;
}