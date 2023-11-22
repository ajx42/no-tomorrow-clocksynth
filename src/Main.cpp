#include "parser.hpp"

int main(int argc, char *argv[]) {
  if(argc!=3) {
    std::cout<<"Wrong input format. Correct Format: [INPUT_FILE] [OUTPUT_FILE]\n";
    return 0;
  }

  struct inparams t = parse(argv[1]);

  //DME here

  struct outparams t_out; 
  //print_output(argv[2], t_out);

  return 0;
}
