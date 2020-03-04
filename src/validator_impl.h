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

#ifndef SRC_VALIDATOR_IMPL_H_
#define SRC_VALIDATOR_IMPL_H_

#include <string>

#include "src/ast/module.h"

namespace tint {

/// Determines if the module is complete and valid
class ValidatorImpl {
 public:
  /// Constructor
  ValidatorImpl();
  ~ValidatorImpl();

  /// Runs the validator
  /// @param module the module to validate
  /// @returns true if the validation was successful
  bool Validate(const ast::Module& module);

  /// @returns error messages from the validator
  const std::string& error() { return error_; }

  /// @returns true if an error was encountered
  bool has_error() const { return error_.size() > 0; }

  /// Sets the error string
  /// @param src the source causing the error
  /// @param msg the error message
  void set_error(const Source& src, const std::string& msg);

  /// Validates v-0001: Only allowed import is "GLSL.std.450"
  /// @param module the modele to check imports
  /// @returns ture if input complies with v-0001 rule
  bool CheckImports(const ast::Module& module);

 private:
  std::string error_;
};

}  // namespace tint

#endif  // SRC_VALIDATOR_IMPL_H_