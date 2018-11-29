// need to add:
// start time/end time? (i.e. start delay)
// min/max for ratcheting, and/or ev compensation by time or something? (ev ramping of course)
// histogram stuff possibly better than ev...
// combo time/iso stepping

#include "dslr_capture.h"
#include "log.h"
#include "timer.h"
#include "CameraModel.h"
#include "callbacks.h"
#include "ev.h"

#include "EDSDK.h"
#include "EDSDKTypes.h"

#include "opencv/highgui.h"
#include "opencv/cv.h"

#include "GraphUtils.h"

#include "exiv2/exiv2.hpp"

#include <boost/program_options.hpp>
#include <boost/foreach.hpp>

#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <list>
#include <algorithm>

#ifdef WIN32
#else
void Sleep(double msec) {
    timespec tm;
    int seconds = int(msec/1000.);
    int nano = int((msec/1000 - seconds) * 1e9);
    tm.tv_sec = seconds;
    tm.tv_nsec = nano;
    debug("Sleep %f = %d, %d\n", msec, seconds, nano);
    nanosleep(&tm, NULL);
}
#endif


namespace po = boost::program_options;
using namespace cv;

CameraModel * _model;
size_t last_change=1000;
double current_iso;
double current_tv;
double current_av;
double current_ev;
double average_tv=0;
list<double> tv_by_ev, tv_by_ev_all, exposure_ev;
string ratchet = "";
unsigned int ratchet_frames;
bool command_mode, playback_mode;

double time_value_max;
double iso_max;

void process_downloaded_file(string file) {
    EXIF ex;
    Exiv2::URational ur;


    ex.init(file);
    current_ev = ex.measured_ev();
    current_av = ex.aperture();
    current_tv = ex.exposure_time();
    current_iso = ex.iso();
    ur = ex.exposure_time_rational();
    debug("Read from %s: ev=%f, av=%f, tv=%f (%d/%d) iso=%d\n", 
          file.c_str(), current_ev, current_av, current_tv, ur.first, ur.second, ex.iso());

    Mat img = imread(file);
    resize(img, img, cv::Size(600*img.cols/img.rows, 600));

    tv_by_ev.push_front(current_ev);
            
    if (tv_by_ev.size() >= ratchet_frames) {
        double avg1 = ex.moving_average(tv_by_ev, ratchet_frames);
        double avg2 = ex.weighted_moving_average(tv_by_ev, ratchet_frames);
        double med = ex.moving_median(tv_by_ev, ratchet_frames);


        EdsInt32 code, this_iso_code;
        double code_time;
        double this_iso = ex.iso();
        if (playback_mode) {
            code = 0;
            code_time = 0;
        } else {
            while (true) {
                double desired_time = ex.ev_av_to_time(avg1, current_av, this_iso);
                code = _model->_props._tvMap.code_nearest(desired_time, _model->getTvDesc());
                code_time = _model->_props._tvMap.value(code);
                if (time_value_max <= 0 || code_time < time_value_max) break;

                while (iso_max > 0) {
                    this_iso_code = _model->_props._tvMap.next_highest(desired_time, _model->getIsoDesc());
                    this_iso = _model->_props._isoMap.value(this_iso_code);
                    if (this_iso > iso_max) {
                        this_iso = _model->_props._isoMap.value(this_iso_code);
                        this_iso_code = _model->_props._tvMap.code_nearest(iso_max, _model->getIsoDesc());
                        break;
                    }
                }
            }
        }


        tv_by_ev_all.push_back(avg1);
        debug("Avg=%f, WAVG=%f, exp avg=%f, med=%f, code time=%f\n", avg1, avg2, average_tv, med, code_time);

        tv_by_ev.pop_back();

        if (
            (ratchet == "-" && code_time/this_iso < current_tv/current_iso) ||
            (ratchet == "+" && code_time/this_iso > current_tv/current_iso) ||
            (ratchet == "+-" && last_change > ratchet_frames) 
            ) {
            debug("Using new time value of %f\n", code_time);
            current_tv = code_time;
            _model->command_setTv(code);
            last_change=0;
        }

        last_change++;
    }
    
    exposure_ev.push_back(ex.tv_av_to_ev(current_tv, current_av));

    int W = 350, H = 150;
    cv::Rect region(img.cols-1 - W-20, 10, W+20, H+20);
    cv::Rect region2(img.cols-1 - W-20, img.rows-1-H-20, W+20, H+20);
    drawHistogram(img, img(region), W, H);

    setGraphColor(3);
    drawFloatGraph(tv_by_ev_all, img(region2), 0.0, 0.0, W, H);

    setGraphColor(4);
    drawFloatGraph(exposure_ev, img(region2), 0.0, 0.0, W, H);

    imshow(file, img);
}


