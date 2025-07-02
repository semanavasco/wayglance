#include "managers/client.hpp"
#include <cstdlib>
#include <spdlog/spdlog.h>

int main(int argc, char *argv[]) {
  try {
    auto &client = wayglance::managers::Client::inst();

    return client.run(argc, argv);
  } catch (const std::exception &e) {
    spdlog::error("{}", e.what());
    return EXIT_FAILURE;
  }
}
