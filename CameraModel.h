#pragma once

#include "dslr_capture.h"
#include "properties.h"

#include "log.h"
#include "EDSDK.h"

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
//#include <boost/filesystem/fstream.hpp>

#include <string>
#include <vector>
#include <map>

#include "exiv2/exiv2.hpp"

#ifdef WIN32
#include "windows.h"
#include "winuser.h"
#else
#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc.h>
#include <objc/objc-runtime.h>
#endif

using namespace std;

namespace bf = boost::filesystem;



class CameraModel {
public:
    struct SaveItem {
        SaveItem(string filename, EdsDirectoryItemRef directoryItem) 
            : _filename(filename)
            , _directoryItem(directoryItem)
        {}
        
        string _filename;
        EdsDirectoryItemRef _directoryItem;
    };



protected:
    EdsDeviceInfo _deviceInfo;
    EdsCameraRef _camera;
    bool _is_legacy;
    bool do_loop;

    //Count of UIlock
    int		_lockCount;
    
    // Model name
    EdsChar  _modelName[EDS_MAX_NAME];
    
    // Taking a picture parameter
    EdsUInt32 _AEMode;
    EdsUInt32 _Av;
    EdsUInt32 _Tv;
    EdsUInt32 _Iso;
    EdsUInt32 _MeteringMode;
    EdsUInt32 _ExposureCompensation;
    EdsUInt32 _ImageQuality;
    EdsUInt32 _AvailableShot;
    EdsUInt32 _evfMode;
    EdsUInt32 _evfOutputDevice;
    EdsUInt32 _evfDepthOfFieldPreview;
    EdsUInt32 _evfZoom;
    EdsPoint  _evfZoomPosition;
    EdsRect	  _evfZoomRect;
    EdsUInt32 _evfAFMode;

    EdsFocusInfo _focusInfo;

    // List of value in which taking a picture parameter can be set
    EdsPropertyDesc _AEModeDesc;
    EdsPropertyDesc _AvDesc;
    EdsPropertyDesc _TvDesc;
    EdsPropertyDesc _IsoDesc;
    EdsPropertyDesc _MeteringModeDesc;
    EdsPropertyDesc _ExposureCompensationDesc;
    EdsPropertyDesc _ImageQualityDesc;
    EdsPropertyDesc _evfAFModeDesc;

    string _unknown;
    string _savedir;
    int _img_num;

public:
    Properties _props; // ew
    list<SaveItem> images_to_download;
    string current_file;

public:
    // Constructor
    CameraModel(string savedir="/tmp", int start_num=0)
        : _is_legacy(false)
        , do_loop(true)
        , _lockCount(0)
        , _unknown("UNKNOWN")
        , _savedir(savedir)
        , _img_num(start_num)
        , current_file("foo")
    {
        memset(&_focusInfo, 0, sizeof(_focusInfo));

        //initialize();
    } 

    virtual ~CameraModel() {
        command_closeSession();

        debug("About to terminate\n"); fflush(stderr);
        EdsTerminateSDK();
    }
    
    //Acquisition of Camera Object
    EdsCameraRef getCameraObject() const {return _camera;}


    //Property
public:
    void stoploop() {
        do_loop = false;
    }
    
#ifdef WIN32
    void eventloop() {
        do_loop = true;
        
        MSG msg;	
        while(do_loop) {        
            loop_once();
        }
    }

    static void loop_once() {
        GetMessage(&msg, NULL, NULL, NULL);
        TranslateMessage(&msg); 
        DispatchMessage(&msg); 
    }
#else
    void eventloop() {
        do_loop = true;
        while (do_loop) {
            loop_once();
        }
    }

    static void loop_once() {
      id now = (id) objc_getClass("NSDate");
        now = objc_msgSend(now, sel_registerName("alloc"));
        now = objc_msgSend(now, sel_registerName("initWithTimeIntervalSinceNow:"), 0.05);
        
        id runLoop = objc_msgSend((id) objc_getClass("NSRunLoop"), sel_registerName("currentRunLoop"));
        objc_msgSend(runLoop, sel_registerName("runUntilDate:"), now);
    }
#endif



