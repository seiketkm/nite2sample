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

void drawUser( nite::UserTrackerFrameRef& userFrame, cv::Mat& image )
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
            cv::add(image, colors[i], image, mask);
        }
    }
}
void drawSkeleton( nite::UserTrackerFrameRef& userFrame,
                   nite::UserTracker& userTracker,
                   cv::Mat& image)
{
    const nite::Array<nite::UserData>& users = userFrame.getUsers();
    for ( int i = 0; i < users.getSize(); ++i ) {
        const nite::UserData& user = users[i];
        if ( user.isNew() ) {
            userTracker.startSkeletonTracking( user.getId() );
            userTracker.startPoseDetection( user.getId(), nite::POSE_PSI);
            userTracker.startPoseDetection( user.getId(), nite::POSE_CROSSED_HANDS);
        }
        else if ( !user.isLost() ) {
            // skeletonの表示
            const auto skeleton = user.getSkeleton();
            if ( skeleton.getState() == nite::SkeletonState::SKELETON_TRACKED ) {
                for ( int j = 0; j < 15; j++ ) {
                    const auto joint = skeleton.getJoint((nite::JointType)j);
                    if ( joint.getPositionConfidence() >= 0.7f ) {
                        const auto position = joint.getPosition();
                        float x = 0, y = 0;
                        userTracker.convertJointCoordinatesToDepth(
                            position.x, position.y, position.z, &x, &y );

                        cv::circle( image, cvPoint( (int)x, (int)y ),
                            3, cv::Scalar( 0, 0, 255 ), 5 );
                    }
                }
            }
            // poseの表示
            const auto pose_psi = user.getPose(nite::POSE_PSI);
            if( pose_psi.isHeld() || pose_psi.isEntered() )
            {
                auto center = user.getCenterOfMass();
                float x = 0, y = 0;
                userTracker.convertJointCoordinatesToDepth(center.x, center.y, center.z, &x, &y);
                cv::putText(image, "PSI", cv::Point2f(x,y), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0xFF,0xFF,0xFF));
            }
            const auto pose_cross = user.getPose(nite::POSE_CROSSED_HANDS);
            if( pose_cross.isHeld() || pose_cross.isEntered() ){
                auto center = user.getCenterOfMass();
                float x = 0, y = 0;
                userTracker.convertJointCoordinatesToDepth(center.x, center.y, center.z, &x, &y);
                cv::putText(image, "Cross", cv::Point2f(x,y), cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0xFF,0xFF,0xFF));
            }
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
            drawSkeleton( userFrame, userTracker, depthImage);
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

