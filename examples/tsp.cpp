#include "cxqubo/cxqubo.h"
#include "cxqubo/misc/containers.h"
#include <cstdlib>
#include <ctime>
#include <memory>
#include <sys/types.h>
#include <unistd.h>

using namespace cxqubo;

double measured_time(clock_t start, clock_t end) {
  return static_cast<double>(end - start) / CLOCKS_PER_SEC * 1000.0;
}

int print_memusage() {
  char command[128];
  sprintf(command, "grep VmHWM /proc/%d/status", getpid());
  return system(command);
}

void tsp(unsigned ncity) {
  Context ctx;
  CXQUBOModel model(ctx);

  auto t0 = clock();
  auto x = model.add_vars({ncity, ncity}, Vartype::BINARY, "x");
  auto H = model.fp(0.0);

  for (unsigned i : irange(ncity)) {
    auto h = accumulate(irange(ncity), model.fp(0.0),
                        [&x, i](auto v, unsigned j) { return v + x[i][j]; });
    H += constraint((h - 1.0).pow(2) == 0.0, "time" + std::to_string(i));
  }

  for (unsigned j : irange(ncity)) {
    auto h = accumulate(irange(ncity), model.fp(0.0),
                        [&x, j](auto v, unsigned i) { return v + x[i][j]; });
    H += constraint((h - 1.0).pow(2) == 0.0, "city" + std::to_string(j));
  }

  for (unsigned i : irange(ncity))
    for (unsigned j : irange(ncity))
      for (unsigned k : irange(ncity))
        H += 10 * x[k][i] * x[(k + 1) % ncity][j];

  // std::cout << H.as_tree() << '\n';
  FeedDict feed_dict = {{"A", 1.0}};

  auto t1 = clock();
  auto compiled = model.compile(H);
  auto t2 = clock();
  auto [qubo, offset] = model.create_qubo(compiled, feed_dict);
  auto t3 = clock();

  print_memusage();

  std::cout << "len(qubo): " << qubo.size() << '\n';
  std::cout << "-- times (msec)" << '\n';
  std::cout << "total: " << std::to_string(measured_time(t0, t3)) << '\n';
  std::cout << "express: " << std::to_string(measured_time(t0, t1)) << '\n';
  std::cout << "compile: " << std::to_string(measured_time(t1, t2)) << '\n';
  std::cout << "qubo: " << std::to_string(measured_time(t2, t3)) << '\n';
}

int main(int argc, const char **argv) {
  unsigned ncity = 20;
  if (argc == 2) {
    ncity = atoi(argv[1]);
  }

  std::cout << "ncity = " << ncity << '\n';
  tsp(ncity);

  return 0;
}
