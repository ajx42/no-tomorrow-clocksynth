#pragma once

#include "parser.hpp"
#include "WowLogger.H"

#include <iterator>
#include <queue>
#include <set>
#include <map>

namespace clksyn {

// Supporting two Topology Generation Algorithms.
// NNA: https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=1600293
//    - Simple distance based cost function.
//    - In each pass, we pick `Delta` fraction of the nodes at hand.
// DNNA: https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5419850
//    - Cost function includes information regarding blockages and loads.
//    - In each pass, a cost range defined by (min_cost, min_cost * Delta)
//      is used to pick nodes.
enum class TopologyAlgorithm
{
  DNNA, NNA
};

// Various parameter settings required by the algorithms.
// Note that NNA only requires Delta.
struct TreeSynthesisSettings
{
  TopologyAlgorithm Algo;
  double Alpha, Beta, Gamma, Delta;
};

struct TreeNode
{
  enum {SINK, INTERNAL, SOURCE} Kind;
  int32_t Idx;
  int64_t x, y;
  int64_t LdCap;

  auto operator<=>( const TreeNode& ) const = default;
};

inline std::ostream& operator<<( std::ostream& os, const TreeNode& p )
{
  auto nodeType = p.Kind == TreeNode::SOURCE ? "SOURCE"
                : (p.Kind == TreeNode::INTERNAL ? "INTERNAL" : "SINK");
  os << "Node(Type=" << nodeType
     << " Idx=" << p.Idx << " x=" << p.x << " y=" << p.y
     << " cap=" << p.LdCap << ")";
  return os;
}

struct TopologyResult
{
  std::vector<TreeNode> Nodes;
  std::vector<std::pair<int32_t, int32_t>> Edges;
  std::map<int32_t, std::string> Tags;

  outparams toOutParam();
};

inline outparams TopologyResult::toOutParam()
{
  outparams res;
  
  res.src = out_srcnode {
    .node_name = std::to_string( 0 ),
    .src_name = Tags[0]
  };

  for ( auto& node: Nodes ) {
    if ( node.Kind == TreeNode::INTERNAL ) {
      res.nodes.push_back( out_node {
        .name = std::to_string( node.Idx ),
        .pt = point {
          .x = node.x,
          .y = node.y
        }
      } );
    } else if ( node.Kind == TreeNode::SINK ) {
      res.sinks.push_back( out_sink {
        .node_name = std::to_string( node.Idx ),
        .sink_name = Tags[node.Idx]
      } );
    }
  }

  for ( auto& edge: Edges ) {
    res.wires.push_back( out_wire {
      .from = std::to_string( edge.first ),
      .to = std::to_string( edge.second ),
    } );
  }
  
  return res;
}

struct NodePair {
  double Cost;
  TreeNode A, B;

  auto operator<=>( const NodePair& ) const = default;
  
  bool operator<( const NodePair& rhs ) const
  {
    return Cost > rhs.Cost;
  }

  TreeNode simpleMerge( int32_t resIdx ) const;
};

inline std::ostream& operator<<( std::ostream& os, const NodePair& p )
{
  os << "Pair(Cost=" << p.Cost << " A:[" << p.A << "] B:[" << p.B << "])";
  return os;
}

// Merging at midpoint which is not ideal. May be a good idea to merge
// based on ratio capacitive load.
inline TreeNode NodePair::simpleMerge( int32_t resIdx ) const
{
  return TreeNode {
    .Kind = TreeNode::INTERNAL,
    .Idx = resIdx,
    .x = (A.x + B.x) / 2,
    .y = (A.y + B.y) / 2,
    .LdCap = A.LdCap + B.LdCap,
  };
}

// Main class for handling tree synthesis tasks.
// @TODO move this to a cleaner place.
struct TreeSynthesis
{
  TreeSynthesis( inparams, TreeSynthesisSettings );

  TopologyResult getTopology();
  outparams getSynthesisedTree();

private:
  double pairCost( TreeNode a, TreeNode b );
  bool endPass(
    int32_t picked, int32_t total, double curCost, double minCost );

  inparams inp_;
  TreeSynthesisSettings sett_;

  std::map<int32_t, std::string> idxToTag_;

