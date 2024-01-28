#include "cxqubo/cxqubo.h"
#include <memory>

static void example1();
static void example2();
static void example3();
static void example4();

using namespace cxqubo;

int main() {
  example1();
  example2();
  example3();
  example4();
  return 0;
}

static void example1() {
  std::cout << "<example1>\n";

  Context context;
  CXQUBOModel model(context);
  auto x = model.add_binary("x");
  auto y = model.add_binary("y");
  auto h = (x + y) * (x + y);

  std::cout << h << '\n';
  std::cout << h.as_tree() << '\n';

  const auto compiled = model.compile(h);
  auto [qubo, offset] = model.create_qubo(compiled);

  std::cout << model.decode(qubo) << '\n';
}

static void example2() {
  std::cout << "<example2>\n";

  Context context;
  CXQUBOModel model(context);
  auto s0 = model.add_spin("s0");
  auto s1 = model.add_spin("s1");

  auto h = (s0 + s1).pow(2);
  std::cout << h << '\n';
  std::cout << h.as_tree() << '\n';

  const auto compiled = model.compile(h);
  auto [qubo, offset] = model.create_qubo(compiled);

  std::cout << model.decode(qubo) << '\n';
}

static void example3() {
  std::cout << "<example3>\n";

  Context context;
  CXQUBOModel model(context);
  auto s0 = model.add_spin("s0");
  auto s1 = model.add_spin("s1");
  auto s2 = model.add_spin("s2");
  auto s3 = model.add_spin("s3");

  auto h = (4 * s0 + 2 * s1 + 7 * s2 + s3).pow(2);
  std::cout << h << '\n';
  std::cout << h.as_tree() << '\n';

  const auto compiled = model.compile(h);
  auto [qubo, offset] = model.create_qubo(compiled);

  std::cout << model.decode(qubo) << '\n';
}

static void example4() {
  std::cout << "<example4>\n";

  Context context;
  CXQUBOModel model(context);
  auto xs = model.add_vars({3, 4, 5}, Vartype::BINARY, "x");
  std::cout << xs << '\n';

  for (auto x : xs) {
    for (auto x2 : x) {
      for (auto x3 : x2) {
        std::cout << *x3 << '\n';
      }
    }
  }
}