    void initialize(EdsSaveTo saveTo, EdsObjectEventHandler objHandler, EdsPropertyEventHandler propHandler, EdsStateEventHandler stateHandler) {
        EdsError error = EDS_ERR_OK;
        EdsCameraListRef  cameraList = NULL;
        EdsUInt32 count = 0;

        error = EdsInitializeSDK();
    
        //Acquisition of camera list
        if(error == EDS_ERR_OK) {
            error = EdsGetCameraList(&cameraList);
        }
        
        //Acquisition of number of Cameras
        if(error == EDS_ERR_OK) {
            error = EdsGetChildCount(cameraList, &count);
            if(count == 0) {
                error = EDS_ERR_DEVICE_NOT_FOUND;
            }
        }
        
        //Acquisition of camera at the head of the list
        if(error == EDS_ERR_OK) {
            error = EdsGetChildAtIndex(cameraList, 0, &_camera);
        } else {
            debug("Failed to get camera list\n"); fflush(stderr);
            exit(0);
        }
        
        EdsSetObjectEventHandler (_camera, kEdsObjectEvent_All, objHandler, this);
        EdsSetPropertyEventHandler (_camera, kEdsPropertyEvent_All, propHandler, this);
        EdsSetCameraStateEventHandler (_camera, kEdsStateEvent_All, stateHandler, this);
        
        //Acquisition of camera information
        if(error == EDS_ERR_OK) {
            error = EdsGetDeviceInfo(_camera, &_deviceInfo);
            if(error == EDS_ERR_OK && _camera == NULL) {
                error = EDS_ERR_DEVICE_NOT_FOUND;
            }
        }
        
        //Release camera list
        if(cameraList != NULL) EdsRelease(cameraList);
        
        //Release camera list
        if(cameraList != NULL) {
            EdsRelease(cameraList);
        }

        // if Legacy protocol.
        if (_deviceInfo.deviceSubType == 0) {
            _is_legacy = true;
        }

        if(error != EDS_ERR_OK) {
            debug("Exiting for some reason\n"); fflush(stderr);
            EdsRelease(_camera);
            exit(0);
        }

        //          Both     Host Camera
        if (!command_openSession(saveTo)) {
            debug("Error opening session\n"); fflush(stderr);
            exit(1);
        }        

        EdsCapacity newCapacity = {0x7FFFFFFF, 0x1000, 1};
        command_setCapacity(newCapacity);
    }

    int img_num(string extension) {
        return _img_num++;
    }

    // Taking a picture parameter
    void setAEMode(EdsUInt32 value )                 { _AEMode = value;}
    void setTv( EdsUInt32 value )                    { _Tv = value;}
    void setAv( EdsUInt32 value )                    { _Av = value;}
    void setIso( EdsUInt32 value )                   { _Iso = value; }
    void setMeteringMode( EdsUInt32 value )          { _MeteringMode = value; }
    void setExposureCompensation( EdsUInt32 value)   { _ExposureCompensation = value; }
    void setImageQuality( EdsUInt32 value)           { _ImageQuality = value; }
    void setEvfMode( EdsUInt32 value)                { _evfMode = value; }
    void setEvfOutputDevice( EdsUInt32 value)        { _evfOutputDevice = value; }
    void setEvfDepthOfFieldPreview( EdsUInt32 value) { _evfDepthOfFieldPreview = value; }
    void setEvfZoom( EdsUInt32 value)                { _evfZoom = value; }
    void setEvfZoomPosition( EdsPoint value)         { _evfZoomPosition = value; }
    void setEvfZoomRect( EdsRect value)              { _evfZoomRect = value; }
    void setModelName(EdsChar *modelName)            { strcpy(_modelName, modelName); }
    void setEvfAFMode( EdsUInt32 value)              { _evfAFMode = value; }
    void setFocusInfo( EdsFocusInfo value)           { _focusInfo = value; }

    string const& propIdToString(const PropertyBag &propmap, const EdsUInt32 propid) const {
        PropertyMap::const_iterator it = propmap._map.find(propid);
        if (it == propmap._map.end()) {
            debug("Unknown %x\n", propid);
            return _unknown;
        } else {
            return it->second._desc;
        }
    }

    // Taking a picture parameter
    EdsUInt32 getAEMode() const                 { return _AEMode; }
    EdsUInt32 getTv() const                     { return _Tv; }
    EdsUInt32 getAv() const                     { return _Av; }
    EdsUInt32 getIso() const                    { return _Iso; }
    EdsUInt32 getMeteringMode() const           { return _MeteringMode; }
    EdsUInt32 getExposureCompensation() const   { return _ExposureCompensation; }
    EdsUInt32 getImageQuality() const           { return _ImageQuality; }
    EdsUInt32 getEvfMode() const                { return _evfMode; }
    EdsUInt32 getEvfOutputDevice() const        { return _evfOutputDevice; }
    EdsUInt32 getEvfDepthOfFieldPreview() const { return _evfDepthOfFieldPreview; }
    EdsUInt32 getEvfZoom() const                { return _evfZoom; }	
    EdsPoint  getEvfZoomPosition() const        { return _evfZoomPosition; }	
    EdsRect   getEvfZoomRect() const            { return _evfZoomRect; }	
    EdsUInt32 getEvfAFMode() const              { return _evfAFMode; }
    EdsChar  *getModelName()                    { return _modelName; }
    EdsFocusInfo getFocusInfo()const            { return _focusInfo; }

