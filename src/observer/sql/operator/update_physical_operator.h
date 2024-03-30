#pragma once

#include "sql/operator/physical_operator.h"

class Trx;
class DeleteStmt;

/**
 * @brief 物理算子，更新
 * @ingroup PhysicalOperator
 */
class UpdatePhysicalOperator : public PhysicalOperator {
   public:
    UpdatePhysicalOperator(Table* table, Value& value, const char* field_name)
        : table_(table), value_(value) {
        // 复制 field_name
        field_name_ = new char[strlen(field_name) + 1];
        strcpy(field_name_, field_name);
    }

    // 析构函数
    ~UpdatePhysicalOperator() {
        delete[] field_name_;  // 释放 field_name 的内存
    }

    PhysicalOperatorType type() const override {
        return PhysicalOperatorType::UPDATE;
    }

    RC open(Trx* trx) override;
    RC next() override;
    RC close() override;

    Tuple* current_tuple() override {
        return nullptr;
    }

   private:
    Table* table_ = nullptr;
    Value value_;
    char* field_name_ = nullptr;
    Trx* trx_ = nullptr;
};