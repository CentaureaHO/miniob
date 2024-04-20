#include "sql/operator/update_logical_operator.h"

UpdateLogicalOperator::UpdateLogicalOperator(
    Table* table, const std::vector<Value>& values, const std::vector<std::string>& field_names)
    : table_(table), values_(values), field_names_(field_names)
{}