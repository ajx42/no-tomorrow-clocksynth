#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <utils/catch.hpp>
#include "topology.hpp"

TEST_CASE( "Topology::NodePair Comparison Test", "[nodepaircomp]" ) {
  clksyn::NodePair lhs {.Cost = 20, .A = {}, .B = {}};
  clksyn::NodePair rhs {.Cost = 30, .A = {}, .B = {}};
  
  auto mn = lhs < rhs ? lhs : rhs;
  
  // higher cost NodePair is considered lesser
  REQUIRE( mn == rhs );
}