    string const& str_getAEMode() const                 { return propIdToString(_props._aemodeMap, _AEMode); }
    string const& str_getTv() const                     { return propIdToString(_props._tvMap, _Tv); }
    string const& str_getAv() const                     { return propIdToString(_props._avMap, _Av); }
    string const& str_getIso() const                    { return propIdToString(_props._isoMap, _Iso); }
    string const& str_getMeteringMode() const           { return propIdToString(_props._metermodeMap, _MeteringMode); }
    string const& str_getExposureCompensation() const   { return propIdToString(_props._exposurecompMap, _ExposureCompensation); }
    string const& str_getImageQuality() const           { return propIdToString(_props._imagequalityMap, _ImageQuality); }
    //EdsUInt32 str_getEvfMode() const                { return propIdToString(_props._evfmodeMap, _evfMode); }
    //EdsUInt32 str_getEvfOutputDevice() const        { return propIdToString(_props.Map, _evfOutputDevice); }
    //EdsUInt32 str_getEvfDepthOfFieldPreview() const { return propIdToString(_props.Map, _evfDepthOfFieldPreview); }
    //EdsUInt32 str_getEvfZoom() const                { return propIdToString(_props.Map, _evfZoom); }	
    string const& str_getEvfAFMode() const              { return propIdToString(_props._evfafmodeMap, _evfAFMode); }

    //List of value in which taking a picture parameter can be set
    EdsPropertyDesc getAEModeDesc() const                { return _AEModeDesc;}
    EdsPropertyDesc getAvDesc() const                    { return _AvDesc;}
    EdsPropertyDesc getTvDesc()	const                    { return _TvDesc;}
    EdsPropertyDesc getIsoDesc() const                   { return _IsoDesc;}
    EdsPropertyDesc getMeteringModeDesc() const          { return _MeteringModeDesc;}
    EdsPropertyDesc getExposureCompensationDesc() const	 { return _ExposureCompensationDesc;}
    EdsPropertyDesc getImageQualityDesc() const          { return _ImageQualityDesc;}
    EdsPropertyDesc getEvfAFModeDesc() const             { return _evfAFModeDesc;}

    //List of value in which taking a picture parameter can be set
    void setAEModeDesc(const EdsPropertyDesc* desc)               { _AEModeDesc = *desc;  }
    void setAvDesc(const EdsPropertyDesc* desc)                   { _AvDesc = *desc;      }
    void setTvDesc(const EdsPropertyDesc* desc)                   { _TvDesc = *desc;      }
    void setIsoDesc(const EdsPropertyDesc* desc)                  { _IsoDesc = *desc;     }
    void setMeteringModeDesc(const EdsPropertyDesc* desc)         { _MeteringModeDesc = *desc;  }
    void setExposureCompensationDesc(const EdsPropertyDesc* desc) { _ExposureCompensationDesc = *desc; }
    void setImageQualityDesc(const EdsPropertyDesc* desc)         { _ImageQualityDesc = *desc;         }
    void setEvfAFModeDesc(const EdsPropertyDesc* desc)            { _evfAFModeDesc = *desc;            }


    string descStr(PropertyBag const& propmap, EdsPropertyDesc const& desc) {
        string descstr;
        for (int i = 0; i < desc.numElements; i++) {
            descstr += propIdToString(propmap, desc.propDesc[i]);
            if (i!=desc.numElements-1) descstr += ", ";
        }
        return descstr;
    }

    /*
    vector<string> descList(PropertyMap const& propmap, EdsPropertyDesc const& desc) {
        vector<string> desclist;
        for (int i = 0; i < desc.numElements; i++) {
            desclist.push_back(propIdToString(propmap, desc.propDesc[i]));
        }
        return desclist;
    }
    */


public:
    //Setting of taking a picture parameter(UInt32)
    void setPropertyUInt32(EdsUInt32 propertyID, EdsUInt32 value)	
    {
        switch(propertyID) 
            {
            case kEdsPropID_AEModeSelect:            setAEMode(value); break;
            case kEdsPropID_Tv:                      setTv(value); break;		               
            case kEdsPropID_Av:                      setAv(value); break;           	  
            case kEdsPropID_ISOSpeed:                setIso(value); break;       
            case kEdsPropID_MeteringMode:            setMeteringMode(value); break;       
            case kEdsPropID_ExposureCompensation:    setExposureCompensation(value); break;
            case kEdsPropID_ImageQuality:            setImageQuality(value); break;
            case kEdsPropID_Evf_Mode:                setEvfMode(value); break;
            case kEdsPropID_Evf_OutputDevice:        setEvfOutputDevice(value); break;
            case kEdsPropID_Evf_DepthOfFieldPreview: setEvfDepthOfFieldPreview(value); break;	
            case kEdsPropID_Evf_AFMode:              setEvfAFMode(value); break;
            }
    }

    //Setting of taking a picture parameter(String)
    void setPropertyString(EdsUInt32 propertyID, EdsChar *str) {	
        switch(propertyID) {
        case kEdsPropID_ProductName:			setModelName(str);					break;
        }
    }

    void setProeprtyFocusInfo(EdsUInt32 propertyID, EdsFocusInfo info) {
        switch(propertyID) {
        case kEdsPropID_FocusInfo:				setFocusInfo(info);				break;
        }
    }

    //Setting of value list that can set taking a picture parameter
    void setPropertyDesc(EdsUInt32 propertyID, const EdsPropertyDesc* desc) {
        switch(propertyID) 
            {
            case kEdsPropID_AEModeSelect:			setAEModeDesc(desc); break;
            case kEdsPropID_Tv:					setTvDesc(desc); break;		               
            case kEdsPropID_Av:					setAvDesc(desc); break;           	  
            case kEdsPropID_ISOSpeed:				setIsoDesc(desc); break;       
            case kEdsPropID_MeteringMode:			setMeteringModeDesc(desc); break;       
            case kEdsPropID_ExposureCompensation:	        setExposureCompensationDesc(desc); break;       
            case kEdsPropID_ImageQuality:			setImageQualityDesc(desc); break;   
            case kEdsPropID_Evf_AFMode:				setEvfAFModeDesc(desc); break;
            }	
    }

    //Acquisition of value list that can set taking a picture parameter
    EdsPropertyDesc getPropertyDesc(EdsUInt32 propertyID)	
    {
        EdsPropertyDesc desc = {0};
        switch(propertyID) {
            case kEdsPropID_AEModeSelect:         desc = getAEModeDesc(); break;
            case kEdsPropID_Tv:                   desc = getTvDesc(); break;		               
            case kEdsPropID_Av:                   desc = getAvDesc(); break;           	  
            case kEdsPropID_ISOSpeed:             desc = getIsoDesc(); break;       
            case kEdsPropID_MeteringMode:         desc = getMeteringModeDesc(); break;       
            case kEdsPropID_ExposureCompensation: desc = getExposureCompensationDesc(); break;       
            case kEdsPropID_ImageQuality:         desc = getImageQualityDesc(); break;    
            case kEdsPropID_Evf_AFMode:           desc = getEvfAFModeDesc(); break;    
        }
        return desc;
    }


public:
    virtual bool isLegacy() {
        return _is_legacy;
    }

    bool command_takePicture_FixedBulb(double bulb_time) {
        //debug("Starting to take picture\n");

        EdsError err = EDS_ERR_OK;
        bool locked = false;
		
        // For cameras earlier than the 30D , the UI must be locked before commands are reissued
        err = EdsSendStatusCommand(getCameraObject(), kEdsCameraStatusCommand_UILock, 0);
        
        if(err == EDS_ERR_OK) locked = true;
		
        //Taking a picture
        //err = EdsSendCommand(getCameraObject(), kEdsCameraCommand_BulbStart, 0);
        command_pressShutter(kEdsCameraCommand_ShutterButton_Completely);
            //if (err != EDS_ERR_OK) {
            //debug("BulbStart error %x\n", err);
            //}
        timespec tm;
        tm.tv_sec = static_cast<int>(bulb_time);
        tm.tv_nsec = (bulb_time - tm.tv_sec)*1e9;
        nanosleep(&tm, NULL);
        command_pressShutter(kEdsCameraCommand_ShutterButton_Completely);
            //err = EdsSendCommand(getCameraObject(), kEdsCameraCommand_BulbEnd, 0);
            //if (err != EDS_ERR_OK) {
            //debug("BulbEnd error %x\n", err);
            //}

        //It releases it when locked
        if (locked) {
            err = EdsSendStatusCommand(getCameraObject(), kEdsCameraStatusCommand_UIUnLock, 0);
        }

        //Notification of error
        if(err != EDS_ERR_OK) {
            // It retries it at device busy
            if(err == EDS_ERR_DEVICE_BUSY) return false;
            debug("takePicture error: %x\n", err);
            return false;
        }

        // start event loop, which will be stopped when download is complete.
        // make this configurable if we're not downloading
        //eventloop(); // prob should only do if we need to transfer

        //debug("Leaving takePicture\n");

        return true;
    }
    
    bool command_takePicture() {
        //debug("Starting to take picture\n");

        EdsError err = EDS_ERR_OK;
        bool locked = false;
		
        // For cameras earlier than the 30D , the UI must be locked before commands are reissued
        if( isLegacy()) {
            err = EdsSendStatusCommand(getCameraObject(), kEdsCameraStatusCommand_UILock, 0);
        
            if(err == EDS_ERR_OK) {
                locked = true;
            }		
        }
		
        //Taking a picture
        err = EdsSendCommand(getCameraObject(), kEdsCameraCommand_TakePicture, 0);

        //It releases it when locked
        if (locked) {
            err = EdsSendStatusCommand(getCameraObject(), kEdsCameraStatusCommand_UIUnLock, 0);
        }

        //Notification of error
        if(err != EDS_ERR_OK) {
            // It retries it at device busy
            if(err == EDS_ERR_DEVICE_BUSY) {
                //debug("takePicture DeviceBusy\n");
                return false;
            }
			
            debug("takePicture error: %x\n", err);
            return false;
        }

        // start event loop, which will be stopped when download is complete.
        // make this configurable if we're not downloading
        //eventloop(); // prob should only do if we need to transfer

        //debug("Leaving takePicture\n");

        return true;
    }

    static EdsError EDSCALLBACK ProgressFunc (EdsUInt32 inPercent, EdsVoid* inContext, EdsBool* outCancel) {
        //debug("ProgressFunc %u\n", inPercent);
        return EDS_ERR_OK;
    }


    string command_download(EdsDirectoryItemRef _directoryItem, const char *to_file=NULL) {
        EdsError err = EDS_ERR_OK;
        EdsStreamRef stream = NULL;

        //Acquisition of the downloaded image information
        EdsDirectoryItemInfo	dirItemInfo;
        err = EdsGetDirectoryItemInfo( _directoryItem, &dirItemInfo);
	
        string file;
        if (to_file)
            file = string(to_file);
        else {
            bf::path p(dirItemInfo.szFileName);
            string ext = p.extension().string();
            char fname[11+ext.size()];
            sprintf(fname, "IMG_%06d%s", img_num(ext), ext.c_str());
            file = _savedir + "/" + string(fname);
        }

        // Forwarding beginning notification	
        if(err == EDS_ERR_OK) {
            debug("DownloadStart %s %d\n", file.c_str(), dirItemInfo.size);
        }

        //Make the file stream at the forwarding destination
        if(err == EDS_ERR_OK) {	
            err = EdsCreateFileStream(file.c_str(), kEdsFileCreateDisposition_CreateAlways, kEdsAccess_ReadWrite, &stream);
        }	

        //Set Progress
        if(err == EDS_ERR_OK) {
            // or Periodically or NoReport
            err = EdsSetProgressCallback(stream, ProgressFunc, kEdsProgressOption_Done, this);
        }


        //Download image
        if(err == EDS_ERR_OK) {
            err = EdsDownload( _directoryItem, dirItemInfo.size, stream);
        }

        //Forwarding completion
        if(err == EDS_ERR_OK) {
            err = EdsDownloadComplete(_directoryItem);
        }

        //Release Item
        if(_directoryItem != NULL) {
            err = EdsRelease( _directoryItem);
            _directoryItem = NULL;
        }

        //EdsImageRef imref=NULL;
        //err = EdsCreateImageRef(stream, &imref);
        //if (err != EDS_ERR_OK) {
        //    debug("error creating imref %x\n", err);
        //}

        //Release stream
        if(stream != NULL) {
            err = EdsRelease(stream);
            stream = NULL;
        }		
    
        // Forwarding completion notification
        //if( err == EDS_ERR_OK) { debug("DownloadComplete\n"); }

        //Notification of error
        if( err != EDS_ERR_OK) { debug("download error %x\n", err); }

        //Release item
        if(_directoryItem != NULL) {
            EdsRelease( _directoryItem);
            _directoryItem = NULL;
        }

        if( err != EDS_ERR_OK) {
            return "";
        }

        stoploop();

        //EdsStreamRef stream2 = NULL;
        /*
        err = EdsCreateFileStream(file.c_str(), kEdsFileCreateDisposition_OpenExisting, kEdsAccess_ReadWrite, &stream2);
        if (err != EDS_ERR_OK) {
            debug("error creating stream\n");
        }
        */


        /*
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(file);
        assert(image.get() != 0);
        image->readMetadata();
        debug("Time=%s, Aperture=%s\n", meta(image, "Exif.Photo.ExposureTime").c_str(), meta(image, "Exif.Photo.ApertureValue").c_str());
        */

        /*
        EdsPropertyID propertyID = kEdsPropID_Tv;
        EdsDataType	dataType = kEdsDataType_Unknown;
        EdsUInt32   dataSize = 0;
        EdsUInt32 inParam=0;

        //Acquisition of the property size
        if(err == EDS_ERR_OK) {
            err = EdsGetPropertySize( imref,
                                      propertyID,
                                      inParam,
                                      &dataType,
                                      &dataSize );
        }

        if (err != EDS_ERR_OK) {
            debug("error getting prop size %x\n", err);
        }
        debug("data type = %x, data size = %x\n", dataType, dataSize);
        */

        /*
        if(err == EDS_ERR_OK) {
			
            if(dataType == kEdsDataType_UInt32) {
                EdsUInt32 data;

                //Acquisition of the property
                err = EdsGetPropertyData( getCameraObject(),
                                          propertyID,
                                          inParam,
                                          dataSize,
                                          &data );
                
                //Acquired property value is set
                if(err == EDS_ERR_OK) {
                    setPropertyUInt32(propertyID, data);
                }
            }
			
            if(dataType == kEdsDataType_String) {
                EdsChar str[EDS_MAX_NAME];
                //Acquisition of the property
                err = EdsGetPropertyData( getCameraObject(),
                                          propertyID,
                                          0,
                                          dataSize,
                                          str );
                
                //Acquired property value is set
                if(err == EDS_ERR_OK) {
                    setPropertyString(propertyID, str);
                }			
            }
            if(dataType == kEdsDataType_FocusInfo) {
                EdsFocusInfo focusInfo;
                //Acquisition of the property
                err = EdsGetPropertyData( getCameraObject(),
                                          propertyID,
                                          0,
                                          dataSize,
                                          &focusInfo );
                
                //Acquired property value is set
                if(err == EDS_ERR_OK) {
                    setFocusInfo(focusInfo);
                }		
            }
        }
        */

        return file;
    }


    bool command_openSession(EdsSaveTo saveTo = kEdsSaveTo_Host) {
        EdsError err = EDS_ERR_OK;
        bool	 locked = false;
	
        //The communication with the camera begins
        err = EdsOpenSession(getCameraObject());
	
        if(isLegacy()) {
            //Preservation ahead is set to PC
            if(err == EDS_ERR_OK) {
                err = EdsSetPropertyData(getCameraObject(), kEdsPropID_SaveTo, 0, sizeof(saveTo) , &saveTo);
            }
        
            command_getProperty(kEdsPropID_Unknown);
            command_getPropertyDesc(kEdsPropID_Unknown);
        } else {
            if(err == EDS_ERR_OK) {
                err = EdsSetPropertyData(getCameraObject(), kEdsPropID_SaveTo, 0, sizeof(saveTo) , &saveTo);
            }

            //UI lock
            if(err == EDS_ERR_OK) {
                err = EdsSendStatusCommand(getCameraObject(), kEdsCameraStatusCommand_UILock, 0);
            }

            if(err == EDS_ERR_OK) {
                locked = true;
            }
			
            if(err == EDS_ERR_OK) {
                EdsCapacity capacity = {0x7FFFFFFF, 0x1000, 1};
                err = EdsSetCapacity( getCameraObject(), capacity);
            }
			
            //It releases it when locked
            if(locked) {
                EdsSendStatusCommand(getCameraObject(), kEdsCameraStatusCommand_UIUnLock, 0);
            }	
        }

        //Notification of error
        if(err != EDS_ERR_OK) {
            debug("openSession error: %x\n", err);
            return false;
        }

        return true;
    }

    bool command_closeSession() {
        EdsError err = EDS_ERR_OK;
	
        //The communication with the camera is ended
        err = EdsCloseSession(getCameraObject());

        //Notification of error
        if(err != EDS_ERR_OK) {
            debug("closeSession error: %x\n", err);
            return false;
        }

        return true;
    }

    bool command_getProperty(EdsPropertyID propertyID, EdsUInt32 inParam=0) {
        EdsError err = EDS_ERR_OK;
        bool	 locked = false;
	
        // For cameras earlier than the 30D , the UI must be locked before commands are reissued
        if( isLegacy()) {
            err = EdsSendStatusCommand(getCameraObject(), kEdsCameraStatusCommand_UILock, 0);
            if(err == EDS_ERR_OK) locked = true;
        }		
		
        //Get property value
        if(err == EDS_ERR_OK) {
            err = _getProperty(propertyID, inParam);
        }

        //It releases it when locked
        if(locked) {
            EdsSendStatusCommand(getCameraObject(), kEdsCameraStatusCommand_UIUnLock, 0);
        }

        //Notification of error
        if(err != EDS_ERR_OK) {
            // It retries it at device busy
            if((err & EDS_ERRORID_MASK) == EDS_ERR_DEVICE_BUSY ) {
                debug("getProperty DeviceBusy\n");
                return false;
            }

            debug("getProperty error %x, propid=%x\n", err, propertyID);
        }

        return true;	
	
    }

    EdsError _getProperty(EdsPropertyID propertyID, EdsUInt32 inParam=0) {
        EdsError err = EDS_ERR_OK;
        EdsDataType	dataType = kEdsDataType_Unknown;
        EdsUInt32   dataSize = 0;


        if(propertyID == kEdsPropID_Unknown) {
            //If unknown is returned for the property ID , the required property must be retrieved again
            if(err == EDS_ERR_OK) err = _getProperty(kEdsPropID_AEModeSelect);
            if(err == EDS_ERR_OK) err = _getProperty(kEdsPropID_Tv);
            if(err == EDS_ERR_OK) err = _getProperty(kEdsPropID_Av);
            if(err == EDS_ERR_OK) err = _getProperty(kEdsPropID_ISOSpeed);
            if(err == EDS_ERR_OK) err = _getProperty(kEdsPropID_MeteringMode);
            if(err == EDS_ERR_OK) err = _getProperty(kEdsPropID_ExposureCompensation);
            if(err == EDS_ERR_OK) err = _getProperty(kEdsPropID_ImageQuality);
            
            return err;
        }
	
        //Acquisition of the property size
        if(err == EDS_ERR_OK) {
            err = EdsGetPropertySize( getCameraObject(),
                                      propertyID,
                                      inParam,
                                      &dataType,
                                      &dataSize );
        }

        if(err == EDS_ERR_OK) {
			
            if(dataType == kEdsDataType_UInt32) {
                EdsUInt32 data;

                //Acquisition of the property
                err = EdsGetPropertyData( getCameraObject(),
                                          propertyID,
                                          inParam,
                                          dataSize,
                                          &data );
                
                //Acquired property value is set
                if(err == EDS_ERR_OK) {
                    setPropertyUInt32(propertyID, data);
                }
            }
			
            if(dataType == kEdsDataType_String) {
                EdsChar str[EDS_MAX_NAME];
                //Acquisition of the property
                err = EdsGetPropertyData( getCameraObject(),
                                          propertyID,
                                          0,
                                          dataSize,
                                          str );
                
                //Acquired property value is set
                if(err == EDS_ERR_OK) {
                    setPropertyString(propertyID, str);
                }			
            }
            if(dataType == kEdsDataType_FocusInfo) {
                EdsFocusInfo focusInfo;
                //Acquisition of the property
                err = EdsGetPropertyData( getCameraObject(),
                                          propertyID,
                                          0,
                                          dataSize,
                                          &focusInfo );
                
                //Acquired property value is set
                if(err == EDS_ERR_OK) {
                    setFocusInfo(focusInfo);
                }		
            }
        }

        //Update notification
        //if(err == EDS_ERR_OK) {
            //debug("PropertyChanged %x\n", propertyID);
        //}

        //_property_changed[propertyID] = true;

        return err;
    }

    // Execute command	
    bool command_getPropertyDesc(EdsPropertyID propertyID) {
        EdsError err = EDS_ERR_OK;
        bool	 locked = false;
		
        // For cameras earlier than the 30D , the UI must be locked before commands are reissued
        if(isLegacy()) {
            err = EdsSendStatusCommand(getCameraObject(), kEdsCameraStatusCommand_UILock, 0);
            
            if(err == EDS_ERR_OK) {
                locked = true;
            }		
        }		
	
        //Get property
        if(err == EDS_ERR_OK) {
            err = _getPropertyDesc(propertyID);
        }
        
        //It releases it when locked
        if(locked) {
            EdsSendStatusCommand(getCameraObject(), kEdsCameraStatusCommand_UIUnLock, 0);
        }
        
        
        //Notification of error
        if(err != EDS_ERR_OK) {
            // It retries it at device busy
            if((err & EDS_ERRORID_MASK) == EDS_ERR_DEVICE_BUSY) {
                debug("DeviceBusy\n");
                return false;
            }
                
            debug("getPropertyDesc error %x\n", err);
        }
        
        return true;
    }	
	
    EdsError _getPropertyDesc(EdsPropertyID propertyID) {
        EdsError  err = EDS_ERR_OK;
        EdsPropertyDesc	 propertyDesc = {0};
        
        if(propertyID == kEdsPropID_Unknown) {
            //If unknown is returned for the property ID , the required property must be retrieved again
            if(err == EDS_ERR_OK) err = _getPropertyDesc(kEdsPropID_AEModeSelect);
            if(err == EDS_ERR_OK) err = _getPropertyDesc(kEdsPropID_Tv);
            if(err == EDS_ERR_OK) err = _getPropertyDesc(kEdsPropID_Av);
            if(err == EDS_ERR_OK) err = _getPropertyDesc(kEdsPropID_ISOSpeed);
            if(err == EDS_ERR_OK) err = _getPropertyDesc(kEdsPropID_MeteringMode);
            if(err == EDS_ERR_OK) err = _getPropertyDesc(kEdsPropID_ExposureCompensation);			
            if(err == EDS_ERR_OK) err = _getPropertyDesc(kEdsPropID_ImageQuality);			
            
            return err;
        }		
	
        //Acquisition of value list that can be set
        if(err == EDS_ERR_OK) {
            err = EdsGetPropertyDesc( getCameraObject(),
                                      propertyID,
                                      &propertyDesc);
        }

        //The value list that can be the acquired setting it is set		
        if(err == EDS_ERR_OK) {
            setPropertyDesc(propertyID , &propertyDesc);
        }
        
        //Update notification
        if(err == EDS_ERR_OK) {
            //debug("PropertyDescChanged %x\n",  propertyID);
        }
        
        return err;
    }

    bool checkDesc(EdsInt32 data, EdsPropertyDesc const& desc) {
        for (int i = 0; i < desc.numElements; i++) {
            if (data == desc.propDesc[i]) return true;
            //debug("Not %x %x\n", data, desc.propDesc[i]);
	}

        return false;
    }

    bool command_setAEMode(string strdata) {
        return command_setStuff(_props._aemodeMap, kEdsPropID_AEMode, getAEModeDesc(), strdata);
    }

    bool command_setTv(string strdata) {
        return command_setStuff(_props._tvMap, kEdsPropID_Tv, getTvDesc(), strdata);
    }

