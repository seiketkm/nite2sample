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

/*
  depthの生データをグレイスケールの画像にする。
  CV_8UC3で返すよ
*/
cv::Mat depthToImage( nite::UserTrackerFrameRef& userFrame )
{
	cv::Mat depthImage;

    openni::VideoFrameRef depthFrame = userFrame.getDepthFrame();
    if ( depthFrame.isValid() ) {
        // Depthの情報をdepthImageに
        openni::VideoMode videoMode = depthFrame.getVideoMode();
        depthImage = cv::Mat( videoMode.getResolutionY(),
                              videoMode.getResolutionX(),
                              CV_16SC1,
							  (short*) depthFrame.getData() );
		// depthImageを表示向きのCV_8UC3に変更。
        depthImage.convertTo(depthImage, CV_8UC1, 255.0/10000);
        cv::cvtColor(depthImage, depthImage, CV_GRAY2BGR);
	}
	return depthImage;
}

void drawUser( nite::UserTrackerFrameRef& userFrame, cv::Mat& depthImage )
{
	openni::VideoFrameRef depthFrame = userFrame.getDepthFrame();
	if ( depthFrame.isValid() ) {
		openni::VideoMode videoMode = depthFrame.getVideoMode();
		// cv::MatでUserMapを取得する
		cv::Mat pMapLabel = cv::Mat( videoMode.getResolutionY(),
										videoMode.getResolutionX(),
										CV_16SC1, 
										(short*) userFrame.getUserMap().getPixels());
		pMapLabel.convertTo(pMapLabel,CV_8UC1);
		// 見つけた人に色をつけるよ
		for(int i = 0; i < 6; i++){
			cv::Mat mask;
			cv::compare(pMapLabel, i+1, mask, CV_CMP_EQ);
			cv::add(depthImage, colors[i], depthImage, mask);
		}
	}
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

			depthImage = depthToImage( userFrame );
            drawUser( userFrame, depthImage );

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

