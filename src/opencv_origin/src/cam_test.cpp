#include <stdio.h>
#include <iostream>

#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/xfeatures2d.hpp"

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

static const std::string IMAGE = "images/book.png";

void readme()
  { std::cout << " Usage: ./SURF_descriptor <img1> <img2>" << std::endl; }



int main(int argc, char** argv)
{
	ros::init(argc, argv, "cam_test");
	
	//////// OpenCV code //////////	
	
	Mat img_object = imread( IMAGE, IMREAD_GRAYSCALE );

	VideoCapture cap(0);

	int fps = 60;
	cap.set(CV_CAP_PROP_FPS, fps);

	if(!cap.isOpened())
	{
		cout << "Error opening video stream!" << endl;
		return -1;
	}

	while(1)
	{
		Mat scene;
		cap >> scene;
		
		Mat img_scene;
		cvtColor(scene, img_scene, COLOR:BGR2GRAY);

		if( !img_object.data || !img_scene.data ) continue; // should be finalized
		
		//-- Step 1: Detect the keypoints and extract descriptors using SURF
		  int minHessian = 400;
		  Ptr<SURF> detector = SURF::create( minHessian );
		  std::vector<KeyPoint> keypoints_object, keypoints_scene;
		  Mat descriptors_object, descriptors_scene;
		  detector->detectAndCompute( img_object, Mat(), keypoints_object, descriptors_object );
		  detector->detectAndCompute( img_scene, Mat(), keypoints_scene, descriptors_scene );
		  //-- Step 2: Matching descriptor vectors using FLANN matcher
		  FlannBasedMatcher matcher;
		  std::vector< DMatch > matches;
		  matcher.match( descriptors_object, descriptors_scene, matches );
		  double max_dist = 0; double min_dist = 100;
		  //-- Quick calculation of max and min distances between keypoints
		  for( int i = 0; i < descriptors_object.rows; i++ )
		  { double dist = matches[i].distance;
		    if( dist < min_dist ) min_dist = dist;
		    if( dist > max_dist ) max_dist = dist;
		  }
		  //printf("-- Max dist : %f \n", max_dist );
		  //printf("-- Min dist : %f \n", min_dist );
		  //-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
		  std::vector< DMatch > good_matches;
		  for( int i = 0; i < descriptors_object.rows; i++ )
		  { if( matches[i].distance <= 3*min_dist )
		     { good_matches.push_back( matches[i]); }
		  }
		  Mat img_matches;
		  drawMatches( img_object, keypoints_object, img_scene, keypoints_scene,
			       good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
			       std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
		  //-- Localize the object
		  std::vector<Point2f> obj;
		  std::vector<Point2f> scene;
		  for( size_t i = 0; i < good_matches.size(); i++ )
		  {
		    //-- Get the keypoints from the good matches
		    obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
		    scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
		  }
		  Mat H = findHomography( obj, scene, RANSAC );
		  //-- Get the corners from the image_1 ( the object to be "detected" )
		  std::vector<Point2f> obj_corners(4);
		  obj_corners[0] = cvPoint(0,0);
		  obj_corners[1] = cvPoint( img_object.cols, 0 );
		  obj_corners[2] = cvPoint( img_object.cols, img_object.rows );
		  obj_corners[3] = cvPoint( 0, img_object.rows );
		  std::vector<Point2f> scene_corners(4);
		  perspectiveTransform( obj_corners, scene_corners, H);
		  //-- Draw lines between the corners (the mapped object in the scene - image_2 )
		  line( img_matches, scene_corners[0] + Point2f( img_object.cols, 0), scene_corners[1] + Point2f( img_object.cols, 0), Scalar(0, 255, 0), 4 );
		  line( img_matches, scene_corners[1] + Point2f( img_object.cols, 0), scene_corners[2] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
		  line( img_matches, scene_corners[2] + Point2f( img_object.cols, 0), scene_corners[3] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
		  line( img_matches, scene_corners[3] + Point2f( img_object.cols, 0), scene_corners[0] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
	  //-- Show detected matches
	  imshow( "Good Matches & Object detection", img_matches );
		
		char c = (char)waitKey(1);
		if(c == 27) break; // ESC
	}
	
	cap.release();
	destroyAllWindows();
	///////////////////////////////

	ros::spin();
	return 0;
}









	
