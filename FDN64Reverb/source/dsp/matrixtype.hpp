// (c) 2022 Takamitsu Endo
//
// This file is part of FDN64Reverb.
//
// FDN64Reverb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FDN64Reverb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FDN64Reverb.  If not, see <https://www.gnu.org/licenses/>.

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
