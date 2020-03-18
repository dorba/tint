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
#include "src/writer/wgsl/generator_impl.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using GeneratorImplTest = testing::Test;

TEST_F(GeneratorImplTest, EmitImport) {
  ast::Import import("GLSL.std.450", "std::glsl");

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitImport(&import)) << g.error();
  EXPECT_EQ(g.result(), R"(import "GLSL.std.450" as std::glsl;
)");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint