#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;

int main( int argc, char **argv )
{
    Mat image, imgL, imgR;
    Rect r;
    int sizeX, sizeY;

    printf( "Reading image... " );

    if ( argc != 2 )
    {
        printf( "No image file specified \n" );
        return -1;
    }

    image = imread( argv[1], 1 );

    if ( !image.data )
    {
        printf( "No image data \n" );
        return -1;
    }

    sizeX = image.cols/2;
    sizeY = image.rows;

    r = Rect(0, 0, sizeX, sizeY);
    imgL = image(r);

    r = Rect(sizeX, 0, sizeX, sizeY);
    imgR = image(r);

    namedWindow( "Display Image", CV_WINDOW_AUTOSIZE );
    imshow( "Display Image", image );

    waitKey(0);

    namedWindow( "Display Image", CV_WINDOW_AUTOSIZE );
    imshow( "Display Image", imgL );

    waitKey(0);

    namedWindow( "Display Image", CV_WINDOW_AUTOSIZE );
    imshow( "Display Image", imgR );

    waitKey(0);

    return 0;
}
