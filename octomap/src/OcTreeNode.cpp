/*
 * OctoMap - An Efficient Probabilistic 3D Mapping Framework Based on Octrees
 * http://octomap.github.com/
 *
 * Copyright (c) 2009-2013, K.M. Wurm and A. Hornung, University of Freiburg
 * All rights reserved.
 * License: New BSD
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of Freiburg nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <bitset>
#include <cassert>
#include <math.h>
#include <fstream>
#include <stdlib.h>
#include <inttypes.h>

#include <octomap/OcTreeNode.h>

namespace octomap {

  OcTreeNode::OcTreeNode()
    : OcTreeDataNode<float>(0.0)
  {
  }

  OcTreeNode::~OcTreeNode(){
  }

  
  // ============================================================
  // =  occupancy probability  ==================================
  // ============================================================

  double OcTreeNode::getMeanChildLogOdds() const{
    double mean = 0;
    uint8_t c = 0;
    if (children !=NULL){
      for (unsigned int i=0; i<8; i++) {
        if (children[i] != NULL) {
          mean += static_cast<OcTreeNode*>(children[i])->getOccupancy(); // TODO check if works generally
          ++c;
        }
      }
    }
    
    if (c > 0)
      mean /= (double) c;

    return log(mean/(1-mean));
  }

  float OcTreeNode::getMaxChildLogOdds() const{
    float max = -std::numeric_limits<float>::max();
    
    if (children !=NULL){
      for (unsigned int i=0; i<8; i++) {
        if (children[i] != NULL) {
          float l = static_cast<OcTreeNode*>(children[i])->getLogOdds(); // TODO check if works generally
          if (l > max)
            max = l;
        }
      }
    }
    return max;
  }

  void OcTreeNode::addValue(const float& logOdds) {
    value += logOdds;
  }
  

  float proba_decode(unsigned value, unsigned encoding) {
    return float(value) / ((1ULL << (8*encoding)) - 1);
  }

  unsigned proba_encode(float value, unsigned encoding) {
    return round(value * ((1ULL << (8*encoding)) - 1));
  }

  std::istream& OcTreeNode::readData(std::istream &s, unsigned encoding) {
    if (encoding == 0) {
      OcTreeDataNode<float>::readData(s, encoding);
    } else if (encoding <= 4) {
      unsigned value_encoded = 0;
      for (int i=0;i<encoding;i++) {
        unsigned char byte;
        s.read((char*) &byte, 1);
        value_encoded += (byte << (8*i));
      }

      value = logodds(proba_decode(value_encoded, encoding));
    } else {
      std::cout << "readData: unsupported encoding: " << encoding << std::endl;
      exit(-1);
    };
    return s;
  }


  /// Write node payload (data only) to binary stream
  std::ostream& OcTreeNode::writeData(std::ostream &s, unsigned encoding) const {
    //std::cout << ".." << std::endl;
    if (encoding == 0) {
      OcTreeDataNode<float>::writeData(s, encoding);
    } else if (encoding <= 4) {
      //std::cout << "v:" << probability << std::endl;
      unsigned value_encoded = proba_encode(probability(value), encoding); // set between 0 and 2**encoding-1
      //std::cout << value_encoded << "/" << ((1 << (8*encoding)) - 1) << std::endl;
      for (int i=0;i<encoding;i++) {
        char byte = value_encoded;
        s.write((const char*) &byte, 1);
        value_encoded = value_encoded >> 8;
      };


      if (value_encoded != 0) {
        std::cout << "writeData: error while encoding data. (" << value_encoded << ")" << std::endl;
        exit(-1);
      }
      //std::cout << "ok" << std::endl;
    } else {
      std::cout << "writeData: unsupported encoding: " << encoding << std::endl;
      exit(-1);
    }
    return s;
  }
} // end namespace


