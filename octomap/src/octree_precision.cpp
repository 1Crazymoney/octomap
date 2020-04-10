#include <string>
#include <fstream>
#include <iostream>
#include <octomap/octomap.h>
#include <cstdlib>
#include <cstring>
#include <utility>

using namespace std;
using namespace octomap;

const double THRESH_MIN = 0.1192-0.0001;
const double THRESH_MAX = 0.971 +0.0001;

float proba_decode(unsigned value, unsigned bit_precision) {
  double res = value / double((1LL << (bit_precision)) - 1LL); 
  return THRESH_MIN + (res * (THRESH_MAX - THRESH_MIN));
}

unsigned proba_encode(float p_value, unsigned bit_precision) {
  double value = (double(p_value) - THRESH_MIN)/(THRESH_MAX - THRESH_MIN); // transform between zero and one.
  return round(value * double((1LL << (bit_precision)) - 1LL));
}

std::pair<float, unsigned> explore(OcTree* tree, OcTreeNode* root, int precision) {
  assert(root != NULL);

  float odds = exp(root->getLogOdds());
  float p = odds / (1 + odds);

  unsigned n = 1;
  float error;

  error = pow(proba_decode(proba_encode(p, precision), precision) - p, 2);

  for (unsigned int i=0; i<8; i++) {
    if (tree->nodeChildExists(root, i)) {
      auto result = explore(tree, tree->getNodeChild(root, i), precision);
      n += result.second;
      error += result.first;
    }
  }
  return std::pair<float, unsigned>(error, n);
}

int main(int argc, char **argv)
{
    bool show_help = false;
    string inputFilename("");

    if(argc == 1) show_help = true;
    for(int i = 1; i < argc && !show_help; i++) {
        if(strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-help") == 0 ||
           strcmp(argv[i], "--usage") == 0 || strcmp(argv[i], "-usage") == 0 ||
           strcmp(argv[i], "-h") == 0
          )
               show_help = true;
    }
    
    if(show_help) {
        cout << "Usage: "<<argv[0]<<" <filename.bt>" << endl;
        exit(0);
    }
    
    for(int i = 1; i < argc; i++) {
      if (i == argc-1){
        inputFilename = string(argv[i]);
      }
    }

    AbstractOcTree* a_tree = AbstractOcTree::read(inputFilename);
    OcTree* tree = dynamic_cast<OcTree*>(a_tree);
    
    std::cout << tree->getTreeType() << std::endl;

    for (int precision=1; precision<33; precision++) {
      auto result = explore(tree, tree->getRoot(), precision);
      std::cout << sqrt(result.first / result.second) << std::endl;
    }

    return 0;
}
