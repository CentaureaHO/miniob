#pragma once

#include "sql/operator/physical_operator.h"

class Trx;
class DeleteStmt;

/**
 * @brief 物理算子
 * @ingroup PhysicalOperator
 */
class UpdatePhysicalOperator : public PhysicalOperator
{
  public:
    UpdatePhysicalOperator(Table* table, Value& value, const char* field_name);
    ~UpdatePhysicalOperator();

    PhysicalOperatorType type() const override;

    RC open(Trx* trx) override;
    RC next() override;
    RC close() override;

    Tuple* current_tuple() override { return nullptr; }

  private:
    Table* table_ = nullptr;
    Value  value_;
    char*  field_name_ = nullptr;
    Trx*   trx_        = nullptr;
};