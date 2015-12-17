#include "HelloWorldCmd.h"
#include <maya/MFnPlugin.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <maya/MPoint.h>
#include <maya/MFloatPoint.h>
#include <maya/MFloatPointArray.h>
#include <maya/MPointArray.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MDagPath.h>
 
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

// Maps vertex points in uv coordinates (uPoints and vPoints) onto plane and creates a new mesh
// Only works with planes with the normal along the y-axis for now.
MStatus HelloWorld::addPlaneSubMesh(MObject &object, MFloatArray uPoints, MFloatArray vPoints, const MFnMesh &planeMesh) {
  if(uPoints.length() != vPoints.length())
    return MS::kFailure;

  MPointArray planePoints;
  MIntArray planeVertexCount;
  MIntArray planeVertexList;
  planeMesh.getPoints(planePoints);
  planeMesh.getVertices(planeVertexCount, planeVertexList);

  cout << "planeVertexCount: " << planeVertexCount.length() << endl;
  cout << "planeVertexList: " << planeVertexList.length() << endl;

  double minX, minZ, maxX, maxZ;
  minX = minZ = 100000;
  maxX = maxZ = -100000;

  // Find min and max points of the plane
  for(unsigned int i = 0; i < planePoints.length(); i++) {
	  if (i == 0 || i == planePoints.length() - 1){
		  // cout << "point[" << i << "]" << planePoints[i].x << endl;
	  }
    double x = planePoints[i].x;
    double z = planePoints[i].z;
    if(planePoints[i].x < minX)
      minX = x;
    if(planePoints[i].x > maxX)
      maxX = x;
    if(planePoints[i].z < minZ)
      minZ = x;
    if(planePoints[i].z > maxZ)
      maxZ = z;
  }

  // Set plane's corner pos and width and height
  double planeX = minX;
  double planeY = minZ;
  double planeWidth = maxX - minX;
  double planeHeight = maxZ - minZ;

  // Prepare stuff for MFnMesh
  int numVertices = uPoints.length();
  int numPolygons = 1;
  MPointArray pointArray;
  int polygon_counts[1] = {numVertices};
  MIntArray polygonCounts(polygon_counts, 1);
  int *polygon_connects = new int[numVertices];

  for(int i = 0; i < numVertices; i++) {
    polygon_connects[i] = i;
    MPoint point(planeX + planeWidth * uPoints[i], 0, planeY + planeHeight * vPoints[i]);
    pointArray.append(point);
  }

  MIntArray polygonConnects(polygon_connects, numVertices);
  delete[] polygon_connects;

  // cout << "numVertices: " << numVertices << endl
  //      << "numPolygons: " << numPolygons << endl
  //      << "pointArray: " << pointArray << endl
  //      << "polygonCounts: " << polygonCounts << endl
  //      << "polygonConnects: " << polygonConnects << endl
  //      << "planeX: " << planeX << endl
  //      << "planeY: " << planeY << endl
  //      << "planeWidth: " << planeWidth << endl
  //      << "planeHeight: " << planeHeight << endl;

  for (int i = 0; i < vPoints.length(); i++){
	  vPoints[i] = 1.0 - vPoints[i];
  }

  MFnMesh mesh;
  object = mesh.create(numVertices, numPolygons, pointArray, polygonCounts, polygonConnects, uPoints, vPoints);
  mesh.assignUVs(polygonCounts, polygonConnects);

  return MS::kSuccess;
}

