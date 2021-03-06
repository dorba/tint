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
#include "src/ast/type/storage_texture_type.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, StorageTextureType_Invalid) {
  auto* p = parser("abc");
  auto t = p->storage_texture_type();
  EXPECT_EQ(t.first, ast::type::TextureDimension::kNone);
  EXPECT_EQ(t.second, ast::type::StorageAccess::kRead);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, StorageTextureType_Readonly1d) {
  auto* p = parser("texture_ro_1d");
  auto t = p->storage_texture_type();
  EXPECT_EQ(t.first, ast::type::TextureDimension::k1d);
  EXPECT_EQ(t.second, ast::type::StorageAccess::kRead);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, StorageTextureType_Readonly1dArray) {
  auto* p = parser("texture_ro_1d_array");
  auto t = p->storage_texture_type();
  EXPECT_EQ(t.first, ast::type::TextureDimension::k1dArray);
  EXPECT_EQ(t.second, ast::type::StorageAccess::kRead);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, StorageTextureType_Readonly2d) {
  auto* p = parser("texture_ro_2d");
  auto t = p->storage_texture_type();
  EXPECT_EQ(t.first, ast::type::TextureDimension::k2d);
  EXPECT_EQ(t.second, ast::type::StorageAccess::kRead);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, StorageTextureType_Readonly2dArray) {
  auto* p = parser("texture_ro_2d_array");
  auto t = p->storage_texture_type();
  EXPECT_EQ(t.first, ast::type::TextureDimension::k2dArray);
  EXPECT_EQ(t.second, ast::type::StorageAccess::kRead);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, StorageTextureType_Readonly3d) {
  auto* p = parser("texture_ro_3d");
  auto t = p->storage_texture_type();
  EXPECT_EQ(t.first, ast::type::TextureDimension::k3d);
  EXPECT_EQ(t.second, ast::type::StorageAccess::kRead);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, StorageTextureType_Writeonly1d) {
  auto* p = parser("texture_wo_1d");
  auto t = p->storage_texture_type();
  EXPECT_EQ(t.first, ast::type::TextureDimension::k1d);
  EXPECT_EQ(t.second, ast::type::StorageAccess::kWrite);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, StorageTextureType_Writeonly1dArray) {
  auto* p = parser("texture_wo_1d_array");
  auto t = p->storage_texture_type();
  EXPECT_EQ(t.first, ast::type::TextureDimension::k1dArray);
  EXPECT_EQ(t.second, ast::type::StorageAccess::kWrite);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, StorageTextureType_Writeonly2d) {
  auto* p = parser("texture_wo_2d");
  auto t = p->storage_texture_type();
  EXPECT_EQ(t.first, ast::type::TextureDimension::k2d);
  EXPECT_EQ(t.second, ast::type::StorageAccess::kWrite);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, StorageTextureType_Writeonly2dArray) {
  auto* p = parser("texture_wo_2d_array");
  auto t = p->storage_texture_type();
  EXPECT_EQ(t.first, ast::type::TextureDimension::k2dArray);
  EXPECT_EQ(t.second, ast::type::StorageAccess::kWrite);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, StorageTextureType_Writeonly3d) {
  auto* p = parser("texture_wo_3d");
  auto t = p->storage_texture_type();
  EXPECT_EQ(t.first, ast::type::TextureDimension::k3d);
  EXPECT_EQ(t.second, ast::type::StorageAccess::kWrite);
  EXPECT_FALSE(p->has_error());
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
