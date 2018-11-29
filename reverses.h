#pragma once

#include <string>

using namespace std;

string getPropertyName(propertyID) {
    std::string property = "Unknown";

    switch (PropertyID) {
    case kEdsPropID_Unknown: property = "kEdsPropID_Unknown"; break;
    case kEdsPropID_ProductName: property = "kEdsPropID_ProductName"; break;
    case kEdsPropID_OwnerName: property = "kEdsPropID_OwnerName"; break;
    case kEdsPropID_MakerName: property = "kEdsPropID_MakerName"; break;
    case kEdsPropID_DateTime: property = "kEdsPropID_DateTime"; break;
    case kEdsPropID_FirmwareVersion: property = "kEdsPropID_FirmwareVersion"; break;
    case kEdsPropID_BatteryLevel: property = "kEdsPropID_BatteryLevel"; break;
    case kEdsPropID_CFn: property = "kEdsPropID_CFn"; break;
    case kEdsPropID_SaveTo: property = "kEdsPropID_SaveTo"; break;
    case kEdsPropID_CurrentStorage: property = "kEdsPropID_CurrentStorage"; break;
    case kEdsPropID_CurrentFolder: property = "kEdsPropID_CurrentFolder"; break;
    case kEdsPropID_MyMenu: property = "kEdsPropID_MyMenu"; break;
    case kEdsPropID_BatteryQuality: property = "kEdsPropID_BatteryQuality"; break;
    case kEdsPropID_BodyIDEx: property = "kEdsPropID_BodyIDEx"; break;
    case kEdsPropID_HDDirectoryStructure: property = "kEdsPropID_HDDirectoryStructure"; break;
    case kEdsPropID_ImageQuality: property = "kEdsPropID_ImageQuality"; break;
    case kEdsPropID_JpegQuality: property = "kEdsPropID_JpegQuality"; break;
    case kEdsPropID_Orientation: property = "kEdsPropID_Orientation"; break;
    case kEdsPropID_ICCProfile: property = "kEdsPropID_ICCProfile"; break;
    case kEdsPropID_FocusInfo: property = "kEdsPropID_FocusInfo"; break;
    case kEdsPropID_DigitalExposure: property = "kEdsPropID_DigitalExposure"; break;
    case kEdsPropID_WhiteBalance: property = "kEdsPropID_WhiteBalance"; break;
    case kEdsPropID_ColorTemperature: property = "kEdsPropID_ColorTemperature"; break;
    case kEdsPropID_WhiteBalanceShift: property = "kEdsPropID_WhiteBalanceShift"; break;
    case kEdsPropID_Contrast: property = "kEdsPropID_Contrast"; break;
    case kEdsPropID_ColorSaturation: property = "kEdsPropID_ColorSaturation"; break;
    case kEdsPropID_ColorTone: property = "kEdsPropID_ColorTone"; break;
    case kEdsPropID_Sharpness: property = "kEdsPropID_Sharpness"; break;
    case kEdsPropID_ColorSpace: property = "kEdsPropID_ColorSpace"; break;
    case kEdsPropID_ToneCurve: property = "kEdsPropID_ToneCurve"; break;
    case kEdsPropID_PhotoEffect: property = "kEdsPropID_PhotoEffect"; break;
    case kEdsPropID_FilterEffect: property = "kEdsPropID_FilterEffect"; break;
    case kEdsPropID_ToningEffect: property = "kEdsPropID_ToningEffect"; break;
    case kEdsPropID_ParameterSet: property = "kEdsPropID_ParameterSet"; break;
    case kEdsPropID_ColorMatrix: property = "kEdsPropID_ColorMatrix"; break;
    case kEdsPropID_PictureStyle: property = "kEdsPropID_PictureStyle"; break;
    case kEdsPropID_PictureStyleDesc: property = "kEdsPropID_PictureStyleDesc"; break;
    case kEdsPropID_PictureStyleCaption: property = "kEdsPropID_PictureStyleCaption"; break;
    case kEdsPropID_Linear: property = "kEdsPropID_Linear"; break;
    case kEdsPropID_ClickWBPoint: property = "kEdsPropID_ClickWBPoint"; break;
    case kEdsPropID_WBCoeffs: property = "kEdsPropID_WBCoeffs"; break;
    case kEdsPropID_GPSVersionID: property = "kEdsPropID_GPSVersionID"; break;
    case kEdsPropID_GPSLatitudeRef: property = "kEdsPropID_GPSLatitudeRef"; break;
    case kEdsPropID_GPSLatitude: property = "kEdsPropID_GPSLatitude"; break;
    case kEdsPropID_GPSLongitudeRef: property = "kEdsPropID_GPSLongitudeRef"; break;
    case kEdsPropID_GPSLongitude: property = "kEdsPropID_GPSLongitude"; break;
    case kEdsPropID_GPSAltitudeRef: property = "kEdsPropID_GPSAltitudeRef"; break;
    case kEdsPropID_GPSAltitude: property = "kEdsPropID_GPSAltitude"; break;
    case kEdsPropID_GPSTimeStamp: property = "kEdsPropID_GPSTimeStamp"; break;
    case kEdsPropID_GPSSatellites: property = "kEdsPropID_GPSSatellites"; break;
    case kEdsPropID_GPSStatus: property = "kEdsPropID_GPSStatus"; break;
    case kEdsPropID_GPSMapDatum: property = "kEdsPropID_GPSMapDatum"; break;
    case kEdsPropID_GPSDateStamp: property = "kEdsPropID_GPSDateStamp"; break;
    case kEdsPropID_AtCapture_Flag: property = "kEdsPropID_AtCapture_Flag"; break;
    case kEdsPropID_AEMode: property = "kEdsPropID_AEMode"; break;
    case kEdsPropID_DriveMode: property = "kEdsPropID_DriveMode"; break;
    case kEdsPropID_ISOSpeed: property = "kEdsPropID_ISOSpeed"; break;
    case kEdsPropID_MeteringMode: property = "kEdsPropID_MeteringMode"; break;
    case kEdsPropID_AFMode: property = "kEdsPropID_AFMode"; break;
    case kEdsPropID_Av: property = "kEdsPropID_Av"; break;
    case kEdsPropID_Tv: property = "kEdsPropID_Tv"; break;
    case kEdsPropID_ExposureCompensation: property = "kEdsPropID_ExposureCompensation"; break;
    case kEdsPropID_FlashCompensation: property = "kEdsPropID_FlashCompensation"; break;
    case kEdsPropID_FocalLength: property = "kEdsPropID_FocalLength"; break;
    case kEdsPropID_AvailableShots: property = "kEdsPropID_AvailableShots"; break;
    case kEdsPropID_Bracket: property = "kEdsPropID_Bracket"; break;
    case kEdsPropID_WhiteBalanceBracket: property = "kEdsPropID_WhiteBalanceBracket"; break;
    case kEdsPropID_LensName: property = "kEdsPropID_LensName"; break;
    case kEdsPropID_AEBracket: property = "kEdsPropID_AEBracket"; break;
    case kEdsPropID_FEBracket: property = "kEdsPropID_FEBracket"; break;
    case kEdsPropID_ISOBracket: property = "kEdsPropID_ISOBracket"; break;
    case kEdsPropID_NoiseReduction: property = "kEdsPropID_NoiseReduction"; break;
    case kEdsPropID_FlashOn: property = "kEdsPropID_FlashOn"; break;
    case kEdsPropID_RedEye: property = "kEdsPropID_RedEye"; break;
    case kEdsPropID_FlashMode: property = "kEdsPropID_FlashMode"; break;
    case kEdsPropID_LensStatus: property = "kEdsPropID_LensStatus"; break;
    case kEdsPropID_Artist: property = "kEdsPropID_Artist"; break;
    case kEdsPropID_Copyright: property = "kEdsPropID_Copyright"; break;
    case kEdsPropID_DepthOfField: property = "kEdsPropID_DepthOfField"; break;
    case kEdsPropID_EFCompensation: property = "kEdsPropID_EFCompensation"; break;
    case kEdsPropID_AEModeSelect: property = "kEdsPropID_AEModeSelect"; break;
    case kEdsPropID_Evf_OutputDevice: property = "kEdsPropID_Evf_OutputDevice"; break;
    case kEdsPropID_Evf_Mode: property = "kEdsPropID_Evf_Mode"; break;
    case kEdsPropID_Evf_WhiteBalance: property = "kEdsPropID_Evf_WhiteBalance"; break;
    case kEdsPropID_Evf_ColorTemperature: property = "kEdsPropID_Evf_ColorTemperature"; break;
    case kEdsPropID_Evf_DepthOfFieldPreview: property = "kEdsPropID_Evf_DepthOfFieldPreview"; break;
    case kEdsPropID_Evf_Zoom: property = "kEdsPropID_Evf_Zoom"; break;
    case kEdsPropID_Evf_ZoomPosition: property = "kEdsPropID_Evf_ZoomPosition"; break;
    case kEdsPropID_Evf_FocusAid: property = "kEdsPropID_Evf_FocusAid"; break;
    case kEdsPropID_Evf_Histogram: property = "kEdsPropID_Evf_Histogram"; break;
    case kEdsPropID_Evf_ImagePosition: property = "kEdsPropID_Evf_ImagePosition"; break;
    case kEdsPropID_Evf_HistogramStatus: property = "kEdsPropID_Evf_HistogramStatus"; break;
    case kEdsPropID_Evf_AFMode: property = "kEdsPropID_Evf_AFMode"; break;
    case kEdsPropID_Record: property = "kEdsPropID_Record"; break;
    case kEdsPropID_Evf_HistogramY: property = "kEdsPropID_Evf_HistogramY"; break;
    case kEdsPropID_Evf_HistogramR: property = "kEdsPropID_Evf_HistogramR"; break;
    case kEdsPropID_Evf_HistogramG: property = "kEdsPropID_Evf_HistogramG"; break;
    case kEdsPropID_Evf_HistogramB: property = "kEdsPropID_Evf_HistogramB"; break;
    case kEdsPropID_Evf_CoordinateSystem: property = "kEdsPropID_Evf_CoordinateSystem"; break;
    case kEdsPropID_Evf_ZoomRect: property = "kEdsPropID_Evf_ZoomRect"; break;
    case kEdsPropID_Evf_ImageClipRect: property = "kEdsPropID_Evf_ImageClipRect"; break;
    }

    return property;
}