MStatus HelloWorld::splitFromImage(MFnMesh &mesh, MString image) {
	cv::Mat img, imgGray;
	// std::string str = image.asChar();
	cout << "Image filename: " << image.asChar() << endl;
	img = cv::imread(image.asChar(), 1);
	if (!img.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << endl;
		return MS::kFailure;
	}

	int thresh = 100;
	int max_thresh = 255;
	cv::RNG rng(12345);

	/// Load source image and convert it to gray

	/// Convert image to gray and blur it
	cv::cvtColor(img, imgGray, CV_BGR2GRAY);
	// cv::blur(imgGray, imgGray, cv::Size(3, 3));

	cv::Mat canny_output;
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;

	/// Detect edges using canny
	cv::Canny(imgGray, canny_output, thresh, thresh * 2, 3);
	/// Find contours
	cout << "Before find contours" << endl;
	cv::findContours(canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	cout << "After find contours" << endl;


	for (int i = 0; i < contours.size(); i++) {
		MFloatArray uPoints;
		MFloatArray vPoints;

		// std::cout << "contours[" << i << "]" << std::endl;
		for (int j = 0; j < contours[i].size(); j++) {
			//std::cout << "   contours[" << i << "][" << j << "]" << std::endl;
			cv::Point pt = contours[i][j];
			float u = (float)pt.x / img.cols;
			float v = (float)pt.y / img.rows;
			uPoints.append(u);
			vPoints.append(v);
			//std::cout << "       " << pt.x << ", " << pt.y << std::endl;
			//std::cout << "       " << u << ", " << v << std::endl;
		}

		MObject newMesh;
		if (addPlaneSubMesh(newMesh, uPoints, vPoints, mesh) == MS::kSuccess)
			cout << "Added plane sub mesh!" << endl;
		else
			cout << "Couldn't add plane sub mesh!" << endl;
 	}

	return MS::kSuccess;
}

MStatus HelloWorld::doIt(const MArgList& args) {
  //MString str = "Hello " + argList.asString(0);
  //MGlobal::displayInfo(str.asChar());
	
  MSelectionList list;
  MGlobal::getActiveSelectionList(list);

  MObject node;
  MFnDependencyNode depFn;
  MStringArray types;
  MString name;
  MDagPath dag;
  
  for( MItSelectionList iter( list ); !iter.isDone(); iter.next() ) {
      // Print the types and function sets of all of the objects
      iter.getDependNode( node );
      depFn.setObject( node );
      
      name = depFn.name();
      MGlobal::getFunctionSetList( node, types );
      cout << "Name: " << name.asChar() << endl;
      cout << "Type: " << node.apiTypeStr() << endl;
	  
	  /*cout << "Function Sets: ";
	  
      for(unsigned int i = 0; i < types.length(); i++ ) {
          if ( i > 0 ) cout << ", ";
          cout << types[i].asChar();
      }
      cout << endl << endl;*/

      // Check if object has a MDagPath
      if(iter.getDagPath( dag ) == MS::kSuccess) {
      
        // Get the MFnMesh from the MDagPath
        dag.extendToShape();
        MFnMesh mesh(dag.node());

        MPointArray points;
        mesh.getPoints(points);

		MFloatArray uPoints;
		MFloatArray vPoints;
		mesh.getUVs(uPoints, vPoints);
		for (unsigned int i = 0; i < uPoints.length(); i++) {
			cout << "uPoints[" << i << "]: " << uPoints[i] << endl;
		}
        /*for(unsigned int i = 0; i < points.length(); i++) {
          cout << "Points[" << i << "]: " << points[i] << endl;
        }*/

		if (splitFromImage(mesh, args.asString(0)) == MS::kSuccess) {
			cout << "Splitted image!" << endl;
		}
		else {
			cout << "Could not split image!" << endl;
		}

        /*const float u[4] = {0, 0, 0.5, 0.5};
        const float v[4] = {0, 0.5, 0.5, 0};
        MFloatArray uPoints(u, 4);
        MFloatArray vPoints(v, 4);

        MObject newMesh;
        if(addPlaneSubMesh(newMesh, uPoints, vPoints, mesh) == MS::kSuccess)
          cout << "Added plane sub mesh!" << endl;
        else
          cout << "Couldn't add plane sub mesh!" << endl;*/

        // TODO: Copy transform

        //Doing random stuff to mesh
        // int f[1] = {0};
        // MIntArray faces(f, 1);
        // mesh.subdivideFaces(faces, 1);
        // mesh.updateSurface();

        // MPoint p[3] = {MPoint(-0.5, 0.0, 0.5), MPoint(0.0, 0.0, 1.0), MPoint(0.5, 0.0, 0.5)};
        // MPointArray newPolygonPoints(p, 3);
        // mesh.addPolygon(newPolygonPoints);
      }
      else {
        cout << "Selected object has no MDagPath!" << endl << endl;
      }
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