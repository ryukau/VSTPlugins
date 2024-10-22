// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

namespace SomeDSP {

namespace FeedbackMatrixType {
enum FeedbackMatrixType : unsigned {
  orthogonal,
  specialOrthogonal,
  circulantOrthogonal,
  circulant4,
  circulant8,
  circulant16,
  circulant32,
  upperTriangularPositive,
  upperTriangularNegative,
  lowerTriangularPositive,
  lowerTriangularNegative,
  schroederPositive,
  schroederNegative,
  absorbentPositive,
  absorbentNegative,
  hadamard,
  conference,
  FeedbackMatrixType_ENUM_LENGTH,
};
} // namespace FeedbackMatrixType

} // namespace SomeDSP
