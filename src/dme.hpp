#include "parser.hpp"
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
      : std::pair<ManhattanPt, ManhattanPt>(a, b) {
    if ( second.x < first.x ) std::swap(first, second);    
  }

  int64_t getSlope() const {
    auto dx = second.x - first.x;
    auto dy = second.y - first.y;
    if (dx == 0) {
      return std::numeric_limits<int64_t>::max();
    }
    return dy / dx;
  }

  bool isActuallyPoint() const { return first == second; }

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

  // @FIXME: brute forcing for now
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

// @FIXME: brute forcing here as well
inline pt_t closestOnSegment(pt_t src, seg_t target) {
  auto slopePt = pt_t {.x = 1, .y = target.getSlope()};
  auto mnDist = std::numeric_limits<int64_t>::max();
  pt_t retAns;
  for ( pt_t x = target.first; x != target.second; x = x + slopePt ) {
    auto dist = manhattanDistance(src, x);
    if ( dist < mnDist ) {
      mnDist = dist;
      retAns = x;
    }
  }
  return retAns;
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
    if (l.isActuallyPoint() && r.isActuallyPoint()) {
      if (l.first == r.first) {
        return {{l.first, l.first}};
      }
      return {};
    }

    if (r.isActuallyPoint()) {
      std::swap(l, r);
    }

    if (l.isActuallyPoint()) {
      auto [l1, _] = l;
      auto [r1, r2] = r;
      if (l1 == r1 || l1 == r2) {
        return {{l1, l1}};
      }
      if (seg_t(l1, r1).getSlope() == seg_t(l1, r2).getSlope()) {
        if (r1.x > r2.x)
          std::swap(r1, r2);
        if (l1.x >= r1.x && l1.x <= r2.x) {
          return {{l1, l1}};
        }
      }
      return {};
    }

    if (l.getSlope() != r.getSlope()) {
      return {};
    }
    auto [l1, l2] = l;
    auto [r1, r2] = r;
    auto refSlope = l.getSlope();
    auto checkSlopeCondition = [](auto &&l, auto &&r, auto &&slope) {
      return l == r || seg_t(l, r).getSlope() == slope;
    };

    if (checkSlopeCondition(l1, r1, refSlope) &&
        checkSlopeCondition(l1, r2, refSlope)) {
      if (r1.x > r2.x)
        std::swap(r1, r2);
      if (l1.x > l2.x)
        std::swap(l1, l2);
      auto p1 = l1.x > r1.x ? l1 : r1;
      auto p2 = l2.x > r2.x ? r2 : l2;
      // no intersection
      if (r2.x < l1.x || r1.x > l2.x) {
        return {};
      }
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
  double LdCap = 0;
  double Delay = 0;

  std::string str() const;
};

inline std::string DMENode::str() const {
  std::ostringstream oss;
  oss << "DMENode=[" << Core.str() << ", "
      << "LdCap=" << LdCap << ", "
      << "Delay=" << Delay << "]";
  return oss.str();
}

inline DMENode merge(const DMENode &lhs, const DMENode &rhs, wire wr) {
  auto d = coreDistance(lhs.Core, rhs.Core);
  if (d == 0) {
    LogError("Intersecting cores. This won't end well!");
  }

  auto del1 = lhs.Delay, del2 = rhs.Delay, c1 = lhs.LdCap, c2 = rhs.LdCap;

  double eaDbl =
      (double)((del2 - del1) + (double)(d * d * wr.resistance * wr.cap) / 2 +
               d * wr.resistance * c2) /
      (wr.resistance * (c1 + c2 + d));

  eaDbl = std::max(eaDbl, 0.);
  eaDbl = std::min(eaDbl, (double)d);

  int64_t ea = static_cast<int64_t>(eaDbl);
  auto eb = d - ea;

  auto trrLhs = DMETiledRegion(lhs.Core, ea);
  auto trrRhs = DMETiledRegion(rhs.Core, eb);

  auto intersection = getTRRIntersection(trrLhs, trrRhs);
  if (!intersection.has_value()) {
    LogError("No TRR intersection. Something is wrong!");
    // can't recover
    std::terminate();
  }

  auto delay1 = del1 + ea * wr.resistance * ((double)ea * wr.cap / 2 + c1);
  auto delay2 = del2 + eb * wr.resistance * ((double)eb * wr.cap / 2 + c2);

  return DMENode{.Core = intersection.value(),
                 .Delay = std::max(delay1, delay2),
                 .LdCap = lhs.LdCap + rhs.LdCap + d * wr.cap};
}

struct EmbeddingResult {};

struct EmbeddingManager {
  EmbeddingManager(inparams, clksyn::TopologyResult);

  void computeEmbedding();

private:
  void dfs(int32_t nodeIdx, int32_t parentIdx);

  inparams inp_;
  wire wire_;
  clksyn::TopologyResult topology_;
  std::vector<std::vector<int32_t>> adj_;
  std::vector<clksyn::TreeNode> topoNodes_;
  std::vector<DMENode> nodes_;
};

inline EmbeddingManager::EmbeddingManager(inparams inp,
                                          clksyn::TopologyResult res)
    : inp_(inp), topology_(res) {
  // @TODO
  // going to use a random wire for now
  // ideally we may want to pick the one that gives least delay
  wire_ = inp_.wires.back();
  LogInfo("Using wire type=" + wire_.type);

  adj_.resize(res.Nodes.size() + 1);
  for (const auto &edge : res.Edges) {
    adj_[edge.first].push_back(edge.second);
    adj_[edge.second].push_back(edge.first);
  }

  topoNodes_.resize(res.Nodes.size() + 1);
  for (const auto &node : res.Nodes) {
    topoNodes_[node.Idx] = node;
  }

  nodes_.resize(res.Nodes.size() + 1);

  for (size_t i = 0; i < adj_.size(); ++i) {
    std::cout << i << " " << adj_[i].size() << ": ";
    for (auto j : adj_[i]) {
      std::cout << j << ' ';
    }
    std::cout << std::endl;
  }
}

inline void EmbeddingManager::dfs(int32_t nodeIdx, int32_t parentIdx) {
  int32_t kidOne = -1, kidTwo = -1;

  for (const auto &idx : adj_[nodeIdx]) {
    if (idx == parentIdx) {
      continue;
    }
    kidTwo = kidOne;
    kidOne = idx;
    dfs(idx, nodeIdx);
  }

  if (kidOne == -1 && kidTwo != -1) {
    // single child
    LogError("Unexpected single child in binary tree.");
  }

  auto &topoNode = topoNodes_[nodeIdx];
  if (kidOne == -1) {
    // no kids, leaf node
    nodes_[nodeIdx] = DMENode{
        .Core = DMECore{.Kind = DMECore::POINT,
                        .Loc = pt_t{.x = topoNode.x, .y = topoNode.y}},
        .Delay = 0,
        .LdCap = topoNode.LdCap,
    };
  } else {
    nodes_[nodeIdx] = merge(nodes_[kidOne], nodes_[kidTwo], wire_);
  }
}

inline void EmbeddingManager::computeEmbedding() {
  // 0 is SRC
  auto root = adj_[0].back();
  std::cout << root << std::endl;
  dfs(root, 0);

  // @TODO: return stuff in proper format
  for (size_t i = 1; i < nodes_.size(); ++i) {
    std::cout << i << " ";
    std::cout << nodes_[i].str() << std::endl;
  }
}

} // namespace dme
