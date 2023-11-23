
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>

#define READ_FLOORPLAN 0
#define READ_SOURCE 1
#define READ_SINK 2
#define READ_WIRE 3
#define READ_BUF 4
#define READ_SIMUL 5
#define READ_SLEW 6
#define READ_CAP 7
#define READ_BLOCKAGE 8

struct point {
  long int x;
  long int y;
};

struct voltage {
  float vdd_param1;
  float vdd_param2;
};

struct simulation{
  struct point lower_left;
  struct point upper_right;
  struct voltage vdd;
  long int slew_limit;
  long int cap_limit;
};

struct source{
  struct point pt;
  std::string source_name;
  std::string buf_name;
};

struct sink {
  int id;
  struct point cord;
  long int cap;
};

struct wire {
  int type;
  float cap;
  float resistance;
};

struct buffer { //Probably not needed ?
  int id;
  std::string cktname;
  int inverted;
  long int in_cap;
  long int out_cap;
  float resistance;
};

struct inparams {
  std::vector<sink> sinks;
  std::vector<wire> wires;
  std::vector<buffer> buffers;
  simulation smul;
  source src;
};

struct out_node {
  std::string name;
  struct point pt;
};

struct out_wire {
  std::string from, to;
  int type;
};

struct out_buffer { 
  std::string from, to;
  int type;
};

struct out_sink { 
  std::string node_name, sink_name;
};

struct out_srcnode { 
  std::string node_name, src_name;
};

struct outparams {
  out_srcnode src;
  std::vector<out_node> nodes;
  std::vector<out_sink> sinks;
  std::vector<out_wire> wires;
  std::vector<out_buffer> buffers;
};

inline inparams parse(std::string filename) {

  struct inparams output_pkt;

  std::ifstream file(filename);
  int mode = READ_FLOORPLAN, iter = 0;

  if (file.is_open()) {
      std::string line;
      while (std::getline(file, line)) {

          if(!(line.size())) {
            continue;
          }
          else {
            std::stringstream s(line);
            if ( mode == READ_FLOORPLAN ) {
              s >> output_pkt.smul.lower_left.x
                >> output_pkt.smul.lower_left.y
                >> output_pkt.smul.upper_right.x
                >> output_pkt.smul.upper_right.y;
              mode = READ_SOURCE;
            }
            else if ( mode == READ_SOURCE ) {
              std::string t; 
              s >> t
                >> output_pkt.src.source_name
                >> output_pkt.src.pt.x
                >> output_pkt.src.pt.y
                >> output_pkt.src.buf_name;
               
              mode = READ_SINK;
            }
            else if ( mode == READ_SINK ) {
              if ( iter == 0 ) {
                std::string t1,t2;
                int num_sinks;
                s >> t1 >> t2 >> num_sinks;
                iter = num_sinks;
              }
              else {
                sink curr;
                s >> curr.id >> curr.cord.x >> curr.cord.y >> curr.cap;
                iter--;
                output_pkt.sinks.push_back(curr);
                if ( iter == 0 ) {
                  mode = READ_WIRE;
                }
              }
            }
            else if ( mode == READ_WIRE ) {
              if ( iter == 0 ) {
                std::string t1, t2;
                int num_wires;
                s >> t1 >> t2 >> num_wires;
                iter = num_wires;
              }
              else {
                wire curr;
                s >> curr.type >> curr.cap >> curr.resistance;
                iter--;
                output_pkt.wires.push_back(curr);
                if ( iter == 0 ) {
                  mode = READ_BUF;
                }
              }
            }
            else if ( mode == READ_BUF ) {
              if ( iter == 0 ) {
                std::string t1, t2;
                int num_buf;
                s >> t1 >> t2 >> num_buf;
                iter = num_buf;
              }
              else {
                buffer curr;
                s >> curr.id >> curr.cktname
                  >> curr.inverted >> curr.in_cap
                  >> curr.out_cap >> curr.resistance;
                iter--;
                output_pkt.buffers.push_back(curr);
                if ( iter == 0 ) {
                  mode = READ_SIMUL;
                }
              }
            }
            else if ( mode == READ_SIMUL ) {
              std::string t1,t2;
              s>>t1>>t2>>output_pkt.smul.vdd.vdd_param1>>output_pkt.smul.vdd.vdd_param2;
              mode = READ_SLEW;
            }
            else if(mode==READ_SLEW) {
              std::string t1,t2;
              s>>t1>>t2>>output_pkt.smul.slew_limit;
              mode = READ_CAP;
            }
            else if(mode==READ_CAP) {
              std::string t1,t2;
              s>>t1>>t2>>output_pkt.smul.cap_limit;
              mode = READ_BLOCKAGE;
            }
          }
      }
      file.close();
  }

  return output_pkt;
}

inline void print_output(std::string filename, struct outparams t){

  std::ofstream file;
  file.open (filename);
  file <<"sourcenode "<<t.src.node_name<<" "<<t.src.src_name<<"\n";
  file <<"num node "<<t.nodes.size()<<"\n";

  for(size_t k=0; k<t.nodes.size(); k++){
    file <<t.nodes[k].name<<" "<<t.nodes[k].pt.x<<" "<<t.nodes[k].pt.y<<"\n";
  }
  file <<"num sinknode "<<t.sinks.size()<<"\n";

  for(size_t k=0; k<t.sinks.size(); k++){
    file <<t.sinks[k].node_name<<" "<<t.sinks[k].sink_name<<"\n";
  }
  file <<"num wire "<<t.wires.size()<<"\n";

  for(size_t k=0; k<t.wires.size(); k++){
    file <<t.wires[k].from<<" "<<t.wires[k].to<<" "<<t.wires[k].type<<"\n";
  }

  file <<"num buffer "<<t.buffers.size()<<"\n";

  for(size_t k=0; k<t.wires.size(); k++){
    file <<t.buffers[k].from<<" "<<t.buffers[k].to<<" "<<t.buffers[k].type<<"\n";
  }

  file.close();

}
