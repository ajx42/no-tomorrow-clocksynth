#pragma once

#include <algorithm>
#include <map>
#include <set>
#include <vector>

namespace clksyn {

struct BlockageManager
{
  BlockageManager() {}

  void insertBlockage(int64_t x1, int64_t y1, int64_t x2, int64_t y2);
  int64_t getOverlapPerimeter(int64_t x1, int64_t y1, int64_t x2, int64_t y2);

private:
  using interval_t = std::pair<int64_t, int64_t>;
  std::map<interval_t, std::set<interval_t>> intervalsXToY_;
  std::set<interval_t> intervalsX_;
};

inline int64_t BlockageManager::getOverlapPerimeter(int64_t x1, int64_t y1, int64_t x2, int64_t y2)
{
  auto it = intervalsX_.upper_bound({x1, 0});
  if ( it != intervalsX_.begin() ) {
    --it;
  }
  auto res = 0;
  while ( it != intervalsX_.end() && it->first <= x2 ) {
    auto [x1ref, x2ref] = *it;
    auto& iY = intervalsXToY_[*(it++)];
    if ( x2ref < x1 ) {
      continue;
    }
    
    auto ity1 = iY.upper_bound({y1, std::numeric_limits<int64_t>::max()});
    if ( ity1 != iY.begin() ) {
      auto [y1ref, y2ref] = *(--ity1);
      if ( y1ref <= y1 && y2ref >= y1 ) {
        res += std::min(x2, x2ref) - std::max(x1, x1ref) + 1;
      }
    }

    auto ity2 = iY.upper_bound({y2, std::numeric_limits<int64_t>::max()});
    if ( ity2 != iY.begin() ) {
      auto [y1ref, y2ref] = *(--ity2);
      if ( y1ref <= y2 && y2ref >= y2 ) {
        res += std::min(x2, x2ref) - std::max(x1, x1ref) + 1;
      }
    }

    auto ity = iY.upper_bound({y1, std::numeric_limits<int64_t>::max()});
    if ( ity != iY.begin() ) {
      ity--;
    }

    int64_t sideOverlap = 0;
    while ( ity != iY.end() && ity->first <= y2 ) {
      if ( ity->second >= y1 ) {
        sideOverlap += std::min(ity->second, y2) - std::max(ity->first, y1) + 1;
      }
      ity++;
    }

    if (x1ref <= x1 && x2ref >= x1) {
      res += sideOverlap;
    }
    
    if (x1ref <= x2 && x2ref >= x2 ) {
      res += sideOverlap;
    }
  }
  return res;
}


inline void BlockageManager::insertBlockage(int64_t x1, int64_t y1, int64_t x2, int64_t y2)
{
  auto it = intervalsX_.upper_bound({x2, std::numeric_limits<int64_t>::max()});
  if ( it == intervalsX_.begin() ) {
    // empty or smallest
    intervalsX_.insert({x1, x2});
    intervalsXToY_[{x1, x2}] = {{y1, y2}};
  for ( auto [rxx, ryy]: intervalsXToY_ ) {
    std::cout << "(" << rxx.first << " " << rxx.second << ") -> ";
    for ( auto yy: ryy ) {
      std::cout << "(" << yy.first << " " << yy.second << ") ";
    }
    std::cout << std::endl;
  }


    return;
  }

  // less than or equal to
  std::vector<interval_t> toRemove;
  std::map<interval_t, std::set<interval_t>> toAdd;

  auto lim = x2;

  do {
    --it;
    auto [x1ref, x2ref] = *it;
    
    if ( x2ref < x1 ) {
      break;
    }

    auto addInterval = [&]( interval_t xx, interval_t yy, interval_t ref ) {
      toAdd[xx] = intervalsXToY_[ref];
      toAdd[xx].insert(yy);
    };

    if ( x1ref <= x1 && x2ref >= x2 ) {
      addInterval({x1, x2}, {y1, y2}, *it);
      if ( x1ref < x1 ) toAdd[{x1ref, x1-1}] = intervalsXToY_[*it];
      if ( x2ref > x2 ) toAdd[{x2+1, x2ref}] = intervalsXToY_[*it];
      toRemove.push_back(*it);
    } else if ( x1ref <= x2 && x2ref >= x2) {
      addInterval({x1ref, x2}, {y1, y2}, *it);
      if ( x2ref > x2 ) toAdd[{x2+1, x2ref}] = intervalsXToY_[*it];
      toRemove.push_back(*it);
    } else if ( x1ref <= x1 && x2ref < x2 ) {
      addInterval({x1, x2ref}, {y1, y2}, *it);
      if ( lim > x2ref+1 ) toAdd[{x2ref+1, lim}].insert({y1, y2});
      if ( x1ref < x1 ) toAdd[{x1ref, x1-1}] = intervalsXToY_[*it];
      toRemove.push_back(*it);
    } else if ( x1ref > x1 && x2ref < x2 ) {
      addInterval({x1ref, x2ref}, {y1, y2}, *it);
      if ( lim > x2ref+1 ) toAdd[{x2ref+1, lim}].insert({y1, y2});
      toRemove.push_back(*it);
    }
    lim = x1ref-1;
    
  } while ( it != intervalsX_.begin() );

  if ( lim >= x1 ) {
    toAdd[{x1, lim}].insert({y1, y2});
  }

  for ( auto rxx: toRemove ) {
    intervalsXToY_.erase(rxx);
    intervalsX_.erase(rxx);
  }

  for ( auto [rxx, ryy]: toAdd ) {
    intervalsXToY_[rxx] = ryy;
    intervalsX_.insert(rxx);
  }

  for ( auto [rxx, ryy]: intervalsXToY_ ) {
    std::cout << "(" << rxx.first << " " << rxx.second << ") -> ";
    for ( auto yy: ryy ) {
      std::cout << "(" << yy.first << " " << yy.second << ") ";
    }
    std::cout << std::endl;
  }

}

} // end namespace clksyn
