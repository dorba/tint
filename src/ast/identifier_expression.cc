// Copyright 2020 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/ast/identifier_expression.h"

namespace tint {
namespace ast {

IdentifierExpression::IdentifierExpression(const std::string& name)
    : Expression(), name_({name}) {}

IdentifierExpression::IdentifierExpression(const Source& source,
                                           const std::string& name)
    : Expression(source), name_({name}) {}

IdentifierExpression::IdentifierExpression(std::vector<std::string> name)
    : Expression(), name_(std::move(name)) {}

IdentifierExpression::IdentifierExpression(const Source& source,
                                           std::vector<std::string> name)
    : Expression(source), name_(std::move(name)) {}

IdentifierExpression::IdentifierExpression(IdentifierExpression&&) = default;

IdentifierExpression::~IdentifierExpression() = default;

bool IdentifierExpression::IsIdentifier() const {
  return true;
}

bool IdentifierExpression::IsValid() const {
  if (name_.size() == 0)
    return false;

  for (const auto& name : name_) {
    if (name.size() == 0)
      return false;
  }
  return true;
}

void IdentifierExpression::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Identifier{";
  bool first = true;
  for (const auto& name : name_) {
    if (!first)
      out << "::";

    first = false;
    out << name;
  }
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