    bool command_setTv(EdsInt32 data) {
        return command_setProperty(kEdsPropID_Tv, data);
        //return command_setStuff(_props._tvMap, , getTvDesc(), strdata);
    }

    bool command_setAv(string strdata) {
        return command_setStuff(_props._avMap, kEdsPropID_Av, getAvDesc(), strdata);
    }
    
    bool command_setIso(string strdata) {
        return command_setStuff(_props._isoMap, kEdsPropID_ISOSpeed,  getIsoDesc(), strdata);
    }

    bool command_setMeteringMode(string strdata) {
        return command_setStuff(_props._metermodeMap, kEdsPropID_MeteringMode, getMeteringModeDesc(), strdata);
    }

    bool command_setExposureCompensation(string strdata) {
        return command_setStuff(_props._exposurecompMap, kEdsPropID_ExposureCompensation, getExposureCompensationDesc(), strdata);
    }

    bool command_setImageQuality(string strdata) {
        return command_setStuff(_props._imagequalityMap, kEdsPropID_ImageQuality, getImageQualityDesc(), strdata);
    }

    bool command_setEvfAFMode(string strdata) {
        return command_setStuff(_props._evfafmodeMap, kEdsPropID_Evf_AFMode, getEvfAFModeDesc(), strdata);
    }

    bool command_setStuff(PropertyBag const& propmap, EdsPropertyID propertyID, EdsPropertyDesc const& desc, string const& strdata) {
        EdsUInt32 data = propmap.code_from_string(strdata, desc);
        //EdsInt32 propertyID = propmap.code_from_string(strdata, desc);
        if (data == 0xffffffff) {
            debug("Unable to set property (%x) to %s - not in description list (%s)\n", propertyID, strdata.c_str(), descStr(propmap, desc).c_str());
            exit(1);
            return false;
        } else { 
            debug("About to set property (%x) to %s (%x)\n", propertyID, strdata.c_str(), data);
            return command_setProperty(propertyID, data);
        }

        /*
        if (checkDesc(data, desc)) {
        } else {
        }
        */
    }
        
            
    template<typename T>
    bool command_setProperty(EdsPropertyID _propertyID, T _data) {
        EdsError err = EDS_ERR_OK;
        bool locked = false;
	
        // For cameras earlier than the 30D , the UI must be locked before commands are reissued
        if( isLegacy() ) {
            err = EdsSendStatusCommand(getCameraObject(), kEdsCameraStatusCommand_UILock, 0);
		
            if(err == EDS_ERR_OK) locked = true;
        }
    
        // Set property
        if(err == EDS_ERR_OK) {		
            err = EdsSetPropertyData(getCameraObject(),
                                     _propertyID,
                                     0,
                                     sizeof(_data),
                                     (EdsVoid *)&_data );
        }
    
        //It releases it when locked
        if(locked) {
            EdsSendStatusCommand(getCameraObject(), kEdsCameraStatusCommand_UIUnLock, 0);
        } 
    
        //Notification of error
        if(err != EDS_ERR_OK) {
            // It retries it at device busy
            if(err == EDS_ERR_DEVICE_BUSY) {
                debug("DeviceBusy\n");
                return false;
            }

            debug("setProperty error %x\n", err);
        }

        return true;
    }
    
    // Execute command	
    bool command_setCapacity(EdsCapacity capacity) {
        // It is a function only of the model since 30D.
        if(!isLegacy()) {
            EdsError err = EDS_ERR_OK;

            //Acquisition of the number of sheets that can be taken a picture
            if(err == EDS_ERR_OK) {
                err = EdsSetCapacity( getCameraObject(), capacity);
            }

            //Notification of error
            if(err != EDS_ERR_OK) {
                debug("setCapacity error %x\n", err);
            }
        }

        return true;
    }

    // Execute command
    bool command_pressShutter(EdsUInt32 status) {
        EdsError err = EDS_ERR_OK;

        //PressShutterButton
        if(err == EDS_ERR_OK) {
            err = EdsSendCommand(getCameraObject(), kEdsCameraCommand_PressShutterButton, status);
        }

        //Notification of error
        if(err != EDS_ERR_OK) {
            // It retries it at device busy
            if(err == EDS_ERR_DEVICE_BUSY) return true;
            
            debug("pressShutter error %x\n", err);
        }
        
        return true;
    }

    bool command_extendShutdownTimer() {
        EdsError err = EDS_ERR_OK;

        debug("Extending shutdown timer\n");
        err = EdsSendCommand(getCameraObject(), kEdsCameraCommand_ExtendShutDownTimer, 0);

        if(err != EDS_ERR_OK) {
            // It retries it at device busy
            if(err == EDS_ERR_DEVICE_BUSY) {
                debug("extendShutdownTimer DeviceBusy\n");
                return true;
            }
            
            debug("extendShutdownTimer error %x\n", err);
        }

        return true;
    }




};



