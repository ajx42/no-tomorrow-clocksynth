
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include<string>

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
  char source_name[100];
  char buf_name[100];
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
  char cktname[100];
  int inverted;
  long int in_cap;
  long int out_cap;
  float resistance;
};

struct inparams {
  std::vector<struct sink*> sinks;
  std::vector<struct wire*> wires;
  std::vector<struct buffer*> buffers;
  struct simulation *smul;
  struct source *src;
};

struct out_node {
  char name[100];
  struct point pt;
};

struct out_wire {
  char from[100];
  char to[100];
  int type;
};

struct out_buffer { 
  char from[100];
  char to[100];
  int type;
};

struct out_sink { 
  char node_name[100];
  char sink_name[100];
};

struct out_srcnode { 
  char node_name[100];
  char src_name[100];
};

struct outparams {
  struct out_srcnode *src;
  std::vector<struct out_node*> nodes;
  std::vector<struct out_sink*> sinks;
  std::vector<struct out_wire*> wires;
  std::vector<struct out_buffer*> buffers;
};

struct inparams parse(char *filename) {

  struct inparams output_pkt;
  output_pkt.smul = (struct simulation *)malloc(sizeof(struct simulation));
  output_pkt.src = (struct source *)malloc(sizeof(struct source));

  std::ifstream file(filename);
  std::cout<<file.is_open()<<std::endl;
  int mode = READ_FLOORPLAN, iter = 0;

  if (file.is_open()) {
      std::string line;
      while (std::getline(file, line)) {

          if(!(line.size())) {
            continue;
          }
          else {
            std::stringstream s(line);
            if(mode==READ_FLOORPLAN) {
              s>>output_pkt.smul->lower_left.x>>output_pkt.smul->lower_left.y>>output_pkt.smul->upper_right.x>>output_pkt.smul->upper_right.y;
              mode = READ_SOURCE;
            }
            else if(mode==READ_SOURCE) {
              std::string t; 
              s>>t>>output_pkt.src->source_name>>output_pkt.src->pt.x>>output_pkt.src->pt.y>>output_pkt.src->buf_name;
              mode = READ_SINK;
            }
            else if(mode==READ_SINK) {
              if(iter==0) {
                std::string t1,t2; int num_sinks;
                s>>t1>>t2>>num_sinks;
                iter = num_sinks;
              }
              else {
                struct sink *sink_t = (struct sink *)malloc(sizeof(struct sink));
                s>>sink_t->id>>sink_t->cord.x>>sink_t->cord.y>>sink_t->cap;
                iter--;
                output_pkt.sinks.push_back(sink_t);
                if(iter==0) {
                  mode = READ_WIRE;
                }
              }
            }
            else if(mode==READ_WIRE) {
              if(iter==0) {
                std::string t1,t2; int num_wires;
                s>>t1>>t2>>num_wires;
                iter = num_wires;
              }
              else {
                struct wire *wire_t = (struct wire *)malloc(sizeof(struct wire));
                s>>wire_t->type>>wire_t->cap>>wire_t->resistance;
                iter--;
                output_pkt.wires.push_back(wire_t);
                if(iter==0) {
                  mode = READ_BUF;
                }
              }
            }
            else if(mode==READ_BUF) {
              if(iter==0) {
                std::string t1,t2; int num_buf;
                s>>t1>>t2>>num_buf;
                iter = num_buf;
              }
              else {
                struct buffer *buf_t = (struct buffer *)malloc(sizeof(struct buffer));
                s>>buf_t->id>>buf_t->cktname>>buf_t->inverted>>buf_t->in_cap>>buf_t->out_cap>>buf_t->resistance;
                iter--;
                output_pkt.buffers.push_back(buf_t);
                if(iter==0) {
                  mode = READ_SIMUL;
                }
              }
            }
            else if(mode==READ_SIMUL) {
              std::string t1,t2;
              s>>t1>>t2>>output_pkt.smul->vdd.vdd_param1>>output_pkt.smul->vdd.vdd_param2;
              mode = READ_SLEW;
            }
            else if(mode==READ_SLEW) {
              std::string t1,t2;
              s>>t1>>t2>>output_pkt.smul->slew_limit;
              mode = READ_CAP;
            }
            else if(mode==READ_CAP) {
              std::string t1,t2;
              s>>t1>>t2>>output_pkt.smul->cap_limit;
              mode = READ_BLOCKAGE;
            }
          }
      }
      file.close();
  }

  return output_pkt;
}

void print_output(char *filename, struct outparams t){

  std::ofstream file;
  file.open (filename);
  file <<"sourcenode "<<t.src->node_name<<" "<<t.src->src_name<<"\n";
  file <<"num node "<<t.nodes.size()<<"\n";

  for(int k=0; k<t.nodes.size(); k++){
    file <<t.nodes[k]->name<<" "<<t.nodes[k]->pt.x<<" "<<t.nodes[k]->pt.y<<"\n";
  }
  file <<"num sinknode "<<t.sinks.size()<<"\n";

  for(int k=0; k<t.sinks.size(); k++){
    file <<t.sinks[k]->node_name<<" "<<t.sinks[k]->sink_name<<"\n";
  }
  file <<"num wire "<<t.wires.size()<<"\n";

  for(int k=0; k<t.wires.size(); k++){
    file <<t.wires[k]->from<<" "<<t.wires[k]->to<<" "<<t.wires[k]->type<<"\n";
  }

  file <<"num buffer "<<t.buffers.size()<<"\n";

  for(int k=0; k<t.wires.size(); k++){
    file <<t.buffers[k]->from<<" "<<t.buffers[k]->to<<" "<<t.buffers[k]->type<<"\n";
  }

  file.close();

}
