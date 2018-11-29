#include "ev.h"
#include "database.h"

#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include <vector>
#include <iostream>
#include <sstream>

//#include <sys/inotify.h>

#include "opencv/highgui.h"
#include "opencv/cv.h"

#include "GraphUtils.h"


#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>

#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/program_options.hpp>

#include "sqlite3.h"

#include <list>
#include <algorithm>

namespace bf = boost::filesystem;
namespace po = boost::program_options;

const int histSize = 255;

float getFrequencyOfBin(Mat channel) {
    float frequency = 0.0;
    for( int i = 1; i < histSize; i++ ) {
        frequency += abs(channel.at<float>(i));
    }
    return frequency;
}

float computeShannonEntropy(Mat img) {
    vector<Mat> chan;
    split(img, chan);
    float entropy = 0.0;

    for (int c=0; c<chan.size(); c++) {
        float frequency = getFrequencyOfBin(chan[c]);
        for( int i = 1; i < histSize; i++ ) {
            float Hc = abs(chan[c].at<float>(i));
            entropy += -(Hc/frequency) * log10((Hc/frequency));
        }
    }

    return entropy;
}

template<class T>
T foo_moving_average(T &data, const int window) {
    T out;
    double avg=0;
    int count=0;
    //int foo = 0;
    typename T::iterator head = data.begin();;

    for (typename T::iterator it=data.begin(); it!=data.end(); it++) {
        avg += *it;
        //debug("Adding %f\n", *it);
        //foo++;
        if (++count < window) {
            out.push_back(avg/count);
        } else {
            out.push_back(static_cast<double>(avg)/window);
            //debug("Size %d\n", foo);
            avg -= *head;
            //debug("Subtracting %f\n", *head);
            //foo--;
            head++;
        }


    }
    
    return out;
}



using namespace cv;
using namespace std;
//using namespace Exiv2;

// assumes BGR
double luminance(Mat img) {
    vector<Mat> channels;
    split(img, channels);
    
    Scalar m = cv::mean(img);
    
    //return 0;
    return sqrt(0.241*pow(m(2), 2) + 0.691*pow(m(1),2) + 0.068*pow(m(0),2));
}

void match_luminance(Mat m, double target_lum) {
    return;
    double lum = luminance(m);
    if (abs(lum - target_lum) < .5) return;

    m.convertTo(m, -1, 1, target_lum - lum);
    double lum2 = luminance(m);
    debug("Trying to match %f to %f, got diff of %f\n", lum, target_lum, target_lum-lum2);
}