  std::vector<TreeNode> sinks_;
  TreeNode source_;
};

TreeSynthesis::TreeSynthesis( inparams inp, TreeSynthesisSettings sett )
  : inp_( inp ), sett_( sett )
{
  std::for_each(
    inp_.sinks.begin(), inp_.sinks.end(),
    [&]( auto&& sink ) {
      sinks_.push_back(TreeNode {
        .Kind = TreeNode::SINK,
        .Idx = static_cast<int32_t>( sinks_.size() + 1 ),
        .x = sink.cord.x,
        .y = sink.cord.y,
        .LdCap = sink.cap,
      });
      idxToTag_[sinks_.back().Idx] = sink.id;
    }
  );
  
  source_ = TreeNode {
    .Kind = TreeNode::SOURCE,
    .Idx = 0,
    .x = inp_.src.pt.x,
    .y = inp_.src.pt.y,
    .LdCap = 0,
  };
  idxToTag_[0] = inp_.src.source_name;
}

// Determines cost of merging two nodes. This is based on the
// algorithm being used.
inline double TreeSynthesis::pairCost( TreeNode a, TreeNode b )
{
  double ret = 0;
  switch ( sett_.Algo ) {
    case TopologyAlgorithm::NNA: {
      ret = abs(a.x - b.x) + abs(a.y - b.y);
      break;
    }
    case TopologyAlgorithm::DNNA: {
      auto nodeDistance = abs(a.x - b.x) + abs(a.y - b.y);
      double blockageOverlap = 0; // @TODO
      double loadDistance = abs(a.LdCap - b.LdCap) / std::max(a.LdCap, b.LdCap);
      double totalLoad = 0; // @TODO
      ret = double(nodeDistance) * (1 + blockageOverlap/sett_.Alpha)
                                 * (1 + loadDistance/sett_.Beta)
                                 * (1 + totalLoad/sett_.Gamma);
    }
  }
  return ret;
}

// Terminating condition for a pass, again based on the algorithm
// being used.
inline bool TreeSynthesis::endPass(
  int32_t picked, int32_t total, double curCost, double minCost )
{
  switch ( sett_.Algo ) {
    case TopologyAlgorithm::NNA: {
      return total * sett_.Delta < picked;
    }
    case TopologyAlgorithm::DNNA: {
      return curCost > minCost * sett_.Delta;
    }
  }
  __builtin_unreachable();
}

inline TopologyResult TreeSynthesis::getTopology()
{
  // NodePair < operator is overloaded so that priority
  // queue has the lowest cost at the top.
  std::priority_queue<NodePair> pq;
  
  // Keeps track of unmerged nodes at a given point of time
  // during the execution.
  std::set<TreeNode> actv;

  // Result to be returned.
  TopologyResult res;

  // As we continue to merge and move up, we keep track of the
  // last node that was result of a merge as the root.
  TreeNode root;

  // Insert all the pairs corresponding to all sinks. Mark them
  // all as unmerged by pushing to the `actv` set.
  for ( const auto& i : sinks_ ) {
    actv.insert( i );
    res.Nodes.push_back( i );
    for ( const auto& j : sinks_ ) {
      if ( i.Idx <= j.Idx ) {
        continue;
      }
      pq.push( NodePair { .Cost = pairCost(i, j), .A = i, .B = j } );
    }
  }

  // Any nodes that have already been merged or picked for merging in
  // the current pass are marked visited.
  std::vector<bool> vis(sinks_.size() * 2, false);
  
  // Use to assign node indices to newly created internal nodes.
  int32_t nextIdx = sinks_.size() + 1;

  while ( ! pq.empty() ) {
    // start a new pass
    double curCost = 0;
    double minCost = std::numeric_limits<double>::max();
    std::vector<NodePair> pickedPairs;
   
    // In a single pass pick node pairs until we exhaust all available
    // pairs or we meet the terminating condition. 
    do {
      auto top = pq.top(); pq.pop();
      if ( vis.size() <= std::max( top.A.Idx, top.B.Idx ) ) {
        vis.resize( std::max( top.A.Idx, top.B.Idx ) + 1 );
      }
      if ( vis[top.A.Idx] || vis[top.B.Idx] ) {
        continue;
      }
      vis[top.A.Idx] = vis[top.B.Idx] = true;
      pickedPairs.push_back( top );
      curCost = top.Cost;
      minCost = std::min(minCost, curCost); // this should only run once
    } while ( ! endPass(pickedPairs.size() * 2, actv.size(), curCost, minCost) && ! pq.empty() );
    
    // Create new nodes by merging the pairs that have been picked
    // in this pass. Add edges to the result as well.
    std::vector<TreeNode> newNodes;
    for ( const auto& pr : pickedPairs ) {
      auto merged = pr.simpleMerge( nextIdx++ );
      root = merged;
      res.Nodes.push_back( merged );
      res.Edges.push_back( { merged.Idx, pr.A.Idx } );
      res.Edges.push_back( { merged.Idx, pr.B.Idx } );
      newNodes.push_back( merged );
    }

    // Remove merged nodes from the `actv` set. They are no longer
    // available for merging.
    std::for_each( pickedPairs.begin(), pickedPairs.end(),
        [&]( auto&& pr ) { actv.erase( pr.A ); actv.erase( pr.B ); } );

    // Insert newly created nodes of this pass into the `actv` set.
    std::for_each( newNodes.begin(), newNodes.end(),
        [&]( auto&& n ) { actv.insert( std::move( n ) ); } );
 
    // Generate node pairs for the newly created nodes and add to the
    // priority queue.
    for ( const auto& nNode: newNodes ) {
      for ( const auto& kNode: actv ) {
        if ( nNode.Idx == kNode.Idx ) {
          continue;
        }
        pq.push( NodePair {
            .Cost = pairCost(nNode, kNode), .A = nNode, .B = kNode } );
      }
    }
  }

  // Connect source to the root.
  res.Nodes.push_back( source_ );
  res.Edges.push_back( {source_.Idx, root.Idx} );
  res.Tags = idxToTag_;
  
  return res;
}

} // end namespace clksyn
