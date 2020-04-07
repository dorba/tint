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

#ifndef SRC_READER_SPIRV_FUNCTION_H_
#define SRC_READER_SPIRV_FUNCTION_H_

#include "source/opt/function.h"
#include "source/opt/ir_context.h"
#include "src/ast/module.h"
#include "src/reader/spirv/fail_stream.h"
#include "src/reader/spirv/namer.h"
#include "src/reader/spirv/parser_impl.h"

namespace tint {
namespace reader {
namespace spirv {

/// A FunctionEmitter emits a SPIR-V function onto a Tint AST module.
class FunctionEmitter {
 public:
  /// Creates a FunctionEmitter, and prepares to write to the AST module
  /// in |pi|.
  /// @param pi a ParserImpl which has already executed BuildInternalModule
  /// @param function the function to emit
  FunctionEmitter(ParserImpl* pi, const spvtools::opt::Function& function);
  /// Destructor
  ~FunctionEmitter();

  /// Emits the function to AST module.
  /// @return whether emission succeeded
  bool Emit();

  /// @returns true if emission has not yet failed.
  bool success() const { return fail_stream_.status(); }
  /// @returns true if emission has failed.
  bool failed() const { return !success(); }

  /// Records failure.
  /// @returns a FailStream on which to emit diagnostics.
  FailStream& Fail() { return fail_stream_.Fail(); }

  /// Emits the declaration, which comprises the name, parameters, and
  /// return type. The function AST node is appended to the module
  /// AST node.
  /// @returns true if emission has not yet failed.
  bool EmitFunctionDeclaration();

 private:
  ParserImpl& parser_impl_;
  ast::Module& ast_module_;
  spvtools::opt::IRContext& ir_context_;
  FailStream& fail_stream_;
  Namer& namer_;
  const spvtools::opt::Function& function_;
  std::vector<std::unique_ptr<ast::Statement>> ast_body_;
};

}  // namespace spirv
}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_SPIRV_FUNCTION_H_