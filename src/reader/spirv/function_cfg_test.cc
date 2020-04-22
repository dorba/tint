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

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "src/reader/spirv/function.h"
#include "src/reader/spirv/parser_impl.h"
#include "src/reader/spirv/parser_impl_test_helper.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::ElementsAre;

std::string CommonTypes() {
  return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %bool = OpTypeBool
    %cond = OpUndef %bool
    %cond2 = OpUndef %bool
    %cond3 = OpUndef %bool

    %uint = OpTypeInt 32 0
    %selector = OpUndef %uint
  )";
}

TEST_F(SpvParserTest, ComputeBlockOrder_OneBlock) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %42 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(42));
}

TEST_F(SpvParserTest, ComputeBlockOrder_IgnoreStaticalyUnreachable) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %15 = OpLabel ; statically dead
     OpReturn

     %20 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20));
}

TEST_F(SpvParserTest, ComputeBlockOrder_KillIsDeadEnd) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %15 = OpLabel ; statically dead
     OpReturn

     %20 = OpLabel
     OpKill        ; Kill doesn't lead anywhere

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20));
}

TEST_F(SpvParserTest, ComputeBlockOrder_UnreachableIsDeadEnd) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %15 = OpLabel ; statically dead
     OpReturn

     %20 = OpLabel
     OpUnreachable ; Unreachable doesn't lead anywhere

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20));
}

TEST_F(SpvParserTest, ComputeBlockOrder_ReorderSequence) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %30 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %30 ; backtrack

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30));
}

TEST_F(SpvParserTest, ComputeBlockOrder_DupConditionalBranch) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %20

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_RespectConditionalBranchOrder) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %99 = OpLabel
     OpReturn

     %30 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_TrueOnlyBranch) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_FalseOnlyBranch) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %99 %20

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_SwitchOrderNaturallyReversed) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 30 %30

     %99 = OpLabel
     OpReturn

     %30 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 30, 20, 99));
}

TEST_F(SpvParserTest,
       ComputeBlockOrder_SwitchWithDefaultOrderNaturallyReversed) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %80 20 %20 30 %30

     %80 = OpLabel ; the default case
     OpBranch %99

     %99 = OpLabel
     OpReturn

     %30 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 30, 20, 80, 99));
}

TEST_F(SpvParserTest,
       ComputeBlockOrder_Switch_DefaultSameAsACase) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %30 20 %20 30 %30 40 %40

     %99 = OpLabel
     OpReturn

     %30 = OpLabel
     OpBranch %99

     %20 = OpLabel
     OpBranch %99

     %40 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 40, 20, 30, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_RespectSwitchCaseFallthrough) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 30 %30 40 %40 50 %50

     %50 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     %40 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %50 ; fallthrough

     %20 = OpLabel
     OpBranch %40 ; fallthrough

     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 30, 50, 20, 40, 99)) << assembly;
}

TEST_F(SpvParserTest,
       ComputeBlockOrder_RespectSwitchCaseFallthrough_FromDefault) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %80 20 %20 30 %30 40 %40

     %80 = OpLabel ; the default case
     OpBranch %30 ; fallthrough to another case

     %99 = OpLabel
     OpReturn

     %40 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %40

     %20 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 80, 30, 40, 99)) << assembly;
}

TEST_F(SpvParserTest,
       ComputeBlockOrder_RespectSwitchCaseFallthrough_FromCaseToDefaultToCase) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %80 20 %20 30 %30

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %80 ; fallthrough to default

     %80 = OpLabel ; the default case
     OpBranch %30 ; fallthrough to 30

     %30 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 80, 30, 99)) << assembly;
}

TEST_F(SpvParserTest,
       ComputeBlockOrder_SwitchCasesFallthrough_OppositeDirections) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 30 %30 40 %40 50 %50

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %30 ; forward

     %40 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     ; SPIR-V doesn't actually allow a fall-through that goes backward in the
     ; module. But the block ordering algorithm tolerates it.
     %50 = OpLabel
     OpBranch %40 ; backward

     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 50, 40, 20, 30, 99)) << assembly;
}

TEST_F(SpvParserTest,
       ComputeBlockOrder_RespectSwitchCaseFallthrough_Interleaved) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 30 %30 40 %40 50 %50

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %40

     %30 = OpLabel
     OpBranch %50

     %40 = OpLabel
     OpBranch %60

     %50 = OpLabel
     OpBranch %70

     %60 = OpLabel
     OpBranch %99

     %70 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 30, 50, 70, 20, 40, 60, 99))
      << assembly;
}

