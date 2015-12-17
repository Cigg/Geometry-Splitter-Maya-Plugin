#ifndef HELLOWORLD_H
#define HELLOWORLD_H
 
#include <maya/MArgList.h>
#include <maya/MObject.h>
#include <maya/MGlobal.h>
#include <maya/MPxCommand.h>
#include <maya/MFloatArray.h>
#include <maya/MFnMesh.h>
 
class SplitFromImage : public MPxCommand {
 public:
  // Needed functions
  SplitFromImage() {};
  virtual MStatus doIt(const MArgList& argList);
  static void* creator();

  // Other functions
  MStatus addPlaneSubMesh(MObject &object, MFloatArray uPoints, MFloatArray vPoints, const MFnMesh &planeMesh);
  MStatus splitFromBinaryImage(MFnMesh &mesh, MString image);
  MStatus splitFromRGBImage(MFnMesh &mesh, MString image);
  void printSelectedObjects();
};
#endif