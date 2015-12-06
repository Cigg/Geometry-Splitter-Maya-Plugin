#include "HelloWorldCmd.h"
#include <maya/MFnPlugin.h>

#include <maya/MFnMesh.h>
#include <maya/MPoint.h>
#include <maya/MFloatPoint.h>
#include <maya/MFloatPointArray.h>
#include <maya/MSelectionList.h>
 
void* HelloWorld::creator() { return new HelloWorld; }

MObject HelloWorld::createCube(float cubeSize) {
  int numVertices;
  MFloatPointArray points;
  MFnMesh meshFS;

  const int numFaces          = 6;
  numVertices                 = 8;
  const int numFaceConnects   = 24;
  MFloatPoint vtx_1( -cubeSize, -cubeSize, -cubeSize );
  MFloatPoint vtx_2(  cubeSize, -cubeSize, -cubeSize );
  MFloatPoint vtx_3(  cubeSize, -cubeSize,  cubeSize );
  MFloatPoint vtx_4( -cubeSize, -cubeSize,  cubeSize );
  MFloatPoint vtx_5( -cubeSize,  cubeSize, -cubeSize );
  MFloatPoint vtx_6( -cubeSize,  cubeSize,  cubeSize );
  MFloatPoint vtx_7(  cubeSize,  cubeSize,  cubeSize );
  MFloatPoint vtx_8(  cubeSize,  cubeSize, -cubeSize );
  points.append( vtx_1 );
  points.append( vtx_2 );
  points.append( vtx_3 );
  points.append( vtx_4 );
  points.append( vtx_5 );
  points.append( vtx_6 );
  points.append( vtx_7 );
  points.append( vtx_8 );

  // Set up an array containing the number of vertices
  // for each of the 6 cube faces (4 verticies per face)
  int face_counts[numFaces] = { 4, 4, 4, 4, 4, 4 };
  MIntArray faceCounts( face_counts, numFaces );
  // Set up and array to assign vertices from points to each face 
  int face_connects[ numFaceConnects ] = {    0, 1, 2, 3,
                                              4, 5, 6, 7,
                                              3, 2, 6, 5,
                                              0, 3, 5, 4,
                                              0, 4, 7, 1,
                                              1, 7, 6, 2  };

  MIntArray faceConnects( face_connects, numFaceConnects );
  MObject newMesh = meshFS.create(numVertices, numFaces,
                                  points, faceCounts, faceConnects);

  return newMesh;
}

MStatus HelloWorld::doIt(const MArgList& argList) {
  //MString str = "Hello " + argList.asString(0);
  //MGlobal::displayInfo(str.asChar());

  MSelectionList selected;
  MGlobal::getActiveSelectionList(selected);
  for( int i=0; i<selected.length(); ++i ) {
    MObject obj;

    // returns the i'th selected dependency node
    selected.getDependNode(i,obj);

    // Attach a function set to the selected object
    MFnDependencyNode fn(obj);

    // write the object name to the script editor
    MGlobal::displayInfo( fn.name().asChar() );
  }

  //MObject cubeMesh = createCube(1.0f);

  return MS::kSuccess;
}

MStatus initializePlugin(MObject obj) {
  MFnPlugin plugin(obj, "Mikael & Teo", "0.1", "Any"); // (obj, vendor, version, requiredApiVersion)
  MStatus status = plugin.registerCommand("helloWorld", HelloWorld::creator);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  return status;
}
 
MStatus uninitializePlugin(MObject obj) {
  MFnPlugin plugin(obj);
  MStatus status = plugin.deregisterCommand("helloWorld");
  CHECK_MSTATUS_AND_RETURN_IT(status);
  return status;
}