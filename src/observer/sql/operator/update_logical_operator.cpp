#include "sql/operator/update_logical_operator.h"

UpdateLogicalOperator::UpdateLogicalOperator(Table* table, Value& value, const char* field_name)
    : table_(table), value_(value), field_name_(field_name)
{}