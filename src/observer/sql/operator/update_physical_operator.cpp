#include "sql/operator/update_physical_operator.h"
#include "sql/stmt/update_stmt.h"
#include "storage/table/table.h"
#include "storage/trx/trx.h"

using namespace std;

UpdatePhysicalOperator::UpdatePhysicalOperator(
    Table* table, const std::vector<Value>& values, const std::vector<std::string>& field_names)
    : table_(table), values_(values), field_names_(field_names)
{}

UpdatePhysicalOperator::~UpdatePhysicalOperator() {}

PhysicalOperatorType UpdatePhysicalOperator::type() const { return PhysicalOperatorType::UPDATE; }

RC UpdatePhysicalOperator::open(Trx* trx)
{
    if (children_.empty()) return RC::SUCCESS;

    std::unique_ptr<PhysicalOperator>& child = children_[0];
    RC                                 rc    = child->open(trx);
    if (rc != RC::SUCCESS)
    {
        LOG_WARN("Failed to open child operator: %s.", strrc(rc));
        return rc;
    }
    trx_ = trx;
    return RC::SUCCESS;
}

RC UpdatePhysicalOperator::next()
{
    if (children_.empty()) return RC::RECORD_EOF;

    PhysicalOperator* child = children_[0].get();
    RC                rc    = RC::SUCCESS;

    while (RC::SUCCESS == (rc = child->next()))
    {
        Tuple* tuple = child->current_tuple();
        if (!tuple) return rc;

        RowTuple* row_tuple = static_cast<RowTuple*>(tuple);
        Record&   record    = row_tuple->record();

        for (size_t i = 0; i < field_names_.size(); ++i)
        {
            const FieldMeta* fieldmeta = table_->table_meta().field(field_names_[i].c_str());
            if (!fieldmeta) return RC::SCHEMA_FIELD_MISSING;

            rc = trx_->update_record(table_, record, values_[i], fieldmeta->offset(), fieldmeta->len());
            if (rc != RC::SUCCESS)
            {
                LOG_WARN("Failed to update record: %s.", strrc(rc));
                return rc;
            }
        }
    }
    return RC::RECORD_EOF;
}

RC UpdatePhysicalOperator::close()
{
    if (!children_.empty()) children_[0]->close();
    return RC::SUCCESS;
}