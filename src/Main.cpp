#include <argparse/argparse.hpp>
#include "parser.hpp"

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

  outparams test;
  print_output(outputFile, test);

  return 0;
}