void read_command() {
    static string buffer;
    char this_buf[1001];
    if (!command_mode) return;

    int sz = fread(this_buf, 1000, 1, stdin);
    if (sz) {
        buffer += string(this_buf, sz);
        size_t found = buffer.find_first_of('\n');
        if (found!=std::string::npos) {
            string command = buffer.substr(0, found-1);
            buffer.erase(0, found);
            debug("Found command: %s\n", command.c_str());
        }
    }
}

int main(int argc, char **argv) {

    int number;
    double interval;
    string aperture_value, iso_value, comp_value, time_value, ae_value, quality_value;
    string savedir, saveto;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("interval,i", po::value<double>(&interval)->default_value(1), "Interval between frames")
        ("number,n", po::value<int>(&number)->default_value(1), "Number of frames frames")
        ("tv", po::value<string>(&time_value), "use Tv of <value>")
        ("av", po::value<string>(&aperture_value), "use Av of <value>")
        ("iso", po::value<string>(&iso_value), "use ISO of <value>")
        ("comp", po::value<string>(&comp_value), "use exposure compensation of <value> stops")
        //("ae", po::value<string>(&ae_value), "use auto exposure mode of <value>")
        ("quality", po::value<string>(&quality_value), "use image quality mode of <value>")
        ("dir,d", po::value<string>(&savedir)->default_value("/tmp/dslr_capture"), "Directory to save images in")
        ("saveto", po::value<string>(&saveto), "Where to save picture to (host, camera, or both)")
        ("ratchet", po::value<string>(&ratchet), "Auto expose in M mode, allowing exposure to go either down only ('-') or up only ('+').  Give --tv and --av for your first exposure (a best guess)")
        ("ratchet-frames", po::value<unsigned int>(&ratchet_frames)->default_value(10), "Number of frames to take moving avg of for ratcheting")
        ("tv-max", po::value<double>(&time_value_max)->default_value(0), "Maximum allowable tv (used when ratcheting)")
        ("iso-max", po::value<double>(&iso_max)->default_value(0), "Maximum allowable tv (used when ratcheting)")
        ("command-mode", po::value<bool>(&command_mode)->default_value(false), "Enter command mode")
        ("playback-mode", po::value<bool>(&playback_mode)->default_value(false), "Use playback mode");
        ;

    po::positional_options_description p;
    //p.add("output-file", 1);
    //p.add("input-dirs", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
              options(desc).positional(p).run(), vm);
    //po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    

    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }

    EdsSaveTo edssaveto = kEdsSaveTo_Host;
    if (vm.count("saveto")) {
        if (saveto == "host") {
            edssaveto = kEdsSaveTo_Host;
        } else if (saveto == "both") {
            saveto = kEdsSaveTo_Both;
        } else if (saveto == "camera") {
            saveto = kEdsSaveTo_Camera;
        } else {
            fprintf(stderr, "Incorrect value for --saveto, use 'camera', 'host' or 'both'\n");
            cout << desc << endl;
            return 1;
        }
    }

    //EdsError error = EDS_ERR_OK;

    int maxnum=0;
    typedef vector<bf::path> vec;             // store paths,
    vec v;                                // so we can sort them later
    
    if (!bf::exists(savedir)) bf::create_directory(savedir);
    for (bf::directory_iterator it(savedir); it!=bf::directory_iterator(); it++) {
        if (
            bf::path(*it).extension() != ".JPG" && bf::path(*it).extension() != ".jpg" &&
            bf::path(*it).extension() != ".RAW" && bf::path(*it).extension() != ".raw" &&
            bf::path(*it).extension() != ".CR2" && bf::path(*it).extension() != ".cr2" 
            ) continue;

        v.push_back(*it);
    }

    if (v.size()) {
        sort(v.begin(), v.end());             
        string last = v.back().stem().string();
        maxnum = strtol(last.c_str()+4, NULL, 10) + 1;
    }

    if (!playback_mode) {
        debug("Starting with frame #%d\n", maxnum);

        _model = new CameraModel(savedir, maxnum);
        _model->initialize(edssaveto, ObjectCallbackFunc, PropertyCallbackFunc, CameraStateCallbackFunc);

        if (vm.count("tv") && vm.count("av")) {
            _model->command_setTv(time_value);
            _model->command_setAv(aperture_value);
        } else if (vm.count("tv")) {
            _model->command_setTv(time_value);
        } else if (vm.count("av")) {
            _model->command_setAv(aperture_value);
        } else {
            cerr << "Must choose --tv, --av or both" << endl;
            return 1;
        }
    }

    if (vm.count("iso")) { _model->command_setIso(iso_value); }
    if (vm.count("comp")) { _model->command_setExposureCompensation(comp_value); }
    if (vm.count("ae")) { _model->command_setAEMode(ae_value); }
    if (vm.count("quality")) { _model->command_setImageQuality(quality_value); }


    vector<EdsUInt32> proplist;
    proplist.push_back(kEdsPropID_Av);
    //proplist.push_back(kEdsPropID_Tv);

    double time_start, loop_start;
    time_start = time_now();
    while (time_now() - time_start < 2) {
        read_command();
        CameraModel::loop_once();
    }

    time_start = time_now();
    for (int i=0; i<number; i++) {
        loop_start = time_now();
        read_command();
        //_model->current_file = "IMG_" + boost::lexical_cast<string>(i);


        int ti=1;
        if (playback_mode) {
            process_downloaded_file(v[i*20].string());
        } else {
            if (1) {
                while (!_model->command_takePicture()) {
                    CameraModel::loop_once();
                    ti++;
                }
            } else {
                while (!_model->command_takePicture_FixedBulb(0.1)) {
                    CameraModel::loop_once();
                    ti++;
                }
            }
            if (ti > 2) 
                debug("TakePic took %d tries\n", ti);
        }

        while (time_now() - time_start < interval*(i+1)) {
            if (interval > 10) {
                Sleep(1000*min(interval/100.0, 0.5));
            }
            CameraModel::loop_once();
            read_command();
        }

        debug("--------------------%d/%d = %0.2f%% -- Loop took %f, offset=%f\n", i+1, number, 100.0*(i+1)/number, time_now() - loop_start, (time_now() - time_start)/interval);
    }

    if (!playback_mode) {
        time_start = time_now();
        while (time_now() - time_start < 2) {
            CameraModel::loop_once();
        }

        BOOST_FOREACH(CameraModel::SaveItem &r, _model->images_to_download) {
            _model->command_download(r._directoryItem);
        }
    }

    delete _model;
}


