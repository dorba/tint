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

#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "src/ast/break_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/writer/wgsl/generator_impl.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using GeneratorImplTest = testing::Test;

TEST_F(GeneratorImplTest, Emit_Break) {
  ast::BreakStatement b;

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&b)) << g.error();
  EXPECT_EQ(g.result(), "  break;\n");
}

TEST_F(GeneratorImplTest, Emit_BreakWithIf) {
  auto expr = std::make_unique<ast::IdentifierExpression>("expr");
  ast::BreakStatement b(ast::StatementCondition::kIf, std::move(expr));

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&b)) << g.error();
  EXPECT_EQ(g.result(), "  break if (expr);\n");
}

TEST_F(GeneratorImplTest, Emit_BreakWithUnless) {
  auto expr = std::make_unique<ast::IdentifierExpression>("expr");
  ast::BreakStatement b(ast::StatementCondition::kUnless, std::move(expr));

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&b)) << g.error();
  EXPECT_EQ(g.result(), "  break unless (expr);\n");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint