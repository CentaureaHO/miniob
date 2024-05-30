#include "sql/stmt/update_stmt.h"
#include <string>
#include "common/log/log.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "common/lang/string.h"

UpdateStmt::UpdateStmt(
    Table* table, std::vector<Value> values, FilterStmt* filter_stmt, std::vector<std::string> attribute_names)
    : table_(table), values_(std::move(values)), filter_stmt_(filter_stmt), attribute_names_(std::move(attribute_names))
{}

RC UpdateStmt::create(Db* db, const UpdateSqlNode& update_sql, Stmt*& stmt)
{
    if (!db || update_sql.relation_name.empty())
    {
        LOG_WARN("Invalid argument. db=%p, table_name=%s", db, update_sql.relation_name.c_str());
        return RC::INVALID_ARGUMENT;
    }
    Table* table = db->find_table(update_sql.relation_name.c_str());
    if (!table)
    {
        LOG_WARN("No such table. db=%s, table_name=%s", db->name(), update_sql.relation_name.c_str());
        return RC::SCHEMA_TABLE_NOT_EXIST;
    }
    std::vector<std::string> attribute_names = update_sql.attribute_names;
    std::vector<Value>       values          = update_sql.values;
    for (size_t i = 0; i < attribute_names.size(); ++i)
    {
        const FieldMeta* field_meta = table->table_meta().field(attribute_names[i].c_str());
        if (!field_meta)
        {
            LOG_WARN("Field '%s' not found in table '%s'.", attribute_names[i].c_str(), update_sql.relation_name.c_str());
            return RC::SCHEMA_FIELD_MISSING;
        }
        if (field_meta->type() != values[i].attr_type())
        {
            LOG_WARN("Type mismatch. Cannot update a %s field with a %s value.",
                     attr_type_to_string(field_meta->type()), attr_type_to_string(values[i].attr_type()));
            return RC::INVALID_ARGUMENT;
        }
    }
    std::unordered_map<std::string, Table*> table_map;
    table_map[table->name()] = table;
    FilterStmt* filter_stmt  = nullptr;
    RC          rc           = FilterStmt::create(
        db, table, &table_map, update_sql.conditions.data(), update_sql.conditions.size(), filter_stmt);
    if (rc != RC::SUCCESS)
    {
        LOG_WARN("Failed to create filter statement. rc=%d:%s", rc, strrc(rc));
        return rc;
    }
    stmt = new UpdateStmt(table, std::move(values), filter_stmt, std::move(attribute_names));
    return RC::SUCCESS;
}
