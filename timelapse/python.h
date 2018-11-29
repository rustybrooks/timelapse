#include "CameraModel.h"

#include <boost/python.hpp>

BOOST_PYTHON_MODULE(CameraModel) {
    class CameraModel("CameraModel", init())
        .def("command_takePicture", &CameraModel::command_takePicture)
        ;
}
