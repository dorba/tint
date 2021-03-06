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
#include "src/ast/type/depth_texture_type.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, DepthTextureType_Invalid) {
  auto* p = parser("1234");
  auto* t = p->depth_texture_type();
  EXPECT_EQ(t, nullptr);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, DepthTextureType_2d) {
  auto* p = parser("texture_depth_2d");
  auto* t = p->depth_texture_type();
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->IsTexture());
  ASSERT_TRUE(t->AsTexture()->IsDepth());
  EXPECT_EQ(t->AsTexture()->dim(), ast::type::TextureDimension::k2d);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, DepthTextureType_2dArray) {
  auto* p = parser("texture_depth_2d_array");
  auto* t = p->depth_texture_type();
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->IsTexture());
  ASSERT_TRUE(t->AsTexture()->IsDepth());
  EXPECT_EQ(t->AsTexture()->dim(), ast::type::TextureDimension::k2dArray);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, DepthTextureType_Cube) {
  auto* p = parser("texture_depth_cube");
  auto* t = p->depth_texture_type();
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->IsTexture());
  ASSERT_TRUE(t->AsTexture()->IsDepth());
  EXPECT_EQ(t->AsTexture()->dim(), ast::type::TextureDimension::kCube);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, DepthTextureType_CubeArray) {
  auto* p = parser("texture_depth_cube_array");
  auto* t = p->depth_texture_type();
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->IsTexture());
  ASSERT_TRUE(t->AsTexture()->IsDepth());
  EXPECT_EQ(t->AsTexture()->dim(), ast::type::TextureDimension::kCubeArray);
  EXPECT_FALSE(p->has_error());
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