TEST_F(SpvParserTest,
       ComputeBlockOrder_Nest_If_Contains_If) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %50

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond %30 %40

     %49 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %49

     %40 = OpLabel
     OpBranch %49

     %50 = OpLabel
     OpSelectionMerge %79 None
     OpBranchConditional %cond %60 %70

     %79 = OpLabel
     OpBranch %99

     %60 = OpLabel
     OpBranch %79

     %70 = OpLabel
     OpBranch %79

     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 40, 49, 50, 60, 70, 79, 99))
      << assembly;
}

TEST_F(SpvParserTest,
       ComputeBlockOrder_Nest_If_In_SwitchCase) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %50 20 %20 50 %50

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond %30 %40

     %49 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %49

     %40 = OpLabel
     OpBranch %49

     %50 = OpLabel
     OpSelectionMerge %79 None
     OpBranchConditional %cond %60 %70

     %79 = OpLabel
     OpBranch %99

     %60 = OpLabel
     OpBranch %79

     %70 = OpLabel
     OpBranch %79

     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 40, 49, 50, 60, 70, 79, 99))
      << assembly;
}

TEST_F(SpvParserTest,
       ComputeBlockOrder_Nest_IfFallthrough_In_SwitchCase) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %50 20 %20 50 %50

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond %30 %40

     %49 = OpLabel
     OpBranchConditional %cond %99 %50 ; fallthrough

     %30 = OpLabel
     OpBranch %49

     %40 = OpLabel
     OpBranch %49

     %50 = OpLabel
     OpSelectionMerge %79 None
     OpBranchConditional %cond %60 %70

     %79 = OpLabel
     OpBranch %99

     %60 = OpLabel
     OpBranch %79

     %70 = OpLabel
     OpBranch %79

     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 40, 49, 50, 60, 70, 79, 99))
      << assembly;
}

