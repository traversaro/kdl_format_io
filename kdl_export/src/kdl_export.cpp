/*********************************************************************
* Software License Agreement (BSD License)
* 
*  Copyright (c) 2013, Silvio Traversaro, CoDyCo project
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

/* Author: Silvio Traversaro */

#include "kdl_parser/kdl_parser.hpp"
#include <urdf/model.h>
#include <kdl/frames_io.hpp>
#include <ros/console.h>

using namespace std;

namespace kdl_export{


// construct vector
urdf::Vector3 toUrdf(const KDL::Vector & v) 
{
    return urdf::Vector3(v.x(), v.y(), v.z());
}

// construct rotation
urdf::Rotation toUrdf(const KDL::Rotation & r) 
{
    double x,y,z,w;
    r.getQuaternion(x,y,z,w);
    return urdf::Rotation(x,y,z,w);
}

// construct pose
urdf::Pose toUrdf(const KDL::Frame & p)
{
    urdf::Pose ret;
    ret.rotation = toUrdf(p.M);
    ret.position = toUrdf(p.p);
    return ret;
}

// construct joint
urdf::Joint toUrdf(const KDL::Joint & jnt)
{
    urdf::Joint ret;
    ret.name = jnt.getName();
    switch(jnt.getType())
    {
        case Joint::RotAxis:
        case Joint::RotX:
        case Joint::RotY:
        case Joint::RotZ:
            ret.type = urdf::REVOLUTE;
            ret.parent_to_joint_origin_transform = toUrdf(KDL::Frame(KDL::Rotation::Identity(),jnt.JointOrigin()));
            ret.axis = toUrdf(jnt.JointAxis());
        break;
        case Joint::TransAxis:
        case Joint::TransX:
        case Joint::TransY:
        case Joint::TransZ:
            ret.type = urdf::PRISMATIC;
            ret.parent_to_joint_origin_transform = toUrdf(KDL::Frame(KDL::Rotation::Identity(),jnt.JointOrigin()));
            ret.axis = toUrdf(jnt.JointAxis());
        break;
        default: 
            lowWarn("Converting unknown joint type of joint '%s' into a fixed joint",jnt.getTypeName());
        case Joint::None:
            ret.type = urdf::FIXED;
    }
    return ret;
}

// construct inertia
urdf::Inertial toUrdf(const KDL::RigidBodyInertia i)
{
  urdf::Inertial ret;
  ret.mass = i.getMass();
  ret.origin = toUrdf(KDL::Frame(KDL::Rotation::Identity(),ret.getCOG()));
  // kdl specifies the inertia in the reference frame of the link, the urdf specifies the inertia in the inertia reference frame
  // however the kdl RigidBodyInertia constructor take the inertia with the COG as reference point, 
  // but the getInertia
  KDL::RotationalInertia Ic; 
  Ic = i.RefPoint(ret.getCOG()).getRotationalInertia();
  ret.Ixx = Ic.data[0];
  ret.Ixy = Ic.data[1];
  ret.Ixz = Ic.data[2];
  ret.Iyy = Ic.data[4];
  ret.Iyz = Ic.data[5];
  ret.Izz = Ic.data[8];
  return ret;
}

/*
bool treeFromFile(const string& file, Tree& tree)
{
  TiXmlDocument urdf_xml;
  urdf_xml.LoadFile(file);
  return treeFromXml(&urdf_xml, tree);
}

bool treeFromParam(const string& param, Tree& tree)
{
  urdf::Model robot_model;
  if (!robot_model.initParam(param)){
    ROS_ERROR("Could not generate robot model");
    return false;
  }
  return treeFromUrdfModel(robot_model, tree);
}

bool treeFromString(const string& xml, Tree& tree)
{
  TiXmlDocument urdf_xml;
  urdf_xml.Parse(xml.c_str());
  return treeFromXml(&urdf_xml, tree);
}

bool treeToXml(TiXmlDocument *xml_doc, Tree& tree)
{
    urdf::Model robot_model;
    treeToUrdfModel(
  return treeFromUrdfModel(robot_model, tree);
}
*/

bool treeToUrdfModel(const KDL::Tree& tree, const std::string & robot_name, urdf::ModelInterface& robot_model)
{
    unsigned int i = 0;
    robot_model.reset();
    robot_model.name = robot_name;

    //Add all links
    KDL::SegmentMap::iterator seg;
    KDL::SegmentMap segs;
    tree.getSegments(segs);
    for( seg = segs.begin(); seg != segs.end(); seg++ ) {        
        if (model->getLink(seg->first))
        {
            logError("link '%s' is not unique.",  seg->first.c_str());
            robot_model.reset();
            return false;
        }
        else
        { 
            boost::shared_ptr<urdf::Link> link;
            link.reset(new urdf::Link);
            
            //add name
            link->name = seg->first;
            
            //add inertial
            link->inertial.reset(new urdf::Inertial());
            *(link->inertial) = toUrdf(seg->second.getInertial())
            
            //insert link
            robot_model->links_.insert(make_pair(seg->first,link));
            logDebug("successfully added a new link '%s'", link->name.c_str());
        }
    }
    
    for( seg = segs.begin(); seg != segs.end(); seg++ ) { 
        KDL::Joint jnt;
        jnt = seg->second.getJoint();
        if (robot_model->getJoint(jnt->getName()))
        {
            logError("joint '%s' is not unique.", jnt->getName());
            robot_model.reset();
            return model;
        }
        else
        { 
            boost::shared_ptr<urdf::Joint> joint;
            joint.reset(new urdf::Joint);
            
            //convert joint
            *joint = toUrdf(jnt);
            
            //insert parent
            joint->parent_link_name = seg->second.parent->first;
            
            //insert child
            joint->child_link_name = seg->first;
            
            //insert joint
            robot_model->joints_.insert(make_pair(seg->first,link));
            logDebug("successfully added a new link '%s'", link->name.c_str());
        }
    }
    
    // every link has children links and joints, but no parents, so we create a
    // local convenience data structure for keeping child->parent relations
    std::map<std::string, std::string> parent_link_tree;
    parent_link_tree.clear();
    
    // building tree: name mapping
    try 
    {
        robot_model->initTree(parent_link_tree);
    }
    catch(ParseError &e)
    {
        logError("Failed to build tree: %s", e.what());
        robot_model.reset();
        return false;
    }
    
    // find the root link
    try
    {
        robot_model->initRoot(parent_link_tree);
    }
    catch(ParseError &e)
    {
        logError("Failed to find root link: %s", e.what());
        robot_model.reset();
        return false;
    }
        
    return true;
}

//update topology and parameters
bool treeUpdateUrdfModel(const KDL::Tree& tree, urdf::ModelInterface& robot_model)
{
    KDL::SegmentMap::iterator seg;
    KDL::SegmentMap segs;
    tree.getSegments(segs);
    
    //Update all links
    for( seg = segs.begin(); seg != segs.end(); seg++ ) {  
        boost::shared_ptr<urdf::Link> link = model->getLink(seg->first)
        if (link)
        {
            //update inertial
            link->inertial.reset(new urdf::Inertial());
            *(link->inertial) = toUrdf(seg->second.getInertial())
            
            logDebug("successfully updated link '%s'", link->name.c_str());
        }
        else
        { 
            logError("link '%s' not found.",  seg->first.c_str());
            return false;
        }
    }
    
    for( seg = segs.begin(); seg != segs.end(); seg++ ) { 
        KDL::Joint jnt;
        boost::shared_ptr<urdf::Joint> joint;
        urdf::Joint new_joint;
        jnt = seg->second.getJoint();
        joint = model->getJoint(jnt->getName());
        if (joint)
        {
            //convert joint
            new_joint = toUrdf(jnt);
            
            //update existing joint
            joint->type = new_joint.type;
            joint->axis = new_joint.axis;
            joint->parent_to_joint_origin_transform = new_joint.parent_to_joint_origin_transform;
            
            logDebug("successfully updated joint '%s'", jnt->getName());
        }
        else
        { 
            logError("joint '%s' not found.",  jnt->getName());
            return false;
        }
    }
}

}
