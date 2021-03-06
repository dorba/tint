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

#include "gtest/gtest.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/location_decoration.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/module.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/msl/generator_impl.h"

#include <memory>

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = testing::Test;

TEST_F(MslGeneratorImplTest, Emit_Function_EntryPointData_Vertex_Input) {
  // [[location 0]] var<in> foo : f32;
  // [[location 1]] var<in> bar : i32;
  //
  // struct vtx_main_in {
  //   float foo [[attribute(0)]];
  //   int bar [[attribute(1)]];
  // };

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto foo_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("foo", ast::StorageClass::kInput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::LocationDecoration>(0));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("bar", ast::StorageClass::kInput, &i32));
  decos.push_back(std::make_unique<ast::LocationDecoration>(1));
  bar_var->set_decorations(std::move(decos));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(foo_var.get());
  td.RegisterVariableForTesting(bar_var.get());

  mod.AddGlobalVariable(std::move(foo_var));
  mod.AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("vtx_main", std::move(params), &f32);
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  auto* func_ptr = func.get();

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("foo"),
      std::make_unique<ast::IdentifierExpression>("foo")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("bar"),
      std::make_unique<ast::IdentifierExpression>("bar")));
  func->set_body(std::move(body));

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl g(&mod);
  ASSERT_TRUE(g.EmitEntryPointData(func_ptr)) << g.error();
  EXPECT_EQ(g.result(), R"(struct vtx_main_in {
  float foo [[attribute(0)]];
  int bar [[attribute(1)]];
};

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_EntryPointData_Vertex_Output) {
  // [[location 0]] var<out> foo : f32;
  // [[location 1]] var<out> bar : i32;
  //
  // struct vtx_main_out {
  //   float foo [[user(locn0)]];
  //   int bar [[user(locn1)]];
  // };

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto foo_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("foo", ast::StorageClass::kOutput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::LocationDecoration>(0));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("bar", ast::StorageClass::kOutput, &i32));
  decos.push_back(std::make_unique<ast::LocationDecoration>(1));
  bar_var->set_decorations(std::move(decos));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(foo_var.get());
  td.RegisterVariableForTesting(bar_var.get());

  mod.AddGlobalVariable(std::move(foo_var));
  mod.AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("vtx_main", std::move(params), &f32);
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  auto* func_ptr = func.get();

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("foo"),
      std::make_unique<ast::IdentifierExpression>("foo")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("bar"),
      std::make_unique<ast::IdentifierExpression>("bar")));
  func->set_body(std::move(body));

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl g(&mod);
  ASSERT_TRUE(g.EmitEntryPointData(func_ptr)) << g.error();
  EXPECT_EQ(g.result(), R"(struct vtx_main_out {
  float foo [[user(locn0)]];
  int bar [[user(locn1)]];
};

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_EntryPointData_Fragment_Input) {
  // [[location 0]] var<in> foo : f32;
  // [[location 1]] var<in> bar : i32;
  //
  // struct frag_main_in {
  //   float foo [[user(locn0)]];
  //   int bar [[user(locn1)]];
  // };

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto foo_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("foo", ast::StorageClass::kInput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::LocationDecoration>(0));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("bar", ast::StorageClass::kInput, &i32));
  decos.push_back(std::make_unique<ast::LocationDecoration>(1));
  bar_var->set_decorations(std::move(decos));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(foo_var.get());
  td.RegisterVariableForTesting(bar_var.get());

  mod.AddGlobalVariable(std::move(foo_var));
  mod.AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("main", std::move(params), &f32);
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));
  auto* func_ptr = func.get();

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("foo"),
      std::make_unique<ast::IdentifierExpression>("foo")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("bar"),
      std::make_unique<ast::IdentifierExpression>("bar")));
  func->set_body(std::move(body));

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl g(&mod);
  ASSERT_TRUE(g.EmitEntryPointData(func_ptr)) << g.error();
  EXPECT_EQ(g.result(), R"(struct main_in {
  float foo [[user(locn0)]];
  int bar [[user(locn1)]];
};

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_EntryPointData_Fragment_Output) {
  // [[location 0]] var<out> foo : f32;
  // [[location 1]] var<out> bar : i32;
  //
  // struct frag_main_out {
  //   float foo [[color(0)]];
  //   int bar [[color(1)]];
  // };

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto foo_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("foo", ast::StorageClass::kOutput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::LocationDecoration>(0));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("bar", ast::StorageClass::kOutput, &i32));
  decos.push_back(std::make_unique<ast::LocationDecoration>(1));
  bar_var->set_decorations(std::move(decos));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(foo_var.get());
  td.RegisterVariableForTesting(bar_var.get());

  mod.AddGlobalVariable(std::move(foo_var));
  mod.AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("main", std::move(params), &f32);
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));
  auto* func_ptr = func.get();

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("foo"),
      std::make_unique<ast::IdentifierExpression>("foo")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("bar"),
      std::make_unique<ast::IdentifierExpression>("bar")));
  func->set_body(std::move(body));

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl g(&mod);
  ASSERT_TRUE(g.EmitEntryPointData(func_ptr)) << g.error();
  EXPECT_EQ(g.result(), R"(struct main_out {
  float foo [[color(0)]];
  int bar [[color(1)]];
};

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_EntryPointData_Compute_Input) {
  // [[location 0]] var<in> foo : f32;
  // [[location 1]] var<in> bar : i32;
  //
  // -> Error, not allowed

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto foo_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("foo", ast::StorageClass::kInput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::LocationDecoration>(0));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("bar", ast::StorageClass::kInput, &i32));
  decos.push_back(std::make_unique<ast::LocationDecoration>(1));
  bar_var->set_decorations(std::move(decos));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(foo_var.get());
  td.RegisterVariableForTesting(bar_var.get());

  mod.AddGlobalVariable(std::move(foo_var));
  mod.AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("main", std::move(params), &f32);
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kCompute));
  auto* func_ptr = func.get();

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("foo"),
      std::make_unique<ast::IdentifierExpression>("foo")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("bar"),
      std::make_unique<ast::IdentifierExpression>("bar")));
  func->set_body(std::move(body));

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl g(&mod);
  ASSERT_FALSE(g.EmitEntryPointData(func_ptr)) << g.error();
  EXPECT_EQ(g.error(), R"(invalid location variable for pipeline stage)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_EntryPointData_Compute_Output) {
  // [[location 0]] var<out> foo : f32;
  // [[location 1]] var<out> bar : i32;
  //
  // -> Error not allowed

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto foo_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("foo", ast::StorageClass::kOutput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::LocationDecoration>(0));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("bar", ast::StorageClass::kOutput, &i32));
  decos.push_back(std::make_unique<ast::LocationDecoration>(1));
  bar_var->set_decorations(std::move(decos));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(foo_var.get());
  td.RegisterVariableForTesting(bar_var.get());

  mod.AddGlobalVariable(std::move(foo_var));
  mod.AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("main", std::move(params), &f32);
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kCompute));
  auto* func_ptr = func.get();

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("foo"),
      std::make_unique<ast::IdentifierExpression>("foo")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("bar"),
      std::make_unique<ast::IdentifierExpression>("bar")));
  func->set_body(std::move(body));

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl g(&mod);
  ASSERT_FALSE(g.EmitEntryPointData(func_ptr)) << g.error();
  EXPECT_EQ(g.error(), R"(invalid location variable for pipeline stage)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_EntryPointData_Builtins) {
  // Output builtins go in the output struct, input builtins will be passed
  // as input parameters to the entry point function.

  // [[builtin frag_coord]] var<in> coord : vec4<f32>;
  // [[builtin frag_depth]] var<out> depth : f32;
  //
  // struct main_out {
  //   float depth [[depth(any)]];
  // };

  ast::type::F32Type f32;
  ast::type::VoidType void_type;
  ast::type::VectorType vec4(&f32, 4);

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "coord", ast::StorageClass::kInput, &vec4));

  ast::VariableDecorationList decos;
  decos.push_back(
      std::make_unique<ast::BuiltinDecoration>(ast::Builtin::kFragCoord));
  coord_var->set_decorations(std::move(decos));

  auto depth_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "depth", ast::StorageClass::kOutput, &f32));
  decos.push_back(
      std::make_unique<ast::BuiltinDecoration>(ast::Builtin::kFragDepth));
  depth_var->set_decorations(std::move(decos));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(coord_var.get());
  td.RegisterVariableForTesting(depth_var.get());

  mod.AddGlobalVariable(std::move(coord_var));
  mod.AddGlobalVariable(std::move(depth_var));

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("main", std::move(params), &void_type);
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));
  auto* func_ptr = func.get();

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("depth"),
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("coord"),
          std::make_unique<ast::IdentifierExpression>("x"))));
  func->set_body(std::move(body));

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl g(&mod);
  ASSERT_TRUE(g.EmitEntryPointData(func_ptr)) << g.error();
  EXPECT_EQ(g.result(), R"(struct main_out {
  float depth [[depth(any)]];
};

)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
