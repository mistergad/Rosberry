#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

static const std::string OPENCV_WINDOW = "Image_window";

class cam_test{
  public:
	cam_test(){
		VideoCapture cap(CV_CAP_ANY);
		if(!cap.isOpened()){
			cout << "Cannot open the video cam" << endl;
		}
		
		double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
		double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

		cout << "Frame size :" << dWidth << " x " << dHeight <<endl;
		namedWindow(OPENCV_WINDOW, CV_WINDOW_AUTOSIZE);

		while(1){
			Mat frame;
			bool bSuccess = cap.read(frame);

			if(!bSuccess){
				cout << "Cannot read a frame from video stream" << endl;
				break;
			}

			imshow(OPENCV_WINDOW, frame);
			
			if(waitKey(30) == 27) break;
		}
	}

	~cam_test(){
		cvDestroyWindow("Image_window");
	}
};

int main(int argc, char** argv)
{
	ros::init(argc, argv, "cam_test");
	cam_test cam_object;

	ros::spin();
	return 0;
}









	
