#ifndef HELLOWORLD_H
#define HELLOWORLD_H
 
#include <maya/MArgList.h>
#include <maya/MObject.h>
#include <maya/MGlobal.h>
#include <maya/MPxCommand.h>
 
class HelloWorld : public MPxCommand {
 public:
  // Needed functions
  HelloWorld() {};
  virtual MStatus doIt(const MArgList& argList);
  static void* creator();

  // Other functions
  MObject createCube(float cubeSize);
};
#endif