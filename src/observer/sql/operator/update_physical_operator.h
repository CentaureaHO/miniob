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
    UpdatePhysicalOperator(Table* table, const std::vector<Value>& values, const std::vector<std::string>& field_names);
    virtual ~UpdatePhysicalOperator();

    PhysicalOperatorType type() const override;

    RC open(Trx* trx) override;
    RC next() override;
    RC close() override;

    Tuple* current_tuple() override { return nullptr; }

  private:
    Table*                   table_ = nullptr;
    std::vector<Value>       values_;
    std::vector<std::string> field_names_;
    Trx*                     trx_ = nullptr;
};