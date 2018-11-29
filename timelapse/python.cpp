#include "CameraModel.h"

#include <boost/python.hpp>

using namespace boost::python;

BOOST_PYTHON_MODULE(CameraModel) {
    class_<CameraModel>("CameraModel")
        .def("command_takePicture", &CameraModel::command_takePicture)
        ;
}
