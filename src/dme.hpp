#include "topology.hpp"
#include <algorithm>
#include <map>
#include <optional>
#include <variant>
#include <vector>

namespace dme {

struct ManhattanPt {
  int64_t x, y;
  auto operator<=>(const ManhattanPt &) const = default;

  ManhattanPt operator-() const;
  ManhattanPt operator+(const ManhattanPt &rhs) const;
  ManhattanPt operator-(const ManhattanPt &rhs) const;

  std::string str() const {
    std::ostringstream oss;
    oss << "Point=(" << x << ", " << y << ")";
    return oss.str();
  }
};

inline ManhattanPt ManhattanPt::operator-() const {
  return ManhattanPt{.x = -x, .y = -y};
}

inline ManhattanPt ManhattanPt::operator+(const ManhattanPt &rhs) const {
  return ManhattanPt{.x = x + rhs.x, .y = y + rhs.y};
}

inline ManhattanPt ManhattanPt::operator-(const ManhattanPt &rhs) const {
  return (*this) + (-rhs);
}

using pt_t = ManhattanPt;

struct ManhattanSeg : std::pair<ManhattanPt, ManhattanPt> {
  ManhattanSeg(ManhattanPt a, ManhattanPt b)
      : std::pair<ManhattanPt, ManhattanPt>(a, b) {}

  int64_t getSlope() const {
    auto dx = second.x - first.x;
    auto dy = second.y - first.y;
    if (dx == 0) {
      return std::numeric_limits<int64_t>::max();
    }
    return dy / dx;
  }

  std::string str() const {
    std::ostringstream oss;
    oss << "Segment:[" << first.str() << ", " << second.str() << "]";
    return oss.str();
  }
};

using seg_t = ManhattanSeg;

inline int64_t manhattanDistance(pt_t a, pt_t b) {
  return abs(a.x - b.x) + abs(a.y - b.y);
}

inline int64_t manhattanDistance(pt_t a, seg_t b) {
  auto dx = b.first.x - b.second.x;
  auto dy = b.first.y - b.second.y;
  if (dx == 0 && dy == 0) {
    return manhattanDistance(a, b.first);
  }
  auto slopePt = pt_t{.x = 1, .y = dy / dx};

  // brute forcing for now
  // really bored of this and moving on
  auto ans = std::numeric_limits<int64_t>::max();
  for (pt_t x = b.first; x != b.second; x = x + slopePt) {
    ans = std::min(ans, manhattanDistance(a, x));
  }

  return ans;
}

inline int64_t manhattanDistance(seg_t a, seg_t b) {
  auto ans = std::numeric_limits<int64_t>::max();
  ans = std::min(ans, manhattanDistance(a.first, b));
  ans = std::min(ans, manhattanDistance(a.second, b));
  ans = std::min(ans, manhattanDistance(b.first, a));
  ans = std::min(ans, manhattanDistance(b.second, a));
  return ans;
}

struct DMECore {
  enum { POINT, SEGMENT } Kind;
  std::variant<pt_t, seg_t> Loc;

  auto operator==(const DMECore &rhs) const {
    if (Kind != rhs.Kind)
      return false;
    if (Kind == DMECore::POINT) {
      return std::get<pt_t>(Loc) == std::get<pt_t>(rhs.Loc);
    } else {
      return std::get<seg_t>(Loc) == std::get<seg_t>(rhs.Loc);
    }
  }

  std::string str() const {
    return Kind == POINT ? ("Core={" + std::get<pt_t>(Loc).str() + "}")
                         : ("Core={" + std::get<seg_t>(Loc).str() + "}");
  }
};

inline int64_t coreDistance(DMECore lhs, DMECore rhs) {
  if (lhs.Kind == DMECore::POINT && rhs.Kind == DMECore::POINT) {
    return manhattanDistance(std::get<pt_t>(lhs.Loc), std::get<pt_t>(rhs.Loc));
  } else if (lhs.Kind == DMECore::POINT && rhs.Kind == DMECore::SEGMENT) {
    return manhattanDistance(std::get<pt_t>(lhs.Loc), std::get<seg_t>(rhs.Loc));
  } else if (lhs.Kind == DMECore::SEGMENT && rhs.Kind == DMECore::POINT) {
    return manhattanDistance(std::get<pt_t>(rhs.Loc), std::get<seg_t>(lhs.Loc));
  } else {
    return manhattanDistance(std::get<seg_t>(lhs.Loc),
                             std::get<seg_t>(rhs.Loc));
  }
}

struct DMETiledRegion {
  DMETiledRegion(DMECore core, int64_t dist);

  DMECore Core;
  int64_t Radius;
  pt_t Left, Right, Up, Down;
};

inline DMETiledRegion::DMETiledRegion(DMECore core, int64_t dist)
    : Core(core), Radius(dist) {
  auto moveY = pt_t{.x = 0, .y = dist};
  auto moveX = pt_t{.x = dist, .y = 0};

  if (Core.Kind == DMECore::POINT) {
    auto pt = std::get<pt_t>(Core.Loc);
    Right = pt + moveX;
    Left = pt - moveX;
    Up = pt + moveY;
    Down = pt - moveY;
  } else {
    auto [ptA, ptB] = std::get<seg_t>(Core.Loc);
    Up = (ptA.y > ptB.y ? ptA : ptB) + moveY;
    Down = (ptA.y < ptB.y ? ptA : ptB) - moveY;
    Right = (ptA.x > ptB.x ? ptA : ptB) + moveX;
    Left = (ptA.x < ptB.x ? ptA : ptB) - moveX;
  }
}

inline std::optional<DMECore> getTRRIntersection(DMETiledRegion regA,
                                                 DMETiledRegion regB) {
  auto segmentIntersection = [](seg_t l, seg_t r) -> std::optional<seg_t> {
    if (l.getSlope() != r.getSlope()) {
      return {};
    }
    auto [l1, l2] = l;
    auto [r1, r2] = r;
    auto refSlope = l.getSlope();
    if (seg_t(l1, r1).getSlope() == refSlope &&
        seg_t(l1, r2).getSlope() == refSlope) {
      if (r1.x > r2.x)
        std::swap(r1, r2);
      if (l1.x > l2.x)
        std::swap(l1, l2);
      auto p1 = l1.x > r1.x ? l1 : r1;
      auto p2 = l2.x > r2.x ? r2 : l2;
      return {{p1, p2}};
    }
    return {};
  };

  std::vector<seg_t> regASegs = {
      {regA.Right, regA.Down},
      {regA.Up, regA.Right},
      {regA.Left, regA.Up},
      {regA.Down, regA.Left},
  };

  std::vector<seg_t> regBSegs = {
      {regB.Right, regB.Down},
      {regB.Up, regB.Right},
      {regB.Left, regB.Up},
      {regB.Down, regB.Left},
  };

  for (auto segA : regASegs) {
    for (auto segB : regBSegs) {
      // @FIXME : remove
      //      std::cout << segA.str() << " " << segB.str() << std::endl;
      auto res = segmentIntersection(segA, segB);
      if (!res.has_value()) {
        continue;
      }
      auto resVal = res.value();
      if (resVal.first == resVal.second) {
        return {DMECore{
            .Kind = DMECore::POINT,
            .Loc = resVal.first,
        }};
      } else {
        return {DMECore{
            .Kind = DMECore::SEGMENT,
            .Loc = resVal,
        }};
      }
    }
  }
  return {};
}

struct DMENode {
  DMECore Core;
  int64_t LdCap = 0;
  double Delay = 0;
};

} // namespace dme
