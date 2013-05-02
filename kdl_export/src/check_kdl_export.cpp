/*********************************************************************
* Software License Agreement (BSD License)
* 
*  Copyright (c) 2008, Willow Garage, Inc.
*  All rights reserved.
* 
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
* 
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Willow Garage nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
* 
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

/* Author: Wim Meeussen */

#include "kdl_parser/kdl_parser.hpp"
#include <kdl/chainfksolverpos_recursive.hpp>
#include <kdl/frames_io.hpp>
#include <urdf/model.h>
#include <iostream>

using namespace KDL;
using namespace std;
using namespace urdf;

void printLink(const SegmentMap::const_iterator& link, const std::string& prefix)
{
  cout << prefix << "- Segment " << link->second.segment.getName() << " has " << link->second.children.size() << " children" << endl;
  for (unsigned int i=0; i<link->second.children.size(); i++)
    printLink(link->second.children[i], prefix + "  ");
}



int main(int argc, char** argv)
{
  if (argc < 2){
    std::cerr << "Expect xml file to parse" << std::endl;
    return -1;
  }
  Model robot_model;
  if (!robot_model.initFile(argv[1]))
  {cerr << "Could not generate robot model" << endl; return false;}

  Tree my_tree;
  if (!kdl_parser::treeFromUrdfModel(robot_model, my_tree)) 
  {cerr << "Could not extract kdl tree" << endl; return false;}

  // walk through tree
  cout << " ======================================" << endl;
  cout << " Tree has " << my_tree.getNrOfSegments() << " link(s) and a root link" << endl;
  cout << " ======================================" << endl;
  SegmentMap::const_iterator root = my_tree.getRootSegment();
  printLink(root, "");
}

