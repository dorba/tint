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

#ifndef SRC_AST_STRIDE_DECORATION_H_
#define SRC_AST_STRIDE_DECORATION_H_

#include <stddef.h>

#include <string>

#include "src/ast/array_decoration.h"

namespace tint {
namespace ast {

/// A stride decoration
class StrideDecoration : public ArrayDecoration {
 public:
  /// constructor
  /// @param stride the stride value
  explicit StrideDecoration(uint32_t stride);
  ~StrideDecoration() override;

  /// @returns true if this is a stride decoration
  bool IsStride() const override;

  /// @returns the stride value
  uint32_t stride() const { return stride_; }

  /// @returns the decoration as a string
  std::string to_str() const override;

 private:
  uint32_t stride_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_STRIDE_DECORATION_H_
