//------------------------------------------------------------------------------
// Graphing functions for OpenCV.	Part of "ImageUtils.cpp", a set of handy utility functions for dealing with images in OpenCV.
// by Shervin Emami (http://www.shervinemami.co.cc/) on 20th May, 2010.
//------------------------------------------------------------------------------

#ifndef GRAPH_UTILS_H
#define GRAPH_UTILS_H

#include <opencv/cv.h>
#include <opencv/cxcore.h>

#define sprintf_s snprintf

using namespace cv;

#define DEFAULT(val) = val

const CvScalar BLACK = CV_RGB(0,0,0);
const CvScalar WHITE = CV_RGB(255,255,255);
const CvScalar GREY = CV_RGB(150,150,150);


//------------------------------------------------------------------------------
// Graphing functions
//------------------------------------------------------------------------------
CvScalar getGraphColor(void);

void drawHistogram(cv::Mat image, cv::Mat imageDst, int width, int height, bool showScale=false);

// Draw the graph of an array of floats into imageDst or a new image, between minV & maxV if given.
// Remember to free the newly created image if imageDst is not given.
template<class T>
void drawFloatGraph(const T &data, cv::Mat imageDst, float minV, float maxV, int width, int height, char *graphLabel=NULL, bool showScale=false) {
    int w = width;
    int h = height;
    int b = 10;		// border around graph within the image
    if (w <= 20)
        w = data.size() + b*2;	// width of the image
    if (h <= 20)
        h = 220;

    int s = h - b*2;// size of graph height
    float xscale = 1.0;
    if (data.size() > 1)
        xscale = (w - b*2) / (float)(data.size()-1);	// horizontal scale

    //imageDst.setTo(Scalar(255, 255, 255));

    CvScalar colorGraph = getGraphColor();	// use a different color each time.

    // If the user didnt supply min & mav values, find them from the data, so we can draw it at full scale.
    if (fabs(minV) < 0.0000001f && fabs(maxV) < 0.0000001f) {
        for (typename T::const_iterator it=data.begin(); it!=data.end(); it++) {
            if (*it < minV)
                minV = *it;
            if (*it > maxV)
                maxV = *it;
        }
    }
    float diffV = maxV - minV;
    if (diffV == 0)
        diffV = 0.00000001f;	// Stop a divide-by-zero error
    float fscale = (float)s / diffV;

    // Draw the horizontal & vertical axis
    int y0 = cvRound(minV*fscale);
    line(imageDst, cv::Point(b,h-(b-y0)), cv::Point(w-b, h-(b-y0)), BLACK);
    line(imageDst, cv::Point(b,h-(b)), cv::Point(b, h-(b+s)), BLACK);

    // Write the scale of the y axis
    //CvFont font;
    //cvInitFont(&font,CV_FONT_HERSHEY_PLAIN,0.55,0.7, 0,1,CV_AA);	// For OpenCV 1.1
    int font = CV_FONT_HERSHEY_PLAIN;
    if (showScale) {
        //cvInitFont(&font,CV_FONT_HERSHEY_PLAIN,0.5,0.6, 0,1, CV_AA);	// For OpenCV 2.0
        CvScalar clr = GREY;
        char text[16];
        sprintf_s(text, sizeof(text)-1, "%.1f", maxV);
        putText(imageDst, text, cv::Point(1, b+4), font, .5, clr, 1, CV_AA);
        // Write the scale of the x axis
        sprintf_s(text, sizeof(text)-1, "%lu", (data.size()-1) );
        putText(imageDst, text, cv::Point(w-b+4-5*strlen(text), (h/2)+10), font, 1, clr, CV_AA);
    }

    // Draw the values
 cv::Point ptPrev = cv::Point(b,h-(b-y0));	// Start the lines at the 1st point.
    size_t i=0;
    for (typename T::const_iterator it=data.begin(); it!=data.end(); it++) {
        int y = cvRound((*it - minV) * fscale);	// Get the values at a bigger scale
        int x = cvRound(i++ * xscale);
    cv::Point ptNew = cv::Point(b+x, h-(b+y));
        line(imageDst, ptPrev, ptNew, colorGraph, 1, CV_AA);	// Draw a line from the previous point to the new point
        ptPrev = ptNew;
    }


    // Write the graph label, if desired
    if (graphLabel != NULL && strlen(graphLabel) > 0) {
        //cvInitFont(&font,CV_FONT_HERSHEY_PLAIN, 0.5,0.7, 0,1,CV_AA);
        putText(imageDst, graphLabel, cv::Point(30, 10), font, .5, CV_RGB(0,0,0), CV_AA);	// black text
    }
}


// Draw the graph of an array of ints into imageDst or a new image, between minV & maxV if given.
// Remember to free the newly created image if imageDst is not given.
IplImage* drawIntGraph(const int *arraySrc, int nArrayLength, IplImage *imageDst DEFAULT(0), int minV DEFAULT(0), int maxV DEFAULT(0), int width DEFAULT(0), int height DEFAULT(0), char *graphLabel DEFAULT(0), bool showScale DEFAULT(true));

// Draw the graph of an array of uchars into imageDst or a new image, between minV & maxV if given.
// Remember to free the newly created image if imageDst is not given.
IplImage* drawUCharGraph(const uchar *arraySrc, int nArrayLength, IplImage *imageDst DEFAULT(0), int minV DEFAULT(0), int maxV DEFAULT(0), int width DEFAULT(0), int height DEFAULT(0), char *graphLabel DEFAULT(0), bool showScale DEFAULT(true));

// Call 'setGraphColor(0)' to reset the colors that will be used for graphs.
void setGraphColor(int index DEFAULT(0));
// Specify the exact color that the next graph should be drawn as.
void setCustomGraphColor(int R, int B, int G);




#endif //end GRAPH_UTILS
