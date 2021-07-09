#include "opencv4/opencv2/opencv.hpp" //opencv안에 cv
//#include <iostream>
// vim build 파일
// g++ -o main main.cpp $(pkg-config opencv4 --libs --cflags)
#include <stdio.h>

using namespace cv;

int main(void)
{
    printf("Hello opencv\n");
    // 이미지를 저장하기위한 Mat Class 이미지 객체 선언 (매트릭스 클래스)
    Mat img; 

    // imread( , ) : 모든 이미지들 불러오는 함수 /home/pi/blackBox/lenna.bmp
    // 2번째인자 1(default)는 칼라 0이면 흑백 
    img = imread("lenna.bmp");
    // 이미지 파일을 읽었는데 Mat img가 비어있다면
    if (img.empty())
    {
        perror ("Image load failed\n");
        return -1;
    }

    // 새로운 창을 생성, 창의 이름은 "image"
    namedWindow("image");
    // "image"창에 img를 띄우는 함수
    imshow("image", img); 
    // waitKey()괄호안에 아무런 값도 없으면 아무키나 입력받기전까지 대기
    waitKey();
    
    return 0;
}