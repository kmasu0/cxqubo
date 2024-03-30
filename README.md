# cxqubo

cxqubo is header only C++ QUBO generator inspired from [PyQUBO](https://github.com/recruit-communications/pyqubo).

## Example

```c++
#include "cxqubo/cxqubo.h"
#include <memory>

int main() {
    // Create context.
    auto cxq = std::make_unique<cxqubo::CXQUBOModel>();

    // Build expressions.
    auto x = cxq->add_binary("x");
    auto y = cxq->add_binary("y");
    auto h = (x + y).pow(2);

    // Compile AST to polynomial.
    auto compiled = cxq->compile(h);

    // Get QUBO.
    auto [qubo, offset] = cxq->create_qubo(compiled);

    // Sampling.
    // ...

    // All objects are deleted when CXQUBOModel is destructed.
    return 0;
}
```

See [[tsp.cpp]](./examples/tsp.cpp) for details.

## Build and install

```sh
git clone https://github.com/kmasu0/cxqubo
cd cxqubo
./configure.sh
./build-and-install.sh
```

If you want to change install destination, change configure.sh.

## Dependency

cxqubo depends on the following projects.

* [GoogleTest](https://github.com/google/googletest)
* [cimod](https://github.com/OpenJij/cimod)
* [fmt](https://github.com/fmtlib/fmt)

And some implementation is inspired by the following projects.

* [PyQUBO](https://github.com/recruit-communications/pyqubo)
* [Boost](https://github.com/boostorg/boost)
* [LLVM](https://github.com/llvm/llvm-project)

## License

Released under the Apache License 2.0.

