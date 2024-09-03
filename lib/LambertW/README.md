# LambertW (Header Only Version)
`LambertW.hpp` is obtained from [issue #5](https://github.com/DarkoVeberic/LambertW/issues/5) of [the original repository](https://github.com/DarkoVeberic/LambertW). The code is further modified to replace `M_E` to `std::numbers::e_v`, and to avoid duplication of definition across compilation units.

## Usage
```c++
double utl::LambertW(const int branch, const double x);       // Main implementaion.
double Fukushima::LambertW(const int branch, const double x); // Reference implementation.
```

- `x > -1 / e` for branch 0.
- `x > -1 / e && x < 0` for branch -1.
- Returns `std::numeric_limits<double>::quiet_NaN()` if `branch` is not 0 or -1.
- `Fukushima` implementaion calls `std::cerr` in case of numeric error.
- `Fukushima` implementation is supporsed to be slower.

## License
The two-clause BSD license. Below is the license text obtaned from [`LICENSE-BSD-2C.txt` in the original repository](https://raw.githubusercontent.com/DarkoVeberic/LambertW/master/LICENSE-BSD-2C.txt).

```
Copyright (c) 2014 Darko Veberic
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

## Links
- [GitHub - DarkoVeberic/LambertW: C++ implementation of the Lambert W(x) function](https://github.com/DarkoVeberic/LambertW)
- [Lambert W function - develop](https://www.boost.org/doc/libs/develop/libs/math/doc/html/math_toolkit/lambert_w.html)
  - Boost also provides LambertW.