int main(int argc, char *argv[]) {
    int start;
    int end;
    int skip;
    int avg;
    double outfps;
    string vidmode;
    string output_file;
    vector<string> input_dirs;
    vector<int> roilist;
    int sharpen;
    int resize_rows;
    bool do_resize=false;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("video-mode,v", po::value<string>(&vidmode)->default_value("FLV1"), "video mode (DIVX, FLV1, I420, etc)")
        ("output-file,o", po::value<string>(&output_file), "")
        ("input-dirs,i", po::value<vector<string> >(&input_dirs), "")
        ("average,a", po::value<int>(&avg)->default_value(0), "number of frames in moving average (0 means off)")
        ("skip,s", po::value<int>(&skip)->default_value(1), "output only every Nth frame (1 means every frame)")
        ("start", po::value<int>(&start)->default_value(0), "input frame to start with")
        ("end", po::value<int>(&end)->default_value(1000000), "input frame to end with")
        ("fps,f", po::value<double>(&outfps)->default_value(30.0), "Output fps (Frames/s)")
        ("roi,r", po::value<vector<int> >(&roilist), "Region of interest (x y w h)")
        ("sharpen", po::value<int>(&sharpen)->default_value(0), "Radius to unsharp mask (0 is none)")
        ("resize", po::value<int>(&resize_rows), "Number of rows to resize to (default: no resize)")
        ;

    po::positional_options_description p;
    p.add("output-file", 1);
    p.add("input-dirs", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
              options(desc).positional(p).run(), vm);
    //po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    

    if (vm.count("resize")) {
        do_resize = true;
    }

    fprintf(stderr, "output file == %s, fps=%f\n", output_file.c_str(), outfps);

    /*
    if (vm.count("output-file")) {
        fprintf(stderr, "Got output file on CL\n");
        output_file = vm["input-file"].as<string>();
     }
    */

    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }

    list<double> tv_by_ev;
    int ratchet_frames = 5;
    double average_tv=0;

    char datestr[100];

    Mat imgin, imgout, img;
    Rect roi;
    int width=0, height=720;

    VideoWriter writer;
    
    int count=0, tmpcount=0;


    list<Mat> images;

    bool do_write;
    int vid_width, vid_height;

    string tmpdir(output_file + ".dir");
    string tmpfile;
    bf::create_directory(tmpdir);
    
    vector<double> alpha, beta;
    vector<double> lum, lum2;
    vector<double> contrast, contrast2;

    PhotoDB db;
    if (1) {
        EXIF ex, ex2;
        for (int i=1; i<input_dirs.size(); i++) {
            bf::path dir(input_dirs[i]);
            debug("# of elements in db %lu\n", db.size());
            db.reopen(dir.string() + "/photos.db");
            db.preload();

            fprintf(stderr, "Processing directory %s\n", dir.c_str());
            
            for (bf::directory_iterator it(dir); it!=bf::directory_iterator(); it++) {
                if (bf::path(*it).extension() != ".JPG" && bf::path(*it).extension() != ".jpg") continue;
                if (db.exists(it->path().string())) continue;
                //debug("Doesn't exist: %s\n", it->path().c_str());

                db.add_photo(it->path().string());
            
                beta.push_back(0);
                alpha.push_back(1);
            }
        }
    }

    if (0) {
        fprintf(stderr, "Processing luminance/contrast for all images\n");
        double mag=0;
        double ev1, ev2;
        const int window=30;
        PhotoRecord *current=NULL, *last=NULL;
        int i=0;
        for (list<PhotoRecord>::iterator it=db._photos.begin(); it!=db._photos.end(); it++) {
            current = &(*it);
            
            //debug("Comparing %d to %d: %f, %f\n", i, i-1, current._exposure_time, last._exposure_time);
            if (last != NULL &&
                (current->_exposure_time != last->_exposure_time || 
                 current->_aperture != last->_aperture ||
                 current->_iso != last->_iso)
                ) {
                
                //ev1 = log(pow(last._aperture, 2)/(last._exposure_time*last._iso/100.0));
                //ev2 = log(pow(current._aperture, 2)/(current._exposure_time*current._iso/100.0));
                
                Mat im1 = imread(current->_filename);
                Mat im2 = imread(last->_filename);
                ev1 = luminance(im1);
                ev2 = luminance(im2);
                
                debug("Inflection point at %s (%f, %f) %f\n", current->_filename.c_str(), ev1, ev2, ev2-ev1);
                mag = (ev2 - ev1)/2.0;
                
                //for (int j=0; j<window; j++) {
                
                //beta[std::max(i-j-1, 0)] = mag*(window-j)/(-1.0*window);
                //beta[std::min(i+j, static_cast<int>(db.size()-1))] = mag*(window-j)/(1.0*window);
                
                
                //}
            }
            
            i++;
            last = current;
        }
        fprintf(stderr, "Done processing luminance/contrast for all images\n");
    }

    for (list<PhotoRecord>::iterator it=db._photos.begin(); it!=db._photos.end(); it++) {
        if (it->_time.tm_hour < 10 || it->_time.tm_hour >= 17) continue;

        if (count >= start && count <= end && ( avg || (count % skip == 0))) {
            //ex.init(it->path().string());

            //struct tm time = ex.meta_timestamp();
            //if (time.tm_hour < 10 || time.tm_hour >= 17) continue;

            do_write = (count % skip == 0);
            string x =  boost::lexical_cast<string>(tmpcount);
            x.insert(x.begin(), 6 - x.size(), '0');
            tmpfile = tmpdir + "/" + x + ".jpg";

            if (count == start) {

                imgin = imread(it->_filename);

                if (vm.count("roi")) {
                    fprintf(stderr, "roilist size == %lu\n", roilist.size());
                    roi = Rect(roilist[0], roilist[1], roilist[2], roilist[3]);

                    vid_width = roi.width;
                    vid_height = roi.height;
                    do_resize = false;
                } else {
                    roi = Rect(0, 0, imgin.cols, imgin.rows);
                    vid_width = imgin.cols;
                    vid_height = imgin.rows;
                }

                if (do_resize) {
                    width = imgin.cols*resize_rows/imgin.rows;
                    vid_width = width;
                    vid_height = resize_rows;
                }
             
                fprintf(stderr, "ROI = %d, %d, %d, %d\n", roi.x, roi.y, roi.width, roi.height);
                fprintf(stderr, "Video size %dx%d\n", vid_width, vid_height);
                writer.open(output_file,
                            CV_FOURCC(vidmode[0], vidmode[1], vidmode[2], vidmode[3]),
                            outfps,
                            Size(vid_width, vid_height));
                    
                if (!writer.isOpened()) {
                    cout  << "Could not open the output video for write" << endl;
                    return -1;
                }
            }


            imgin = imread(it->_filename);
            imgin.convertTo(imgin, CV_32F);

            imgin = imgin(roi);

            if (avg) {
                Mat imgtmp;
                if (images.size() == skip*avg-1) {
                    imgin.convertTo(img, CV_32F, (1.0/(skip*avg)));
                    BOOST_FOREACH(Mat m, images) {
                        img += m;
                    }
                    images.pop_front();
                } else {
                    do_write = false;
                }
                imgin.convertTo(imgtmp, CV_32F, (1.0/(skip*avg)));
                images.push_back(imgtmp);
            } else {
                img = imgin;
            }

            if (do_write) {
                if (do_resize)
                    resize(img, imgout, Size(vid_width, vid_height));
                else
                    imgout = img;
                    
                Mat imgtmp, imgtmp2;
                imgout.convertTo(imgtmp, CV_8U);

                if (sharpen) {
                    Mat blur;
                    GaussianBlur(imgtmp, blur, Size(sharpen, sharpen), sharpen);
                    addWeighted(imgtmp, 1.5, blur, -0.5, 0, imgtmp);
                }

                //snprintf(datestr, 99, "%0.2d/%0.2d/%d", time.tm_mon, time.tm_mday, time.tm_year+1900);
                //putText(imgtmp, datestr, Point(20, roi.height-20), FONT_HERSHEY_DUPLEX, .7, Scalar(0, 0, 0), 1.75, 8, false);
                //putText(imgtmp, datestr, Point(20, roi.height-20), FONT_HERSHEY_DUPLEX, .7, Scalar(255, 255, 255), 1.5, 8, false);

                //fprintf(stderr, "Img size %dx%d\n", imgtmp.cols, imgtmp.rows);
                    
                Mat orig;
                lum.push_back(luminance(imgtmp));
                //contrast.push_back(computeShannonEntropy(imgtmp));
                if (0) {
                    imgtmp.copyTo(orig);
                    
                    
                    if (beta[count] != 0.0 || alpha[count] != 1.0)
                        imgtmp.convertTo(imgtmp, -1, alpha[count], beta[count]);
                    
                    lum2.push_back(luminance(imgtmp));

                    cv::Rect leftside(0, 0, imgtmp.cols/2, imgtmp.rows);
                    orig(leftside).copyTo(imgtmp(leftside));
                }

                if (0) {

                    double min, max;

                    min = std::min(*std::min_element(lum.begin(), lum.end()), *std::min_element(lum2.begin(), lum2.end()));
                    max = std::max(*std::max_element(lum.begin(), lum.end()), *std::max_element(lum2.begin(), lum2.end()));

                    int W = 350, H = 150;
                    Rect region(imgtmp.cols-1 - W-20, 10, W+20, H+20);
                    Rect region2(imgtmp.cols-1 - W-20, imgtmp.rows-1-H-20, W+20, H+20);
                    drawHistogram(imgtmp, imgtmp(region), W, H);

                    setGraphColor(0);
                    vector<double> lum_windowed = foo_moving_average(lum, 120);
                    drawFloatGraph(lum_windowed, imgtmp(region2), min, max, W, H);

                    setGraphColor(3);
                    drawFloatGraph(lum, imgtmp(region2), min, max, W, H);
                        
                    setGraphColor(4);
                    drawFloatGraph(lum2, imgtmp(region2), min, max, W, H);
                }


                if (1) {
                    vector<double> lum_windowed = foo_moving_average(lum, 120);
                    match_luminance(imgtmp, lum_windowed.back());
                    lum2.push_back(luminance(imgtmp));

                    double min, max;
                    int W = 350, H = 150;
                    Rect region(imgtmp.cols-1 - W-20, 10, W+20, H+20);
                    Rect region2(imgtmp.cols-1 - W-20, imgtmp.rows-1-H-20, W+20, H+20);

                    min = std::min(*std::min_element(lum2.begin(), lum2.end()), *std::min_element(lum_windowed.begin(), lum_windowed.end()));
                    max = std::max(*std::max_element(lum2.begin(), lum2.end()), *std::max_element(lum_windowed.begin(), lum_windowed.end()));

                    setGraphColor(0);
                    drawFloatGraph(lum2, imgtmp(region), min, max, W, H);
                    setGraphColor(1);
                    drawFloatGraph(lum_windowed, imgtmp(region), min, max, W, H);


                    //setGraphColor(1);
                    //drawFloatGraph(contrast, imgtmp(region2), 0.0, 0.0, W, H);
                }


                writer << imgtmp;
                imwrite(tmpfile, imgtmp);
                tmpcount++;
            } 

            if (count % int(outfps*skip) == 0) {
                fprintf(stderr, "processing %s : second number %0.2f\n", it->_filename.c_str(), count/(outfps*skip));
            }
		
        }
        count++;
    }

    Mat imgtmp(vid_height, vid_width, CV_8UC3);
    imgtmp.setTo(cv::Scalar(0,0,0));
    for (int i=0; i<60; i++) {
        count++;
        writer << imgtmp;
    }
    
    fprintf(stderr, "Done: %.2f seconds\n", count/(skip*outfps));
    
    return 0;
}
