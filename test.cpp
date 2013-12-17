#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;

Mat calculateZ(const Mat &imgL, const Mat &imgR, int cellWidth, int cellHeight, int stepX, int stepY, int sweepRange, int zOffset)
{
    int outWidth, outHeight;
    Mat outImg, cellL, cellR, result, outImg2;
    Rect l, r;
    int sr2 = sweepRange / 2;
    int lOffset = 0, rOffset = 0;

    if (zOffset > 0)
        lOffset = zOffset;
    if (zOffset < 0)
        rOffset = -zOffset;

    if ( imgL.cols != imgR.cols || imgL.rows != imgR.rows )
    {
        printf("Input images have different sizes\n");
        return outImg;
    }

    outWidth = (imgL.cols - cellWidth - sweepRange - abs(zOffset) ) / stepX + 1;
    outHeight = (imgL.rows - cellHeight) / stepY + 1;
    outImg = Mat(outWidth, outHeight, CV_8UC1);
    outImg2 = Mat(outWidth, outHeight, CV_8UC1);
    result.create( cellWidth + sweepRange, cellHeight, CV_32FC1 );

    printf("OutWidth: %d\n", outWidth);
    printf("outHeight: %d\n", outHeight);

    for (int y = 0; y != outHeight; y++)
    {
        printf("Line %d\n", y);
        for (int x = 0; x != outWidth; x++)
        {
            //            printf("Running X: %4d, Y: %4d (W: %4d, H: %4d)\n", x, y, imgL.cols, imgL.rows);
            // Define a template
            l = Rect(x * stepX + lOffset, y * stepY, cellWidth, cellHeight);
            cellL = imgL(l);
            // Define area where we look for template
            r = Rect(x * stepX + rOffset, y * stepY, cellWidth + sweepRange, cellHeight);
            cellR = imgR(r);
            matchTemplate(cellL, cellR, result, CV_TM_CCOEFF);

            if(l.x+l.width > imgL.cols || l.y+l.height > imgL.rows || r.x+r.width > imgR.cols || r.y+r.height > imgR.rows)
            {
                printf("Dimensions exceeded\n X: %d, Y: %d\n RectL X: %d, Y: %d\n RectR X: %d, Y: %d\n", x,y,l.x,l.y,r.x,r.y);
            }

            // normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );
            // namedWindow( "Correlation result", CV_WINDOW_AUTOSIZE );
            // imshow( "Correlation result", result );
            // waitKey(0);
//            printf("X: %3d, Y: %3d ", x,y);
            double minVal; double maxVal; Point minLoc; Point maxLoc;
            Point matchLoc;
            minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
//            printf("2");
            //outImg.ptr(y)[x] = minLoc.x;
            outImg.at<char>(x,y) = maxLoc.x;
            outImg2.at<char>(x,y) = minLoc.x;
//            printf("3");
//            printf("Z: %3d, %3d\n", minLoc.x, maxLoc.x);

            // Mat showL, showR;
            // showL = imgL.clone();
            // rectangle(showL, l, Scalar(0, 0, 255));
            // namedWindow( "Left Image", CV_WINDOW_AUTOSIZE );
            // imshow( "Left Image", showL );

            // showR = imgR.clone();
            // rectangle(showR, r, Scalar(0, 0, 255));
            // namedWindow( "Right Image", CV_WINDOW_AUTOSIZE );
            // imshow( "Right Image", showR );

            namedWindow( "Z buffer", CV_WINDOW_AUTOSIZE );
            imshow( "Z buffer", outImg );
            namedWindow( "Z buffer2", CV_WINDOW_AUTOSIZE );
            imshow( "Z buffer2", outImg2 );
            // waitKey(0);
        }
    }
    printf("Calculated, returning...\n");
    return outImg;
}

int main( int argc, char **argv )
{
    Mat image, imgL, imgR, imgZ;
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
    printf("OK\n");


    sizeX = image.cols / 2;
    sizeY = image.rows;

    r = Rect(0, 0, sizeX, sizeY);
    imgL = image(r);

    r = Rect(sizeX, 0, sizeX, sizeY);
    imgR = image(r);

    imgZ = calculateZ(imgL, imgR, 50, 50, 4, 4, 50, 0);

    namedWindow( "Left", CV_WINDOW_AUTOSIZE );
    imshow( "Left", imgL );
    namedWindow( "Right", CV_WINDOW_AUTOSIZE );
    imshow( "Right", imgR );
    // namedWindow( "Display Image", CV_WINDOW_AUTOSIZE );
    // imshow( "Display Image", imgZ );

    waitKey(0);

    return 0;
}
