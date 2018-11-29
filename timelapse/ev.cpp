
#include "ev.h"

#include <cmath>

using namespace std;





Exiv2::ExifData &exif_data(string file, Exiv2::Image::AutoPtr image) {
    image = Exiv2::ImageFactory::open(file);
    //assert(image.get() != 0);
    image->readMetadata();
    return image->exifData();
}

double exif_measured_ev(Exiv2::ExifData &exifData) {
    return static_cast<int>(100.0 * (exifData["Exif.CanonSi.MeasuredEV"].value().toLong() / 32.0 + 5.0) + 0.5) / 100.0;
}

double exif_exposure_time(Exiv2::ExifData &exifData) {
    Exiv2::URational ur = exposureTime(canonEv(exifData["Exif.Photo.ExposureTime"].value().toLong()));
    return static_cast<double>(ur.first) / ur.second;
}

double exif_aperture(Exiv2::ExifData &exifData) {
    return fnumber(exifData["Exif.Photo.ApertureValue"].value().toFloat());
}