EdsError EDSCALLBACK ObjectCallbackFunc(EdsObjectEvent inEvent, EdsBaseRef inRef, EdsVoid *inContext) {
    std::string file;

    //EdsError err;
    CameraModel *_model = (CameraModel*) inContext;

    switch (inEvent)
    {
    case kEdsObjectEvent_DirItemCreated:
        debug("Directory Item Created\n");
        _model->images_to_download.push_back(CameraModel::SaveItem(_model->current_file, inRef));
        //_model->stoploop(); // not sure
        break;
    case kEdsObjectEvent_DirItemRequestTransfer:
        //debug("Directory Item Requested Transfer\n");
        file = _model->command_download(inRef);

        process_downloaded_file(file);

        break;
    default:
        //debug("ObjectEventHandler: %x %p\n", inEvent, inRef);
        //_model->stoploop(); // not sure
        break;
    }
 
    return EDS_ERR_OK;
}

EdsError EDSCALLBACK PropertyCallbackFunc(EdsPropertyEvent inEvent, EdsPropertyID inPropertyID, EdsUInt32 inParam, EdsVoid *inContext) {

    CameraModel *_model = (CameraModel*) inContext;

    //debug("Property Callback Function %d, %d, %lu\n", inEvent, inPropertyID, inParam);
    switch(inEvent) {
    case kEdsPropertyEvent_PropertyChanged:
        _model->command_getProperty(inPropertyID, inParam);
        break;

    case kEdsPropertyEvent_PropertyDescChanged:
        _model->command_getPropertyDesc(inPropertyID);
        break;
    }

    return EDS_ERR_OK;
}

EdsError EDSCALLBACK CameraStateCallbackFunc(EdsStateEvent inEvent, EdsUInt32 inEventData, EdsVoid *inContext) {

    CameraModel *_model = (CameraModel*) inContext;

    switch (inEvent) {
    case kEdsStateEvent_WillSoonShutDown:
        _model->command_extendShutdownTimer();
        break;

    case kEdsStateEvent_All: debug("Camera State: All\n"); break;
    case kEdsStateEvent_Shutdown: debug("Camera State: Shutdown\n"); break;
    case kEdsStateEvent_JobStatusChanged: 
        //debug("Camera State: JobStatusChanged\n"); 
        break;
    case kEdsStateEvent_ShutDownTimerUpdate: 
        //debug("Camera State: ShutDownTimerUpdate\n"); 
        break;
    case kEdsStateEvent_CaptureError: debug("Camera State: CaptureError\n"); break;
    case kEdsStateEvent_InternalError: debug("Camera State: InternalError\n"); break;
    case kEdsStateEvent_AfResult: debug("Camera State: AfResult\n"); break;
    case kEdsStateEvent_BulbExposureTime: debug("Camera State: BulbExposureTime\n"); break;
    default: 
        debug("unknown camera state callback happened %x\n", inEvent); fflush(stderr);
    }

    return EDS_ERR_OK;
};
