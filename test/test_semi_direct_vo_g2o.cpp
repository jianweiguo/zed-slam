
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <boost/timer.hpp>
#include <algorithm>

#include "core/Config.h"
#include "core/Feature.h"
#include "core/Featuredetection.h"
#include "core/Tracker.h"
#include "io/Zed_svo_wrapper.h"
#include "utils/SLAMTools.h"

using namespace std;

int main ( int argc, char** argv )
{
    if ( argc != 3 )
    {
        cout<<"usage: test_semi_direct_vo_g2o config_file svo_file"<<endl;
        return -1;
    }

    string path_to_config ( argv[1] );
    string svo_file ( argv[2] );

    zed_slam::Config::init ( path_to_config );
    zed_slam::Zed_svo_wrapper* svo_wrapper = new zed_slam::Zed_svo_wrapper ( svo_file );
    
    // remove the first frames where the camera does not move
    for (int i=0; i<40; i++)
    {
        svo_wrapper->next();
    }

    zed_slam::Frame::Ptr frame1 = svo_wrapper->next();
    if ( frame1 == nullptr )
        return -1;

    zed_slam::FeatureDetection featureDetection;
    featureDetection.detectFAST ( frame1 );

    zed_slam::Tracker tracker;
    zed_slam::Frame::Ptr frame2 = svo_wrapper->next();
    
    cv::imwrite("first.png", frame1->color_ );
    cv::imwrite("second.png", frame2->color_ );
    
    boost::timer timer;
    tracker.poseEstimationTwoFramesSemiDirectUsingG2O( frame1, frame2 );
    cout<<"semi direct cost time="<<timer.elapsed()<<endl;
    
    frame2->T_f_w_ = frame1->T_f_w_;
    timer.restart();
    tracker.poseEstimationTwoFramesDirectUsingG2o( frame1, frame2 );
    cout<<"direct cost time="<<timer.elapsed()<<endl;

    zed_slam::SLAMTools::drawFeatures(frame1);
    zed_slam::SLAMTools::drawFeaturesReprojection(frame2, "frame2" );
    
    cv::waitKey(0);
    return 0;
}
