// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".fef.indexenvironment");

#include <vespa/vespalib/util/stringfmt.h>
#include <vespa/searchlib/attribute/attributefactory.h>
#include "indexenvironment.h"

namespace search {
namespace fef {
namespace test {

using vespalib::eval::ValueType;
using vespalib::eval::ErrorValue;

namespace {

IndexEnvironment::Constant notFoundError(ValueType::error_type(),
                                         std::make_unique<ErrorValue>());

}

IndexEnvironment::IndexEnvironment() :
    _properties(),
    _fields(),
    _attrMap(),
    _tableMan(),
    _constants()
{
}

const FieldInfo *
IndexEnvironment::getField(uint32_t id) const
{
    return id < _fields.size() ? &_fields[id] : NULL;
}

const FieldInfo *
IndexEnvironment::getFieldByName(const string &name) const
{
    for (std::vector<FieldInfo>::const_iterator it = _fields.begin();
         it != _fields.end(); ++it) {
        if (it->name() == name) {
            return &(*it);
        }
    }
    return NULL;
}


vespalib::eval::ConstantValue::UP
IndexEnvironment::getConstantValue(const vespalib::string &name) const
{
    auto it = _constants.find(name);
    if (it != _constants.end()) {
        return std::make_unique<ConstantRef>(it->second);
    } else {
        return std::make_unique<ConstantRef>(notFoundError);
    }
}

void
IndexEnvironment::addConstantValue(const vespalib::string &name,
                                   vespalib::eval::ValueType type,
                                   std::unique_ptr<vespalib::eval::Value> value)
{
    auto insertRes = _constants.emplace(name,
                                        Constant(std::move(type),
                                                 std::move(value)));
    assert(insertRes.second); // successful insert
    (void) insertRes;
}

} // namespace test
} // namespace fef
} // namespace search
