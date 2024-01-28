#include "cxqubo/cxqubo.h"
#include <iostream>

using namespace cxqubo;

int main() {
  Context context;
  CXQUBOModel model(context);
  auto x = model.add_vars({1}, Vartype::BINARY, "x");
  auto y = model.add_vars({2, 2}, Vartype::BINARY, "y");
  auto z = model.add_vars({2, 2, 2}, Vartype::BINARY, "z");

  auto onehot =
      constraint((x[0] + y[0][0] + y[0][1] + y[1][0] + y[1][1] + z[0][0][0] +
                  z[0][0][1] + z[0][1][0] + z[0][1][1] + z[1][0][0] +
                  z[1][0][1] + z[1][1][0] + z[1][1][1] - 1.0)
                     .pow(2),
                 "onehot");
  auto h = x[0];
  auto H = onehot + h;
  std::cout << H.as_tree() << '\n';

  auto compiled = model.compile(H);
  std::cout << model.decode(compiled) << '\n';

  auto [qubo, offset] = model.create_qubo(compiled);
  std::cout << model.decode(qubo) << '\n';
}
