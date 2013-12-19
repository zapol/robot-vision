#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;

typedef struct calcSettings_t
{
    Mat imgL;
    Mat imgR;
    int cellWidth;
    int cellHeight;
    int stepX;
    int stepY;
    int sweepRange;
    int zOffset;
    int method;
    int scale;
    int cutoff;
    int reversed;
    //calcSettings_t(Mat &a1, Mat &a2) : imgL(a1), imgR(a2) {}
} calcSettings;

typedef struct calcResult_t
{
    Mat imgMin;
    Mat imgMax;
    //calcResult_t(Mat &a1, Mat &a2) : imgMin(a1), imgMax(a2) {}
} calcResult;

typedef struct
{
    double minVal;
    double maxVal;
    uint8_t minLoc;
    uint8_t maxLoc;
} pixelResult;

void generateCallback(int, void *);
//Mat calculateZMat(const Mat &imgL, const Mat &imgR, int cellWidth, int cellHeight, int stepX, int stepY, int sweepRange, int zOffset, int method);
pixelResult calculateZPixel(int x, int y, calcSettings &s, bool do_output);
calcResult calculateZMat(calcSettings &s);
void onMouseOnLeftWin( int event, int x, int y, int, void *);
void onMouseGetDetails( int event, int x, int y, int, void *);

Mat outImgMin, outImgMax, image;

pixelResult calculateZPixel(int x, int y, calcSettings &s, bool do_output = false)
{
    Rect l, r;
    Mat cellL, cellR, result;

    int lOffset = 0, rOffset = 0;
    if (s.zOffset > 0)
        lOffset = s.zOffset;
    if (s.zOffset < 0)
        rOffset = -s.zOffset;

    l = Rect(x * s.stepX + lOffset, y * s.stepY, s.cellWidth, s.cellHeight);
    cellL = s.imgL(l);
    // Define area where we look for template
    r = Rect(x * s.stepX + rOffset, y * s.stepY, s.cellWidth + s.sweepRange, s.cellHeight);
    cellR = s.imgR(r);
    matchTemplate(cellL, cellR, result, s.method);

    if (l.x + l.width > s.imgL.cols || l.y + l.height > s.imgL.rows || r.x + r.width > s.imgR.cols || r.y + r.height > s.imgR.rows)
    {
        printf("Dimensions exceeded\n X: %d, Y: %d\n RectL X: %d, Y: %d\n RectR X: %d, Y: %d\n", x, y, l.x, l.y, r.x, r.y);
    }

    Point minLoc; Point maxLoc;
    pixelResult res;
    minMaxLoc( result, &(res.minVal), &(res.maxVal), &minLoc, &maxLoc, Mat() );
    res.minLoc = minLoc.x;
    res.maxLoc = maxLoc.x;

    if (do_output)
    {
        Mat result_pad;
        normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );
        copyMakeBorder(result, result_pad, 1, 0, 0, 0, BORDER_CONSTANT, Scalar(0));
        result_pad.at<float>(0, minLoc.x) = 1;
        result_pad.at<float>(0, maxLoc.x) = 1;

        float scale = 100 / cellL.rows;
        resize(cellL, cellL, Size(0, 0), scale, scale);
        resize(cellR, cellR, Size(0, 0), scale, scale);
        resize(result_pad, result_pad, Size(0, 0), scale, scale, INTER_NEAREST);
        imshow( "DetailLeft", cellL );
        imshow( "DetailRight", cellR );
        imshow( "DetailMatch", result_pad );
    }
    return res;
}

//Mat calculateZMat(const Mat &imgL, const Mat &imgR, int cellWidth, int cellHeight, int stepX, int stepY, int sweepRange, int zOffset, int method, int cutoff)
calcResult calculateZMat(calcSettings &s)
{
    int outWidth, outHeight;
    Rect l, r;
    int sr2 = s.sweepRange / 2;
    int lOffset = 0, rOffset = 0;
    Mat result;

    printf("Done\n");

    calcResult res;//(Mat(), Mat());

    if (s.zOffset - 100 > 0)
        lOffset = s.zOffset;
    if (s.zOffset - 100 < 0)
        rOffset = -s.zOffset;

    if ( s.imgL.cols != s.imgR.cols || s.imgL.rows != s.imgR.rows )
    {
        printf("Input images have different sizes\n");
        return res;
    }

    outWidth = (s.imgL.cols - s.cellWidth - s.sweepRange - abs(s.zOffset) ) / s.stepX + 1;
    outHeight = (s.imgL.rows - s.cellHeight) / s.stepY + 1;
    res.imgMax = Mat(outHeight, outWidth, CV_8UC1);
    res.imgMin = Mat(outHeight, outWidth, CV_8UC1);
    result.create(s.cellHeight,  s.cellWidth + s.sweepRange, CV_32FC1 );

    printf("OutWidth: %d\n", outWidth);
    printf("outHeight: %d\n", outHeight);

    for (int y = 0; y != outHeight; y++)
    {
        printf("Line %d/%d\n", y, outHeight);
        for (int x = 0; x != outWidth; x++)
        {
            pixelResult pix;
            pix = calculateZPixel(x, y, s);

            if (pix.maxVal - pix.minVal > (float)s.cutoff/1000000)
            {
                res.imgMax.at<char>(y, x) = pix.maxLoc;
                res.imgMin.at<char>(y, x) = pix.minLoc;
            }
            else
            {
                res.imgMax.at<char>(y, x) = 0;
                res.imgMin.at<char>(y, x) = 0;
            }
        }
    }
    printf("OutImg size: %d x %d\n", res.imgMax.cols, res.imgMax.rows);
    normalize( res.imgMax, res.imgMax, 0, 255, NORM_MINMAX, -1, Mat() );
    normalize( res.imgMin, res.imgMin, 0, 255, NORM_MINMAX, -1, Mat() );
    imshow( "ResultMin", res.imgMax );
    imshow( "ResultMax", res.imgMin );
    printf("Calculated, returning...\n");
    return res;
}

