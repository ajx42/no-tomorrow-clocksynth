#include <argparse/argparse.hpp>
#include "parser.hpp"
#include "topology.hpp"
#include "blockage.hpp"

int main( int argc, char** argv ) {
  argparse::ArgumentParser program( "main" );
  program.add_argument( "--input" )
         .required()
         .help( "input file to read from, provided by ISPD 2009" );

  program.add_argument( "--output" )
         .required()
         .help( "file to write output to" );

  try {
    program.parse_args( argc, argv );
  } catch ( const std::runtime_error& err ) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1); 
  }

  auto inputFile = program.get<std::string>( "--input" );
  auto outputFile = program.get<std::string>( "--output" );

  auto inp = parse( inputFile );

  auto syn = clksyn::TreeSynthesis(
    inp,
    clksyn::TreeSynthesisSettings {
      .Algo = clksyn::TopologyAlgorithm::NNA,
      .Alpha = 0, .Beta = 0, .Gamma = 0, .Delta = 0.5
    }
  );

  /*
  auto syn = clksyn::TreeSynthesis(
    inp,
    clksyn::TreeSynthesisSettings {
      .Algo = clksyn::TopologyAlgorithm::DNNA,
      .Alpha = 0.2, .Beta = 1.0, .Gamma = 0.5, .Delta = 2.5
    }
  );
  */

  auto top = syn.getTopology().toOutParam();

  print_output(outputFile, top);

  /*
  auto alpha = clksyn::BlockageManager();  

  while ( true ) {
    int64_t x1, y1, x2, y2;
    std::cin >> x1 >> x2 >> y1 >> y2;
    if ( x1 == -1 && x2 == -1 ) break;
    alpha.insertBlockage(x1, y1, x2, y2);
  }

  std::cout << "overlap" << std::endl;

  while ( true ) {
    int64_t x1, y1, x2, y2;
    std::cin >> x1 >> x2 >> y1 >> y2;
    if ( x1 == -1 && x2 == -1 ) break;
    std::cout << alpha.getOverlapPerimeter(x1, y1, x2, y2) << std::endl;
  }
  */
  return 0;
}