TEST_F(SpvParserTest,
       ComputeBlockOrder_Nest_IfBreak_In_SwitchCase) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %50 20 %20 50 %50

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond %99 %40 ; break-if

     %49 = OpLabel
     OpBranch %99

     %40 = OpLabel
     OpBranch %49

     %50 = OpLabel
     OpSelectionMerge %79 None
     OpBranchConditional %cond %60 %99 ; break-unless

     %79 = OpLabel
     OpBranch %99

     %60 = OpLabel
     OpBranch %79

     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 40, 49, 50, 60, 79, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_SingleBlock_Simple) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     ; The entry block can't be the target of a branch
     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 99)) << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_SingleBlock_Infinite) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     ; The entry block can't be the target of a branch
     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 99)) << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_SingleBlock_DupInfinite) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     ; The entry block can't be the target of a branch
     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 99)) << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_HeaderHasBreakIf) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99 ; like While

     %30 = OpLabel ; trivial body
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 50, 99)) << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_HeaderHasBreakUnless) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %99 %30 ; has break-unless

     %30 = OpLabel ; trivial body
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 50, 99)) << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_BodyHasBreak) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %99 ; break

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 50, 99)) << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_BodyHasBreakIf) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranchConditional %cond2 %99 %40 ; break-if

     %40 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 40, 50, 99)) << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_BodyHasBreakUnless) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranchConditional %cond2 %40 %99 ; break-unless

     %40 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 40, 50, 99)) << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Body_If) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond2 %40 %45 ; nested if

     %40 = OpLabel
     OpBranch %49

     %45 = OpLabel
     OpBranch %49

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 40, 45, 49, 50, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Body_If_Break) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond2 %40 %49 ; nested if

     %40 = OpLabel
     OpBranch %99   ; break from nested if

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 40, 49, 50, 99)) << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_BodyHasContinueIf) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranchConditional %cond2 %50 %40 ; continue-if

     %40 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 40, 50, 99)) << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_BodyHasContinueUnless) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranchConditional %cond2 %40 %50 ; continue-unless

     %40 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 40, 50, 99)) << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Body_If_Continue) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond2 %40 %49 ; nested if

     %40 = OpLabel
     OpBranch %50   ; continue from nested if

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 40, 49, 50, 99)) << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Body_Switch) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %49 None
     OpSwitch %selector %49 40 %40 45 %45 ; fully nested switch

     %40 = OpLabel
     OpBranch %49

     %45 = OpLabel
     OpBranch %49

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 45, 40, 49, 50, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Body_Switch_CaseBreaks) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %49 None
     OpSwitch %selector %49 40 %40 45 %45

     %40 = OpLabel
     ; This case breaks out of the loop. This is not possible in C
     ; because "break" will escape the switch only.
     OpBranch %99

     %45 = OpLabel
     OpBranch %49

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 45, 40, 49, 50, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Body_Switch_CaseContinues) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %49 None
     OpSwitch %selector %49 40 %40 45 %45

     %40 = OpLabel
     OpBranch %50   ; continue bypasses switch merge

     %45 = OpLabel
     OpBranch %49

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 45, 40, 49, 50, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_BodyHasSwitchContinueBreak) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSwitch %selector %99 50 %50 ; default is break, 50 is continue

     %40 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 50, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Continue_Sequence) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %60

     %60 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 50, 60, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Continue_ContainsIf) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpSelectionMerge %89 None
     OpBranchConditional %cond2 %60 %70

     %89 = OpLabel
     OpBranch %20 ; backedge

     %60 = OpLabel
     OpBranch %89

     %70 = OpLabel
     OpBranch %89

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 50, 60, 70, 89, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Continue_HasBreakIf) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranchConditional %cond2 %99 %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 50, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Continue_HasBreakUnless) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranchConditional %cond2 %20 %99

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 50, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Continue_SwitchBreak) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpSwitch %selector %20 99 %99 ; yes, this is obtuse but valid

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 50, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Loop) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond2 %35 %49

     %35 = OpLabel
     OpBranch %37

     %37 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner loop's continue
     OpBranch %30 ; backedge

     %49 = OpLabel ; inner loop's merge
     OpBranch %50

     %50 = OpLabel ; outer loop's continue
     OpBranch %20 ; outer loop's backege

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 35, 37, 40, 49, 50, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Loop_InnerBreak) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond2 %35 %49

     %35 = OpLabel
     OpBranchConditional %cond3 %49 %37 ; break to inner merge

     %37 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner loop's continue
     OpBranch %30 ; backedge

     %49 = OpLabel ; inner loop's merge
     OpBranch %50

     %50 = OpLabel ; outer loop's continue
     OpBranch %20 ; outer loop's backege

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 35, 37, 40, 49, 50, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Loop_InnerContinue) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond2 %35 %49

     %35 = OpLabel
     OpBranchConditional %cond3 %37 %49 ; continue to inner continue target

     %37 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner loop's continue
     OpBranch %30 ; backedge

     %49 = OpLabel ; inner loop's merge
     OpBranch %50

     %50 = OpLabel ; outer loop's continue
     OpBranch %20 ; outer loop's backege

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 35, 37, 40, 49, 50, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Loop_InnerContinueBreaks) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond2 %35 %49

     %35 = OpLabel
     OpBranch %37

     %37 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner loop's continue
     OpBranchConditional %cond3 %30 %49 ; backedge and inner break

     %49 = OpLabel ; inner loop's merge
     OpBranch %50

     %50 = OpLabel ; outer loop's continue
     OpBranch %20 ; outer loop's backege

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 35, 37, 40, 49, 50, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Loop_InnerContinueContinues) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond2 %35 %49

     %35 = OpLabel
     OpBranch %37

     %37 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner loop's continue
     OpBranchConditional %cond3 %30 %50 ; backedge and continue to outer

     %49 = OpLabel ; inner loop's merge
     OpBranch %50

     %50 = OpLabel ; outer loop's continue
     OpBranch %20 ; outer loop's backege

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 35, 37, 40, 49, 50, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Loop_SwitchBackedgeBreakContinue) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond2 %35 %49

     %35 = OpLabel
     OpBranch %37

     %37 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner loop's continue
     ; This switch does triple duty:
     ; default -> backedge
     ; 49 -> loop break
     ; 49 -> inner loop break
     ; 50 -> outer loop continue
     OpSwitch %selector %30 49 %49 50 %50

     %49 = OpLabel ; inner loop's merge
     OpBranch %50

     %50 = OpLabel ; outer loop's continue
     OpBranch %20 ; outer loop's backege

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.rspo(), ElementsAre(10, 20, 30, 35, 37, 40, 49, 50, 99));
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint