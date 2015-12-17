#include "SplitFromImageCmd.h"
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
 
void* SplitFromImage::creator() { return new SplitFromImage; }

// Maps vertex points in uv coordinates (uPoints and vPoints) onto plane and creates a new mesh
// Only works with planes with the local normal along the y-axis for now (default Maya polyplane)
MStatus SplitFromImage::addPlaneSubMesh(MObject &object, MFloatArray uPoints, MFloatArray vPoints, const MFnMesh &planeMesh) {
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

MStatus SplitFromImage::splitFromBinaryImage(MFnMesh &mesh, MString image) {
	cout << "Image filename: " << image.asChar() << endl;

	// Load image
	cv::Mat img;
	img = cv::imread(image.asChar(), CV_LOAD_IMAGE_GRAYSCALE);
	if (!img.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << endl;
		return MS::kFailure;
	}

	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	// Find contours
	cv::findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	for (int i = 0; i < contours.size(); i++) {
		MFloatArray uPoints;
		MFloatArray vPoints;

		for (int j = 0; j < contours[i].size(); j++) {
			cv::Point pt = contours[i][j];
			float u = (float)pt.x / img.cols;
			float v = (float)pt.y / img.rows;
			uPoints.append(u);
			vPoints.append(v);
		}

		MObject newMesh;
		if (addPlaneSubMesh(newMesh, uPoints, vPoints, mesh) == MS::kSuccess)
			cout << "Added plane sub mesh!" << endl;
		else
			cout << "Couldn't add plane sub mesh!" << endl;
 	}

	return MS::kSuccess;
}

// We try to do some image processing on a normal rgb image before finding the contours.
// An example when this could be useful is if we want to separate all bricks in a brick wall.
// We can do this by using the texture of the brick wall as input.
// However it doesnt work that well :(
MStatus SplitFromImage::splitFromRGBImage(MFnMesh &mesh, MString image) {
	cout << "Image filename: " << image.asChar() << endl;

	// Load image
	cv::Mat img, imgGray;
	img = cv::imread(image.asChar(), CV_LOAD_IMAGE_COLOR);
	if (!img.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << endl;
		return MS::kFailure;
	}

	int thresh = 100;
	int max_thresh = 255;
	cv::RNG rng(12345);

	// Convert image to gray and blur it
	cv::cvtColor(img, imgGray, CV_BGR2GRAY);
	cv::blur(imgGray, imgGray, cv::Size(3, 3));

	cv::Mat canny_output;
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;

	// Detect edges using canny
	cv::Canny(imgGray, canny_output, thresh, thresh * 2, 3);
	// Find contours
	cv::findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	for (int i = 0; i < contours.size(); i++) {
		MFloatArray uPoints;
		MFloatArray vPoints;

		for (int j = 0; j < contours[i].size(); j++) {
			cv::Point pt = contours[i][j];
			float u = (float)pt.x / img.cols;
			float v = (float)pt.y / img.rows;
			uPoints.append(u);
			vPoints.append(v);
		}

		MObject newMesh;
		if (addPlaneSubMesh(newMesh, uPoints, vPoints, mesh) == MS::kSuccess)
			cout << "Added plane sub mesh!" << endl;
		else
			cout << "Couldn't add plane sub mesh!" << endl;
	}

	return MS::kSuccess;
}

MStatus SplitFromImage::doIt(const MArgList& args) {
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

		//MPointArray points;
		//mesh.getPoints(points);
		//MFloatArray uPoints;
		//MFloatArray vPoints;
		//mesh.getUVs(uPoints, vPoints);
		//for (unsigned int i = 0; i < uPoints.length(); i++) {
		//	cout << "uPoints[" << i << "]: " << uPoints[i] << endl;
		//}
        //for(unsigned int i = 0; i < points.length(); i++) {
        //  cout << "Points[" << i << "]: " << points[i] << endl;
        //}

		if (splitFromBinaryImage(mesh, args.asString(0)) == MS::kSuccess) {
			cout << "Splitted image!" << endl;
		}
		else {
			cout << "Could not split image!" << endl;
		}
      }
      else {
        cout << "Selected object has no MDagPath!" << endl << endl;
      }
  }

  return MS::kSuccess;
}

MStatus initializePlugin(MObject obj) {
  MFnPlugin plugin(obj, "Mikael & Teo", "1.0", "Any"); // (obj, vendor, version, requiredApiVersion)
  MStatus status = plugin.registerCommand("splitFromImage", SplitFromImage::creator);
  CHECK_MSTATUS_AND_RETURN_IT(status);
  return status;
}
 
MStatus uninitializePlugin(MObject obj) {
  MFnPlugin plugin(obj);
  MStatus status = plugin.deregisterCommand("splitFromImage");
  CHECK_MSTATUS_AND_RETURN_IT(status);
  return status;
}