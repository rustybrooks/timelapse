#pragma once

#include "log.h"

#include <string>
#include <cmath>

#include "exiv2/exiv2.hpp"

using namespace std;


class EXIF {
public:
    EXIF() {
    }

    void init(std::string file) {
        image = Exiv2::ImageFactory::open(file);
        image->readMetadata();
        exifData = image->exifData();
    }

    int iso() {
        //debug ("iso == %ld\n", exifData["Exif.Photo.ISOSpeedRatings"].value().toLong());
        if (exifData.findKey(Exiv2::ExifKey("Exif.Photo.ISOSpeedRatings")) != exifData.end()) {
            return exifData["Exif.Photo.ISOSpeedRatings"].value().toLong();
        } else {
            return int(exp(canonEv(exifData["Exif.CanonSi.ISOSpeed"].value().toLong()) * log(2.0)) * 100.0 / 32.0);
        }
    }

    double measured_ev() {
        short val = exifData["Exif.CanonSi.MeasuredEV"].value().toLong();
        //debug("measured ev bare=%ld %d\n", exifData["Exif.CanonSi.MeasuredEV"].value().toLong(), val);
        return static_cast<int>(100.0 * (val / 32.0 + 5.0) + 0.5) / 100.0;
    }

    double exposure_time() {
        Exiv2::URational ur = exposure_time_rational();
        return static_cast<double>(ur.first) / ur.second;
    }

    Exiv2::URational exposure_time_rational() {
        Exiv2::URational ur = exifData["Exif.Photo.ExposureTime"].value().toRational();
        return ur;
    }

    double aperture() {
        return fnumber(exifData["Exif.Photo.ApertureValue"].value().toFloat());
    }

    // EV = log_2(N^2 / t)
    // 2^EV = N^2 / t
    // 2^EV * t = N^2
    // N = sqrt(2^EV * t)
    double ev_time_to_av(double ev, double time, int _iso=0) {
        if (_iso == 0) _iso = iso();
        return sqrt(pow(2,ev)*time*(_iso/100.0));
    }

    // EV = log_2(N^2 / t)
    // 2^EV = N^2 / t
    // t = N^2/2^EV
    double ev_av_to_time(double ev, double av, int _iso=0) {
        if (_iso == 0) _iso = iso();
        return av*av/pow(2,ev)/(_iso/100.0);
    }

    // EV = log_2(N^2 / t)
    double tv_av_to_ev(double tv, double av, int _iso=0) {
        if (_iso == 0) _iso = iso();
        return log(av*av/(tv*_iso/100.0));
    }

    Exiv2::URational exposureTime(float shutterSpeedValue) {
        Exiv2::URational ur(1, 1);
        double tmp = std::exp(std::log(2.0) * shutterSpeedValue);
        if (tmp > 1) {
            ur.second = static_cast<long>(tmp + 0.5);
        }
        else {
            ur.first = static_cast<long>(1/tmp + 0.5);
        }
        return ur;
    }

    float canonEv(long val) {
        // temporarily remove sign
        int sign = 1;
        if (val < 0) {
            sign = -1;
            val = -val;
        }
        // remove fraction
        float frac = static_cast<float>(val & 0x1f);
        val -= long(frac);
        // convert 1/3 (0x0c) and 2/3 (0x14) codes
        if (frac == 0x0c) {
            frac = 32.0f / 3;
        }
        else if (frac == 0x14) {
            frac = 64.0f / 3;
        }
        return sign * (val + frac) / 32.0f;
    }

    float fnumber(float apertureValue) {
        return static_cast<float>(std::exp(std::log(2.0) * apertureValue / 2));
    }

    string meta(string key) {
        //Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(file);
        //assert(image.get() != 0);
        //image->readMetadata();

        Exiv2::ExifData &exifData = image->exifData();
        if (exifData.empty()) {
            std::string error;
            error += "No Exif data found in the file";
            fprintf(stderr, "%s\n", error.c_str());
        }
    
        //cout << key << ": " << exifData[key] << "  " << exifData[key].value().typeId() << endl;
        return exifData[key].value().toString();
        /*
          std::cout << "----------------------------------------" << endl;
          Exiv2::ExifData::const_iterator end = exifData.end();
          for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {
          const char* tn = i->typeName();
          std::cout << std::setw(44) << std::setfill(' ') << std::left
          << i->key() << " "
          << "0x" << std::setw(4) << std::setfill('0') << std::right
          << std::hex << i->tag() << " "
          << std::setw(9) << std::setfill(' ') << std::left
          << (tn ? tn : "Unknown") << " "
          << std::dec << std::setw(3)
          << std::setfill(' ') << std::right
          << i->count() << "  "
          << std::dec << i->value()
          << "\n";
          }
        */
    }

    tm meta_timestamp() {
        struct tm time;
        string timestr = meta("Exif.Image.DateTime");

        char *x;
        if ((x = strptime(timestr.c_str(), "%Y:%m:%d %H:%M:%S", &time)) != NULL) {
            //fprintf(stderr, "Error converting time string: %s %ld\n", x, x-timestr.c_str());
        }
        return time;
    }

    double moving_average(list<double> &data, const int frames) {
        double avg=0;
        for (list<double>::iterator it=data.begin(); it!=data.end(); it++) {
            avg += *it;
        }

        return avg/frames;
    }

    double weighted_moving_average(list<double> &data, const int frames) {
        unsigned int count=frames;
        double avg=0;
        for (list<double>::iterator it=data.begin(); it!=data.end(); it++) {
            avg += (count--)*(*it);
        }

        return avg/((frames*(frames+1))/2);
    }

    double moving_median(list<double> &data, const int frames) {
        vector<double> newdata(data.begin(), data.end());
        sort(newdata.begin(), newdata.end());

        double median;
        size_t size = newdata.size();

        if (size  % 2 == 0) {
            median = (newdata[size / 2 - 1] + newdata[size / 2]) / 2;
        } else {
            median = newdata[size / 2];
        }

        return median;
    }

    double modified_moving_average(list<double> &data, int frames) {
        unsigned int count=0;
        double avg=0;
        for (list<double>::iterator it=data.begin(); it!=data.end(); it++) {
            count++;
            avg += (frames-count)*(*it);
        }

        return avg/(frames*(frames+1))/2;
    }

private:
    Exiv2::Image::AutoPtr image;
    Exiv2::ExifData exifData;
};
