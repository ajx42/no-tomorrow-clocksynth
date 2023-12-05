#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <vector>
#include <algorithm>

#include <utils/catch.hpp>
#include "topology.hpp"
#include "dme.hpp"

using namespace clksyn;

TEST_CASE( "Topology::NodePair Comparison Test", "[nodepaircomp]" ) {
  NodePair lhs {.Cost = 20, .A = {}, .B = {}};
  NodePair rhs {.Cost = 30, .A = {}, .B = {}};
  
  auto mn = lhs < rhs ? lhs : rhs;
  
  // higher cost NodePair is considered lesser
  REQUIRE( mn == rhs );
}

TEST_CASE( "DME::DMENode TRR Tests", "[dme]" ) {

  auto core = dme::DMECore {
    .Kind = dme::DMECore::POINT,
    .Loc = dme::pt_t {.x = 10, .y = 50},
  };

  auto trr = dme::DMETiledRegion( core, 20 );

  REQUIRE( trr.Up ==  dme::pt_t {.x = 10, .y = 70} );
  REQUIRE( trr.Down == dme::pt_t {.x = 10, .y = 30} );
  REQUIRE( trr.Left == dme::pt_t {.x = -10, .y = 50} );
  REQUIRE( trr.Right == dme::pt_t {.x = 30, .y = 50} );
}

TEST_CASE( "DME::manhattanDistance Test", "[dme]" ) {
  auto seg1 = dme::seg_t {{.x = 0, .y = 0}, {.x = 5, .y = 5}};
  auto seg2 = dme::seg_t {{.x = 2, .y = 3}, {.x = 8, .y = 3}};
  auto res = dme::manhattanDistance( seg1, seg2 );
  REQUIRE( res == 1 );
}

TEST_CASE( "DME::DMETiledRegion Intersection Test", "[dme]" ) {
  auto core1 = dme::DMECore {
    .Kind = dme::DMECore::SEGMENT,
    .Loc = dme::seg_t {{.x = 0, .y = 0}, {.x = 5, .y = 5}}
  };

  auto reg1 = dme::DMETiledRegion( core1, 2 );

  auto core2 = dme::DMECore {
    .Kind = dme::DMECore::SEGMENT,
    .Loc = dme::seg_t {{.x = 5, .y = 0}, {.x = 15, .y = 10}}
  };

  auto reg2 = dme::DMETiledRegion( core2, 3 );
  
  auto intersection = dme::getTRRIntersection(reg1, reg2);
  REQUIRE( intersection.has_value() );

  auto coreR = dme::DMECore {
    .Kind = dme::DMECore::SEGMENT,
    .Loc = dme::seg_t {{.x = 2, .y = 0}, {.x = 7, .y = 5}}
  };

  REQUIRE( intersection.value() == coreR );
}


TEST_CASE( "DME::DMETiledRegion Intersection Test 0 Radius Segment Core", "[dme]" ) {
  auto core1 = dme::DMECore {
    .Kind = dme::DMECore::SEGMENT,
    .Loc = dme::seg_t {{.x = 0, .y = 0}, {.x = 5, .y = 5}}
  };

  auto reg1 = dme::DMETiledRegion( core1, 0 );

  auto core2 = dme::DMECore {
    .Kind = dme::DMECore::SEGMENT,
    .Loc = dme::seg_t {{.x = 5, .y = 0}, {.x = 15, .y = 10}}
  };

  auto reg2 = dme::DMETiledRegion( core2, 5 );
  
  auto intersection = dme::getTRRIntersection(reg1, reg2);
  REQUIRE( intersection.has_value() );

  auto coreR = dme::DMECore {
    .Kind = dme::DMECore::SEGMENT,
    .Loc = dme::seg_t {{.x = 0, .y = 0}, {.x = 5, .y = 5}}
  };

  REQUIRE( intersection.value() == coreR );
}

TEST_CASE( "DME::DMETiledRegion Intersection Test 0 Radius Point Core", "[dme]" ) {
  auto core1 = dme::DMECore {
    .Kind = dme::DMECore::POINT,
    .Loc = dme::pt_t {.x = 0, .y = 0}
  };

  auto reg1 = dme::DMETiledRegion( core1, 0 );

  auto core2 = dme::DMECore {
    .Kind = dme::DMECore::SEGMENT,
    .Loc = dme::seg_t {{.x = 5, .y = 0}, {.x = 15, .y = 10}}
  };

  auto reg2 = dme::DMETiledRegion( core2, 5 );
  
  auto intersection = dme::getTRRIntersection(reg1, reg2);
  REQUIRE( intersection.has_value() );

  auto coreR = dme::DMECore {
    .Kind = dme::DMECore::POINT,
    .Loc = dme::pt_t{.x = 0, .y = 0},
  };

  REQUIRE( intersection.value() == coreR );
}
