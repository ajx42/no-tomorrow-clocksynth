#include <argparse/argparse.hpp>
#include "WowLogger.H"

#include <iostream>

int main() {
  std::cout << "hello world" << std::endl;
  LogInfo("Testing Logger");
  return 0;
}
