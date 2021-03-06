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
#include "src/ast/type/array_type.h"
#include "src/ast/type/struct_type.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, GlobalDecl_Semicolon) {
  auto* p = parser(";");
  p->global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();
}

TEST_F(ParserImplTest, GlobalDecl_GlobalVariable) {
  auto* p = parser("var<out> a : vec2<i32> = vec2<i32>(1, 2);");
  p->global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();

  auto m = p->module();
  ASSERT_EQ(m.global_variables().size(), 1u);

  auto* v = m.global_variables()[0].get();
  EXPECT_EQ(v->name(), "a");
}

TEST_F(ParserImplTest, GlobalDecl_GlobalVariable_Invalid) {
  auto* p = parser("var<out> a : vec2<invalid>;");
  p->global_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:19: unknown constructed type 'invalid'");
}

TEST_F(ParserImplTest, GlobalDecl_GlobalVariable_MissingSemicolon) {
  auto* p = parser("var<out> a : vec2<i32>");
  p->global_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:23: missing ';' for variable declaration");
}

TEST_F(ParserImplTest, GlobalDecl_GlobalConstant) {
  auto* p = parser("const a : i32 = 2;");
  p->global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();

  auto m = p->module();
  ASSERT_EQ(m.global_variables().size(), 1u);

  auto* v = m.global_variables()[0].get();
  EXPECT_EQ(v->name(), "a");
}

TEST_F(ParserImplTest, GlobalDecl_GlobalConstant_Invalid) {
  auto* p = parser("const a : vec2<i32>;");
  p->global_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:20: missing = for const declaration");
}

TEST_F(ParserImplTest, GlobalDecl_GlobalConstant_MissingSemicolon) {
  auto* p = parser("const a : vec2<i32> = vec2<i32>(1, 2)");
  p->global_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:38: missing ';' for constant declaration");
}

TEST_F(ParserImplTest, GlobalDecl_TypeAlias) {
  auto* p = parser("type A = i32;");
  p->global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();

  auto m = p->module();
  ASSERT_EQ(m.constructed_types().size(), 1u);
  ASSERT_TRUE(m.constructed_types()[0]->IsAlias());
  EXPECT_EQ(m.constructed_types()[0]->AsAlias()->name(), "A");
}

TEST_F(ParserImplTest, GlobalDecl_TypeAlias_StructIdent) {
  auto* p = parser(R"(struct A {
  a : f32;
};
type B = A;)");
  p->global_decl();
  p->global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();

  auto m = p->module();
  ASSERT_EQ(m.constructed_types().size(), 2u);
  ASSERT_TRUE(m.constructed_types()[0]->IsStruct());
  auto* str = m.constructed_types()[0]->AsStruct();
  EXPECT_EQ(str->name(), "A");

  ASSERT_TRUE(m.constructed_types()[1]->IsAlias());
  auto* alias = m.constructed_types()[1]->AsAlias();
  EXPECT_EQ(alias->name(), "B");
  EXPECT_EQ(alias->type(), str);
}

TEST_F(ParserImplTest, GlobalDecl_TypeAlias_Invalid) {
  auto* p = parser("type A = invalid;");
  p->global_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:10: unknown constructed type 'invalid'");
}

TEST_F(ParserImplTest, GlobalDecl_TypeAlias_MissingSemicolon) {
  auto* p = parser("type A = i32");
  p->global_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:13: missing ';' for type alias");
}

TEST_F(ParserImplTest, GlobalDecl_Function) {
  auto* p = parser("fn main() -> void { return; }");
  p->global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();

  auto m = p->module();
  ASSERT_EQ(m.functions().size(), 1u);
  EXPECT_EQ(m.functions()[0]->name(), "main");
}

TEST_F(ParserImplTest, GlobalDecl_Function_WithDecoration) {
  auto* p = parser("[[workgroup_size(2)]] fn main() -> void { return; }");
  p->global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();

  auto m = p->module();
  ASSERT_EQ(m.functions().size(), 1u);
  EXPECT_EQ(m.functions()[0]->name(), "main");
}

TEST_F(ParserImplTest, GlobalDecl_Function_Invalid) {
  auto* p = parser("fn main() -> { return; }");
  p->global_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:14: unable to determine function return type");
}

TEST_F(ParserImplTest, GlobalDecl_ParsesStruct) {
  auto* p = parser("struct A { b: i32; c: f32;};");
  p->global_decl();
  ASSERT_FALSE(p->has_error());

  auto m = p->module();
  ASSERT_EQ(m.constructed_types().size(), 1u);

  auto* t = m.constructed_types()[0];
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->IsStruct());

  auto* str = t->AsStruct();
  EXPECT_EQ(str->name(), "A");
  EXPECT_EQ(str->impl()->members().size(), 2u);
}

TEST_F(ParserImplTest, GlobalDecl_Struct_WithStride) {
  auto* p =
      parser("struct A { [[offset(0)]] data: [[stride(4)]] array<f32>; };");
  p->global_decl();
  ASSERT_FALSE(p->has_error());

  auto m = p->module();
  ASSERT_EQ(m.constructed_types().size(), 1u);

  auto* t = m.constructed_types()[0];
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->IsStruct());

  auto* str = t->AsStruct();
  EXPECT_EQ(str->name(), "A");
  EXPECT_EQ(str->impl()->members().size(), 1u);
  EXPECT_FALSE(str->IsBlockDecorated());

  const auto* ty = str->impl()->members()[0]->type();
  ASSERT_TRUE(ty->IsArray());
  const auto* arr = ty->AsArray();
  EXPECT_TRUE(arr->has_array_stride());
  EXPECT_EQ(arr->array_stride(), 4u);
}

TEST_F(ParserImplTest, GlobalDecl_Struct_WithDecoration) {
  auto* p = parser("[[block]] struct A { [[offset(0)]] data: f32; };");
  p->global_decl();
  ASSERT_FALSE(p->has_error());

  auto m = p->module();
  ASSERT_EQ(m.constructed_types().size(), 1u);

  auto* t = m.constructed_types()[0];
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->IsStruct());

  auto* str = t->AsStruct();
  EXPECT_EQ(str->name(), "A");
  EXPECT_EQ(str->impl()->members().size(), 1u);
  EXPECT_TRUE(str->IsBlockDecorated());
}

TEST_F(ParserImplTest, GlobalDecl_Struct_Invalid) {
  auto* p = parser("[[block]] A {};");
  p->global_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:11: missing struct declaration");
}

TEST_F(ParserImplTest, GlobalDecl_StructMissing_Semi) {
  auto* p = parser("[[block]] struct A {}");
  p->global_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:22: missing ';' for struct declaration");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