int main( int argc, char **argv )
{
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

    calcSettings s;
    s.cellWidth = 20;
    s.cellHeight = 20;
    s.stepX = 1;
    s.stepY = 1;
    s.sweepRange = 20;
    s.scale = 15;
    s.cutoff = 0;
    s.zOffset = 10;
    s.method = 1;
    s.reversed = 0;

    namedWindow( "Settings", CV_WINDOW_NORMAL );
    namedWindow( "Left", CV_WINDOW_NORMAL );
    namedWindow( "Right", CV_WINDOW_NORMAL );
    namedWindow( "ResultMin", CV_WINDOW_AUTOSIZE );
    namedWindow( "ResultMax", CV_WINDOW_AUTOSIZE );

    namedWindow( "DetailLeft", CV_WINDOW_AUTOSIZE );
    namedWindow( "DetailRight", CV_WINDOW_AUTOSIZE );
    namedWindow( "DetailMatch", CV_WINDOW_AUTOSIZE );

    cvCreateTrackbar( "Cell Width", "Settings", &(s.cellWidth), 255,  NULL);
    cvCreateTrackbar( "Cell Height", "Settings", &(s.cellHeight), 255,  NULL);
    cvCreateTrackbar( "X step", "Settings", &(s.stepX), 255,  NULL);
    cvCreateTrackbar( "Y step", "Settings", &(s.stepY), 255,  NULL);
    cvCreateTrackbar( "Horizontal Sweep Range", "Settings", &(s.sweepRange), 255,  NULL);
    cvCreateTrackbar( "Z offset", "Settings", &(s.zOffset), 200,  NULL);
    cvCreateTrackbar( "Scale", "Settings", &(s.scale), 255,  NULL);
    cvCreateTrackbar( "Reversed", "Settings", &(s.reversed), 1,  NULL);
    cvCreateTrackbar( "Cutoff", "Settings", &(s.cutoff), 4000000,  NULL);
    cvCreateTrackbar( "Method: \n 0: SQDIFF \n 1: SQDIFF NORMED \n 2: TM CCORR \n 3: TM CCORR NORMED \n 4: TM COEFF \n 5: TM COEFF NORMED", \
                      "Settings", &(s.method), 5,  NULL);
    //createButton("Generate", generateCallback);
    setMouseCallback("Left", onMouseOnLeftWin, &s);
    setMouseCallback("ResultMin", onMouseGetDetails, &s);
    setMouseCallback("ResultMax", onMouseGetDetails, &s);
    while (cvWaitKey(33) != 27);
    // {
    //     imshow("Left",imgL);
    //     imshow("Right",imgR);
    // }

    //imgZ = calculateZMat(imgL, imgR, cellWidth, cellHeight, stepX, stepY, sweepRange, zOffset, method);

    // imshow( "Left", imgL );
    // imshow( "Right", imgR );
    // namedWindow( "Display Image", CV_WINDOW_AUTOSIZE );
    // imshow( "Display Image", imgZ );

    //waitKey(0);

    return 0;
}

void generateCallback(int state, void *data)
{
    calcSettings *s;
    s = (calcSettings *)data;
    calcResult res;

    Mat imgL, imgR;

    int sizeX = image.cols / 2;
    int sizeY = image.rows;

    Rect r;
    r = Rect(0, 0, sizeX, sizeY);
    if(s->reversed) imgR = image(r);
    else imgL = image(r);
    r = Rect(sizeX, 0, sizeX, sizeY);
    if(s->reversed) imgL = image(r);
    else imgR = image(r);

    printf("Resizing\n");
    Mat imgLs, imgRs;
    resize(imgL, s->imgL, Size(0, 0), double(s->scale) / 100, double(s->scale) / 100);
    resize(imgR, s->imgR, Size(0, 0), double(s->scale) / 100, double(s->scale) / 100);
    printf("Calculating\n");
    res = calculateZMat(*s);
    outImgMin = res.imgMin;
    outImgMax = res.imgMax;
    printf("Displaying\n");
    imshow( "Left", s->imgL );
    imshow( "Right", s->imgR );
}

void onMouseOnLeftWin( int event, int x, int y, int, void *data)
{
    if ( event == EVENT_LBUTTONDOWN )
    {
        generateCallback(0, data);
    }
}
void onMouseGetDetails( int event, int x, int y, int, void *data)
{
    if ( event == EVENT_LBUTTONDOWN )
    {
        calcSettings *s;
        s = (calcSettings *)data;

        pixelResult pr;
        pr = calculateZPixel(x, y, *s, true);

        printf("X: %d, Y: %d\n", x, y);
        printf("Min: %4d(%5.2f), Max: %4d(%5.2f)\n", pr.minLoc, pr.minVal, pr.maxLoc, pr.maxVal);
        printf("Min: %4d, Max: %4d\n", outImgMin.at<char>(y, x), outImgMax.at<char>(y, x));
    }
}
