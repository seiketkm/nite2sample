#include<iostream>
#include<NiTE.h>
#include<opencv2\opencv.hpp>

cv::Scalar colors[] = {
    cv::Scalar( 0xFF,    0,    0 ),
    cv::Scalar(    0, 0xFF,    0 ),
    cv::Scalar(    0,    0, 0xFF ),
    cv::Scalar( 0xFF, 0xFF,    0 ),
    cv::Scalar( 0xFF,    0, 0xFF ),
    cv::Scalar(    0, 0xFF, 0xFF ),
};

cv::Mat drawUser( nite::UserTrackerFrameRef& userFrame )
{
    cv::Mat depthImage;

    openni::VideoFrameRef depthFrame = userFrame.getDepthFrame();
    if ( depthFrame.isValid() ) {
		// Depth‚Ìî•ñ‚ğdepthImage‚É
        openni::VideoMode videoMode = depthFrame.getVideoMode();
        depthImage = cv::Mat( videoMode.getResolutionY(),
                                videoMode.getResolutionX(),
								CV_16SC1, (short*) depthFrame.getData() );
		depthImage.convertTo(depthImage, CV_8UC1, 255.0/10000);
		cv::cvtColor(depthImage, depthImage, CV_GRAY2BGR);
        
		// cv::Mat‚ÅUserMap‚ğæ“¾‚·‚é
		cv::Mat pMapLabel = cv::Mat( videoMode.getResolutionY(),
									videoMode.getResolutionX(),
									CV_16SC1, (short*) userFrame.getUserMap().getPixels());
		pMapLabel.convertTo(pMapLabel,CV_8UC1);
		// Œ©‚Â‚¯‚½l‚ÉF‚ğ‚Â‚¯‚é‚æ
		for(int i = 0; i < 6; i++){
			cv::Mat mask;
			cv::compare(pMapLabel, i+1, mask, CV_CMP_EQ);
			cv::add(depthImage, colors[i], depthImage, mask);
		}
    }

    return depthImage;
}


void main(int argc, char* argv[])
{
	try{
		auto status = nite::NiTE::initialize();
		nite::UserTracker userTracker;
		status = userTracker.create();
        if ( status != nite::STATUS_OK ) {
            throw std::runtime_error( "userTracker.create" );
        }
		
		cv::Mat depthImage;

        while ( 1 ) {
            nite::UserTrackerFrameRef userFrame;
            userTracker.readFrame( &userFrame );

            depthImage = drawUser( userFrame );

            cv::imshow( "User", depthImage );

            int key = cv::waitKey( 10 );
            if ( key == 'q' || key == 0x1b ) {
                break;
            }
        }
	}
	catch(std::exception&){
		std::cout << openni::OpenNI::getExtendedError() << std::endl;
	}
}

