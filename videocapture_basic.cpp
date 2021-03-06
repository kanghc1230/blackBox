#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>

using namespace cv; //cv::
using namespace std;

int main(int, char**)
{
    // 카메라에서 이미지 한장을 저장하기위한 이미지 객체class 선언
    Mat frame;
    // 카메라를 선택(열기)하기 위한 객체 cap은 fd=fopen의 fd와 동일
    VideoCapture cap;

    // 카메라 개수가 중가하면 1씩 증가하는 int ID변수
    int deviceID = 0;
    int apiID = cv::CAP_V4L2;

    // STEP 1. cap.open() 카메라 장치열기
    cap.open(deviceID, apiID); //devicID증가
    // 카메라가 정상적으로 열리지 못하면
    if (!cap.isOpened()) {
        perror("ERROR! Unable to open camera\n");
        return -1;
    }

    // 그래핑을 시작한다.
    printf("Start grabbing\n");
    printf("Press any key to terminate\n");
    // STEP 2. cap.read() 카메라 읽기
    while(1)
    {
        // 카메라에서 매 프레임 마다 이미지를 frame에 저장
        cap.read(frame);
        // 카메라가 정상적으로 동작하는가 (이미지 읽기 실패시) ex)usb가 뽑혔을때 
        if (frame.empty()) {
            perror("ERROR! blank frame grabbed\n");
            break;
        }

        // STEP 3. imshow() 읽어온 이미지를 창을열고 띄우기
        // "Live"라는 창을 생성하고 frame에 저장된 이미지를 띄운다.
        imshow("Live", frame);
        // 이미지 초당 waiteKey(5ms)마다 대기함수. 커질수록 프레임떨어진다.
        // waitKey() 괄호안이 비어있으면 키보드입력까지 계속대기
        if (waitKey(5) >= 0) // 키입력시 키의 양수값 리턴하고 break함
            break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
