#include <iostream>
#include "../include/Glc.hpp"
using namespace std;

int main(int argc, char** argv) {
  if (argc < 3) {
    cerr << "TOO FEW ARGUMENTS\n";
    return 1;
  }

  bool verbose = false;
  if (argc >= 4) {
    verbose = argv[3][0] == 'v';
  }

  char* input_file = argv[1];
  char* output_file = argv[2];
  GLC* g; 

  try {
    g = new GLC(input_file);
  } catch (runtime_error e) {
    cerr << e.what();
    return 1;
  };
  
  
  g->convertToFNC(verbose);

  ofstream file(output_file);
  
  file << *g;
  file.close();
  return 0;
}