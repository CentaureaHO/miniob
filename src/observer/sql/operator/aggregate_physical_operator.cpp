// aggregation_physical_operator.cpp

#include "sql/operator/aggregation_func_operator.h"
#include "sql/operator/aggregate_physical_operator.h"
#include "common/log/log.h"
#include "storage/trx/trx.h"
#include <iostream>

void AggregationPhysicalOperator::add_aggregation(AggregationType agg) { aggs_.push_back(agg); }

RC AggregationPhysicalOperator::open(Trx* trx)
{
    if (children_.empty()) { return RC::SUCCESS; }

    std::unique_ptr<PhysicalOperator>& child = children_[0];
    RC                                 rc    = child->open(trx);
    if (rc != RC::SUCCESS)
    {
        LOG_WARN("failed to open child operator: %s", strrc(rc));
        return rc;
    }

    return RC::SUCCESS;
}

RC AggregationPhysicalOperator::next()
{
    if (children_.empty()) return RC::SUCCESS;
    PhysicalOperator* oper = children_[0].get();
    if (!oper) return RC::INTERNAL;
    if (result_tuple_.cell_num() > 0) return RC::RECORD_EOF;
    std::vector<AggregationOperator> aggregationOps(aggs_.size());
    std::vector<std::vector<Value>>  aggregatedValues(aggs_.size());
    for (size_t i = 0; i < aggs_.size(); ++i) aggregationOps[i].init(aggs_[i]);
    RC rc = RC::SUCCESS;
    while ((rc = oper->next()) == RC::SUCCESS)
    {
        Tuple* tuple = oper->current_tuple();
        if (!tuple) continue;

        for (size_t i = 0; i < aggs_.size(); ++i)
        {
            Value cell;
            rc = tuple->cell_at(i, cell);
            if (rc != RC::SUCCESS) return rc;
            aggregatedValues[i].push_back(cell);
        }
    }

    std::vector<Value> resultCells(aggs_.size(), Value());
    for (size_t i = 0; i < aggs_.size(); ++i)
    {
        // rc = aggregationOps[i].run(aggregatedValues[i], "");
        // 不知为和，如果以上边的形式运行就会返回empty
        if (aggregationOps[i].run(aggregatedValues[i], "") != RC::SUCCESS)
        {
            // std::cout << "failed to run aggregation operator: " << strrc(rc) << '\n'; 
            LOG_WARN("failed to run aggregation operator.");
            return rc;
        }
        if (!aggregatedValues[i].empty()) resultCells[i] = aggregatedValues[i][0];
    }

    if (rc == RC::RECORD_EOF || aggs_.empty())
    {
        result_tuple_.set_cells(resultCells);
        rc = RC::SUCCESS;
    }

    return rc;
}

RC AggregationPhysicalOperator::close()
{
    RC rc = RC::SUCCESS;
    for (auto& child : children_)
        if (child)
        {
            rc = child->close();
            if (rc != RC::SUCCESS) LOG_WARN("failed to close child operator: %s", strrc(rc));
        }

    return rc;
}
