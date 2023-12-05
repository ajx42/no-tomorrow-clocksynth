#pragma once

#include <iostream>
#include <algorithm>
#include <map>
#include <set>
#include <vector>

namespace clksyn {

constexpr const auto MAX_BOUND = std::numeric_limits<int64_t>::max();

struct BlockageManager {
  BlockageManager() {}

  void printStructure();
  void insertBlockage(int64_t x1, int64_t y1, int64_t x2, int64_t y2);
  int64_t getOverlapPerimeter(int64_t x1, int64_t y1, int64_t x2, int64_t y2);

private:
  using interval_t = std::pair<int64_t, int64_t>;
  std::map<interval_t, std::set<interval_t>> intervalsXToY_;
  std::set<interval_t> intervalsX_;
};

inline int64_t BlockageManager::getOverlapPerimeter(int64_t x1, int64_t y1,
                                                    int64_t x2, int64_t y2) {
  // find the interval just larger than
  auto it = intervalsX_.upper_bound({x1, MAX_BOUND});
  if (it != intervalsX_.begin()) {
    --it;
  }

  auto res = 0;

  while (it != intervalsX_.end() && it->first <= x2) {
    auto [x1ref, x2ref] = *it;
    auto &iY = intervalsXToY_[*(it++)];
    if (x2ref < x1) {
      continue;
    }

    auto yOverlap = [&, x1ref = x1ref, x2ref = x2ref](auto &&y) -> int64_t {
      // find first interval that starts after y
      // and move an interval before that to find
      // the one that may contain y.
      auto it = iY.upper_bound({y, MAX_BOUND});
      if (it != iY.begin()) {
        auto [y1ref, y2ref] = *(--it);
        if (y1ref <= y && y2ref >= y) {
          return std::max(x2, x2ref) - std::min(x1, x1ref) + 1;
        }
      }
      return 0;
    };

    res += yOverlap(y1) + yOverlap(y2);

    // find first interval that starts afer y
    // and move an interval before that to find
    // the one that may contain y.
    auto ity = iY.upper_bound({y1, MAX_BOUND});
    if (ity != iY.begin()) {
      ity--;
    }

    int64_t sideOverlap = 0;

    for (; ity != iY.end() && ity->first <= y2; ity++) {
      if (ity->second >= y1) {
        sideOverlap += std::min(ity->second, y2) - std::max(ity->first, y1) + 1;
      }
    }

    if (x1ref <= x1 && x2ref >= x1) {
      res += sideOverlap;
    }

    if (x1ref <= x2 && x2ref >= x2) {
      res += sideOverlap;
    }
  }
  return res;
}

inline void BlockageManager::printStructure() {
  for (auto [rxx, ryy] : intervalsXToY_) {
    std::cout << "(" << rxx.first << " " << rxx.second << ") -> ";
    for (auto yy : ryy) {
      std::cout << "(" << yy.first << " " << yy.second << ") ";
    }
    std::cout << std::endl;
  }
}

inline void BlockageManager::insertBlockage(int64_t x1, int64_t y1, int64_t x2,
                                            int64_t y2) {
  auto it = intervalsX_.upper_bound({x2, MAX_BOUND});

  if (it == intervalsX_.begin()) {
    // this means that the interval being inserted is the smallest
    // overall and we need not do other reorganisation
    intervalsX_.insert({x1, x2});
    intervalsXToY_[{x1, x2}] = {{y1, y2}};
    // @FIXME: remove this printout
    printStructure();
    return;
  }

  // before commiting changes to the main data structure, we stage
  // all the removes and adds here
  std::vector<interval_t> toRemove;
  std::map<interval_t, std::set<interval_t>> toAdd;

  auto lim = x2;

  // Helpers for data structure manipulation

  auto createFromParentPlusY = [&](interval_t xx, interval_t yy,
                                   interval_t parent) {
    toAdd[xx] = intervalsXToY_[parent];
    toAdd[xx].insert(yy);
  };

  auto createFromParentConditional = [&](interval_t xx, interval_t parent,
                                         bool cond) {
    if (cond) {
      toAdd[xx] = intervalsXToY_[parent];
    }
  };

  auto createConditional = [&](interval_t xx, interval_t yy, bool cond) {
    if (cond) {
      toAdd[xx] = {yy};
    }
  };

  // iterate through interesting (overlapping) intervals and figure
  // out the adds and removes that need to happen
  do {
    --it;
    auto [x1ref, x2ref] = *it;

    if (x2ref < x1) {
      // no more overlapping intervals
      break;
    }

    if (x1ref <= x1 && x2ref >= x2) {
      createFromParentPlusY({x1, x2}, {y1, y2}, *it);
      createFromParentConditional({x1ref, x1 - 1}, *it, x1ref < x1);
      createFromParentConditional({x2 + 1, x2ref}, *it, x2ref > x2);
    } else if (x1ref <= x2 && x2ref >= x2) {
      createFromParentPlusY({x1ref, x2}, {y1, y2}, *it);
      createFromParentConditional({x2 + 1, x2ref}, *it, x2ref > x2);
    } else if (x1ref <= x1 && x2ref < x2) {
      createFromParentPlusY({x1, x2ref}, {y1, y2}, *it);
      createConditional({x2ref + 1, lim}, {y1, y2}, lim > x2ref + 1);
      createFromParentConditional({x1ref, x1 - 1}, *it, x1ref < x1);
    } else if (x1ref > x1 && x2ref < x2) {
      createFromParentPlusY({x1ref, x2ref}, {y1, y2}, *it);
      createConditional({x2ref + 1, lim}, {y1, y2}, lim > x2ref + 1);
    }
    toRemove.push_back(*it);
    lim = x1ref - 1;

  } while (it != intervalsX_.begin());

  createConditional({x1, lim}, {y1, y2}, lim >= x1);

  // commit all the removes and adds
  // we do removes first because we are allowing the same interval
  // to be removed and added in the above algorithm
  for (auto rxx : toRemove) {
    intervalsXToY_.erase(rxx);
    intervalsX_.erase(rxx);
  }

  for (auto [rxx, ryy] : toAdd) {
    intervalsXToY_[rxx] = ryy;
    intervalsX_.insert(rxx);
  }

  // @FIXME: remove this printout
  printStructure();
}

} // end namespace clksyn
