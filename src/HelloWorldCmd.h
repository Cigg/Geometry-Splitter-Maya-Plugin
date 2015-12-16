#ifndef HELLOWORLD_H
#define HELLOWORLD_H
 
#include <maya/MArgList.h>
#include <maya/MObject.h>
#include <maya/MGlobal.h>
#include <maya/MPxCommand.h>
#include <maya/MFloatArray.h>
#include <maya/MFnMesh.h>
 
class HelloWorld : public MPxCommand {
 public:
  // Needed functions
  HelloWorld() {};
  virtual MStatus doIt(const MArgList& argList);
  static void* creator();

  // Other functions
  MObject createCube(float cubeSize);
  MStatus addPlaneSubMesh(MObject &object, MFloatArray uPoints, MFloatArray vPoints, const MFnMesh &planeMesh);
  MStatus splitFromImage(MFnMesh &mesh, MString image);
  void printSelectedObjects();
};
#endif