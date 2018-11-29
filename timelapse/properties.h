#pragma once

#include "EDSDK.h"
#include "EDSDKTypes.h"
#include "log.h"

#include <map>
#include <string>
#include <cmath>
//#include <utility>

using namespace std;

struct PropertyValueType {
    PropertyValueType(EdsInt32 code, string desc, double value=0) 
        : _code(code)
        , _desc(desc)
        , _value(value)
    {}

    EdsInt32 _code;
    string _desc;
    double _value;
};

typedef std::map<EdsInt32, PropertyValueType> PropertyMap;

struct PropertyBag {
    map<EdsInt32, PropertyValueType > _map;

    void add(EdsInt32 code, string desc, double value=0) {
        PropertyValueType propvalue(code, desc, value);
        _map.insert(make_pair(code, propvalue));
    }

    double value(EdsInt32 code) {
        PropertyMap::iterator it = _map.find(code);
        if (it != _map.end()) {
            return it->second._value;
        } else {
            throw string("Poop, couldn't get value from code\n");
        }
    }

    EdsInt32 next_highest(double value, EdsPropertyDesc list) {
        double min = 1e12;
        EdsInt32 min_code, code;
        for (int i = 0; i < list.numElements; i++) {
            code = list.propDesc[i];
            PropertyMap::iterator it = _map.find(code);

            if (it != _map.end()) {
                if (it->second._value > value and it->second._value < min) {
                    min = it->second._value;
                    min_code = it->second._code;
                }
            } else {
                debug("Couldn't find value for code %x, that's odd\n", code);
            }
        }

        return min_code;
    }

    EdsInt32 next_lowerst(double value, EdsPropertyDesc list) {
        double max = -1e12;
        EdsInt32 max_code, code;
        for (int i = 0; i < list.numElements; i++) {
            code = list.propDesc[i];
            PropertyMap::iterator it = _map.find(code);

            if (it != _map.end()) {
                if (it->second._value < value and it->second._value > max) {
                    max = it->second._value;
                    max_code = it->second._code;
                }
            } else {
                debug("Couldn't find value for code %x, that's odd\n", code);
            }
        }

        return max_code;
    }


    EdsInt32 code_nearest(double value, EdsPropertyDesc list) {
        EdsInt32 code, nearest_code = 0xffffffff;;
        double nearest = 1e100;

        for (int i = 0; i < list.numElements; i++) {
            code = list.propDesc[i];
            PropertyMap::iterator it = _map.find(code);

            if (it != _map.end()) {
                if (std::abs(value - it->second._value) < nearest) {
                    nearest = std::abs(value - it->second._value);
                    nearest_code = code;
                }
            } else {
                debug("Couldn't find value for code %x, that's odd\n", code);
            }
        }
        
        return nearest_code;
    }

    EdsInt32 code_from_string(string const desc, EdsPropertyDesc const list) const {
        EdsInt32 code;
        for (int i = 0; i < list.numElements; i++) {
            code = list.propDesc[i];
            PropertyMap::const_iterator it = _map.find(code);

            if (it != _map.end()) {
                if (it->second._desc == desc) 
                    return code;
            }
        }
        
        return 0xffffffff;
    }
};



class Properties {
public:
    Properties() {
        _tvMap.add(0x0c,"Bulb", 1e6);
        _tvMap.add(0x10, "30\"", 30);
        _tvMap.add(0x13, "25\"", 25);
        _tvMap.add(0x14, "20\"", 20);
        _tvMap.add(0x15, "20\"", 20);
        _tvMap.add(0x18, "15\"", 15);
        _tvMap.add(0x1B, "13\"", 13);
        _tvMap.add(0x1C, "10\"", 10);
        _tvMap.add(0x1D, "10\"", 10);
        _tvMap.add(0x20, "8\"", 8);
        _tvMap.add(0x23, "6\"", 6);
        _tvMap.add(0x24, "6\"", 6);
        _tvMap.add(0x25, "5\"", 5);
        _tvMap.add(0x28,"4\"", 4);
        _tvMap.add(0x2B,"3\"2", 3.2);
        _tvMap.add(0x2C,"3\"", 3);
        _tvMap.add(0x2D,"2\"5", 2.5);
        _tvMap.add(0x30,"2\"", 2);
        _tvMap.add(0x33,"1\"6", 1.6);
        _tvMap.add(0x34,"1\"5", 1.5);
        _tvMap.add(0x35,"1\"3", 1.3);
        _tvMap.add(0x38,"1\"", 1);
        _tvMap.add(0x3B,"0\"8", 0.8);
        _tvMap.add(0x3C,"0\"7", 0.7);
        _tvMap.add(0x3D,"0\"6", 0.6);
        _tvMap.add(0x40,"0\"5", 0.5);
        _tvMap.add(0x43,"0\"4", 0.4);
        _tvMap.add(0x44,"0\"3", 0.3);
        _tvMap.add(0x45,"0\"3", 0.3);
        _tvMap.add(0x48,"4", 1.0/4);
        _tvMap.add(0x4B,"5", 1.0/5);
        _tvMap.add(0x4C,"6", 1.0/6);
        _tvMap.add(0x4D,"6", 1.0/6);
        _tvMap.add(0x50,"8", 1.0/8);
        _tvMap.add(0x53,"10", 1.0/10);
        _tvMap.add(0x54,"10", 1.0/10);
        _tvMap.add(0x55,"13", 1.0/13);
        _tvMap.add(0x58,"15", 1.0/15);
        _tvMap.add(0x5B,"20", 1.0/20);
        _tvMap.add(0x5C,"20", 1.0/20);
        _tvMap.add(0x5D,"25", 1.0/25);
        _tvMap.add(0x60,"30", 1.0/30);
        _tvMap.add(0x63,"40", 1.0/40);
        _tvMap.add(0x64,"45", 1.0/45);
        _tvMap.add(0x65,"50", 1.0/50);
        _tvMap.add(0x68,"60", 1.0/60);
        _tvMap.add(0x6B,"80", 1.0/80);
        _tvMap.add(0x6C,"90", 1.0/90);
        _tvMap.add(0x6D,"100", 1.0/100);
        _tvMap.add(0x70,"125", 1.0/125);
        _tvMap.add(0x73,"160", 1.0/160);
        _tvMap.add(0x74,"180", 1.0/180);
        _tvMap.add(0x75,"200", 1.0/200);
        _tvMap.add(0x78,"250", 1.0/250);
        _tvMap.add(0x7B,"320", 1.0/320);
        _tvMap.add(0x7C,"350", 1.0/350);
        _tvMap.add(0x7D,"400", 1.0/400);
        _tvMap.add(0x80,"500", 1.0/500);
        _tvMap.add(0x83,"640", 1.0/640);
        _tvMap.add(0x84,"750", 1.0/750);
        _tvMap.add(0x85,"800", 1.0/800);
        _tvMap.add(0x88,"1000", 1.0/1000);
        _tvMap.add(0x8B,"1250", 1.0/1250);
        _tvMap.add(0x8C,"1500", 1.0/1500);
        _tvMap.add(0x8D,"1600", 1.0/1600);
        _tvMap.add(0x90,"2000", 1.0/2000);
        _tvMap.add(0x93,"2500", 1.0/2500);
        _tvMap.add(0x94,"3000", 1.0/3000);
        _tvMap.add(0x95,"3200", 1.0/3200);
        _tvMap.add(0x98,"4000", 1.0/4000);
        _tvMap.add(0x9B,"5000", 1.0/5000);
        _tvMap.add(0x9C,"6000", 1.0/6000);
        _tvMap.add(0x9D,"6400", 1.0/6400);
        _tvMap.add(0xA0,"8000", 1.0/8000);
        _tvMap.add(0xffffffff,"unknown");

        _aemodeMap.add(0,"P");
        _aemodeMap.add(1,"Tv");
        _aemodeMap.add(2,"Av");
        _aemodeMap.add(3,"M");
        _aemodeMap.add(4,"Bulb");
        _aemodeMap.add(5,"A-DEP");
        _aemodeMap.add(6,"DEP");
        _aemodeMap.add(7,"C1");
        _aemodeMap.add(16,"C2");
        _aemodeMap.add(17,"C3");
        _aemodeMap.add(8,"Lock");
        _aemodeMap.add(9,"GreenMode");
        _aemodeMap.add(10,"Night Portrait");
        _aemodeMap.add(11,"Sports");
        _aemodeMap.add(13,"LandScape");
        _aemodeMap.add(14,"Close Up");
        _aemodeMap.add(15,"No Strobo");
        _aemodeMap.add(12,"Portrait");
        _aemodeMap.add(19,"Creative Auto");
        _aemodeMap.add(0xffffffff,"unknown");

        _avMap.add(0x00,"00");
        _avMap.add(0x08,"1");
        _avMap.add(0x0B,"1.1");
        _avMap.add(0x0C,"1.2");
        _avMap.add(0x0D,"1.2");
        _avMap.add(0x10,"1.4");
        _avMap.add(0x13,"1.6");
        _avMap.add(0x14,"1.8");
        _avMap.add(0x15,"1.8");
        _avMap.add(0x18,"2");
        _avMap.add(0x1B,"2.2");
        _avMap.add(0x1C,"2.5");
        _avMap.add(0x1D,"2.5");
        _avMap.add(0x20,"2.8");
        _avMap.add(0x23,"3.2");
        _avMap.add(0x24,"3.5");
        _avMap.add(0x25,"3.5");
        _avMap.add(0x28,"4");
        _avMap.add(0x2B,"4.5");
        _avMap.add(0x2C,"4.5");
        _avMap.add(0x2D,"5.0");
        _avMap.add(0x30,"5.6");
        _avMap.add(0x33,"6.3");
        _avMap.add(0x34,"6.7");
        _avMap.add(0x35,"7.1");
        _avMap.add(0x38,"8");
        _avMap.add(0x3B,"9");
        _avMap.add(0x3C,"9.5");
        _avMap.add(0x3D,"10");
        _avMap.add(0x40,"11");
        _avMap.add(0x43,"13");
        _avMap.add(0x44,"13");
        _avMap.add(0x45,"14");
        _avMap.add(0x48,"16");
        _avMap.add(0x4B,"18");
        _avMap.add(0x4C,"19");
        _avMap.add(0x4D,"20");
        _avMap.add(0x50,"22");
        _avMap.add(0x53,"25");
        _avMap.add(0x54,"27");
        _avMap.add(0x55,"29");
        _avMap.add(0x58,"32");
        _avMap.add(0x5B,"36");
        _avMap.add(0x5C,"38");
        _avMap.add(0x5D,"40");
        _avMap.add(0x60,"45");
        _avMap.add(0x63,"51");
        _avMap.add(0x64,"54");
        _avMap.add(0x65,"57");
        _avMap.add(0x68,"64");
        _avMap.add(0x6B,"72");
        _avMap.add(0x6C,"76");
        _avMap.add(0x6D,"80");
        _avMap.add(0x70,"91");
        _avMap.add(0xffffffff,"unknown");

        _evfafmodeMap.add(0x00,"Quick mode");
        _evfafmodeMap.add(0x01,"Live mode");
        _evfafmodeMap.add(0x02,"Live face detection mode");
        _evfafmodeMap.add(0xffffffff,"unknown");

        _exposurecompMap.add(0x18,"+3");
        _exposurecompMap.add(0x15,"+2 2/3");
        _exposurecompMap.add(0x14,"+2 1/2");
        _exposurecompMap.add(0x13,"+2 1/3");
        _exposurecompMap.add(0x10,"+2");
        _exposurecompMap.add(0x0d,"+1 2/3");
        _exposurecompMap.add(0x0c,"+1 1/2");
        _exposurecompMap.add(0x0b,"+1 1/3");
        _exposurecompMap.add(0x08,"+1");
        _exposurecompMap.add(0x05,"+2/3");
        _exposurecompMap.add(0x04,"+1/2");
        _exposurecompMap.add(0x03,"+1/3");
        _exposurecompMap.add(0x00,"0");
        _exposurecompMap.add(0xfd,"-1/3");
        _exposurecompMap.add(0xfc,"-1/2");
        _exposurecompMap.add(0xfb,"-2/3");
        _exposurecompMap.add(0xf8,"-1");
        _exposurecompMap.add(0xf5,"-1 1/3");
        _exposurecompMap.add(0xf4,"-1 1/2");
        _exposurecompMap.add(0xf3,"-1 2/3");
        _exposurecompMap.add(0xf0,"-2");
        _exposurecompMap.add(0xed,"-2 1/3");
        _exposurecompMap.add(0xec,"-2 1/2");
        _exposurecompMap.add(0xeb,"-2 2/3");
        _exposurecompMap.add(0xe8,"-3");
        _exposurecompMap.add(0xffffffff,"unknown");

        _imagequalityMap.add(EdsImageQuality_LR,     "RAW");
        _imagequalityMap.add(EdsImageQuality_LRLJF,  "RAW + Large Fine Jpeg");
        _imagequalityMap.add(EdsImageQuality_LRMJF,  "RAW + Middle Fine Jpeg");
        _imagequalityMap.add(EdsImageQuality_LRSJF,  "RAW + Small Fine Jpeg");
        _imagequalityMap.add(EdsImageQuality_LRLJN,  "RAW + Large Normal Jpeg");
        _imagequalityMap.add(EdsImageQuality_LRMJN,  "RAW + Middle Normal Jpeg");
        _imagequalityMap.add(EdsImageQuality_LRSJN,  "RAW + Small Normal Jpeg");
        _imagequalityMap.add(EdsImageQuality_LRS1JF, "RAW + Small1 Fine Jpeg");
        _imagequalityMap.add(EdsImageQuality_LRS1JN, "RAW + Small1 Normal Jpeg");
        _imagequalityMap.add(EdsImageQuality_LRS2JF, "RAW + Small2 Jpeg");
        _imagequalityMap.add(EdsImageQuality_LRS3JF, "RAW + Small3 Jpeg");
	
        _imagequalityMap.add(EdsImageQuality_LRLJ,   "RAW + Large Jpeg");
        _imagequalityMap.add(EdsImageQuality_LRM1J,  "RAW + Middle1 Jpeg");
        _imagequalityMap.add(EdsImageQuality_LRM2J,  "RAW + Middle2 Jpeg");
        _imagequalityMap.add(EdsImageQuality_LRSJ,   "RAW + Small Jpeg");

        _imagequalityMap.add(EdsImageQuality_MR,     "Middle Raw");
        _imagequalityMap.add(EdsImageQuality_MRLJF,  "Middle Raw + Large Fine Jpeg");
        _imagequalityMap.add(EdsImageQuality_MRMJF,  "Middle Raw + Middle Fine Jpeg");
        _imagequalityMap.add(EdsImageQuality_MRSJF,  "Middle Raw + Small Fine Jpeg");
        _imagequalityMap.add(EdsImageQuality_MRLJN,  "Middle Raw + Large Normal Jpeg");
        _imagequalityMap.add(EdsImageQuality_MRMJN,  "Middle Raw + Middle Normal Jpeg");
        _imagequalityMap.add(EdsImageQuality_MRSJN,  "Middle Raw + Small Normal Jpeg");
        _imagequalityMap.add(EdsImageQuality_MRS1JF, "Middle RAW + Small1 Fine Jpeg");
        _imagequalityMap.add(EdsImageQuality_MRS1JN, "Middle RAW + Small1 Normal Jpeg");
        _imagequalityMap.add(EdsImageQuality_MRS2JF, "Middle RAW + Small2 Jpeg");
        _imagequalityMap.add(EdsImageQuality_MRS3JF, "Middle RAW + Small3 Jpeg");

        _imagequalityMap.add(EdsImageQuality_MRLJ,   "Middle Raw + Large Jpeg");
        _imagequalityMap.add(EdsImageQuality_MRM1J,  "Middle Raw + Middle1 Jpeg");
        _imagequalityMap.add(EdsImageQuality_MRM2J,  "Middle Raw + Middle2 Jpeg");
        _imagequalityMap.add(EdsImageQuality_MRSJ,   "Middle Raw + Small Jpeg");

        _imagequalityMap.add(EdsImageQuality_SR,     "Small RAW");
        _imagequalityMap.add(EdsImageQuality_SRLJF,  "Small RAW + Large Fine Jpeg");
        _imagequalityMap.add(EdsImageQuality_SRMJF,  "Small RAW + Middle Fine Jpeg");
        _imagequalityMap.add(EdsImageQuality_SRSJF,  "Small RAW + Small Fine Jpeg");
        _imagequalityMap.add(EdsImageQuality_SRLJN,  "Small RAW + Large Normal Jpeg");
        _imagequalityMap.add(EdsImageQuality_SRMJN,  "Small RAW + Middle Normal Jpeg");
        _imagequalityMap.add(EdsImageQuality_SRSJN,  "Small RAW + Small Normal Jpeg");
        _imagequalityMap.add(EdsImageQuality_SRS1JF, "Small RAW + Small1 Fine Jpeg");
        _imagequalityMap.add(EdsImageQuality_SRS1JN, "Small RAW + Small1 Normal Jpeg");
        _imagequalityMap.add(EdsImageQuality_SRS2JF, "Small RAW + Small2 Jpeg");
        _imagequalityMap.add(EdsImageQuality_SRS3JF, "Small RAW + Small3 Jpeg");

        _imagequalityMap.add(EdsImageQuality_SRLJ,   "Small RAW + Large Jpeg");
        _imagequalityMap.add(EdsImageQuality_SRM1J,  "Small RAW + Middle1 Jpeg");
        _imagequalityMap.add(EdsImageQuality_SRM2J,  "Small RAW + Middle2 Jpeg");
        _imagequalityMap.add(EdsImageQuality_SRSJ,   "Small RAW + Small Jpeg");

        _imagequalityMap.add(EdsImageQuality_LJF,    "Large Fine Jpeg");
        _imagequalityMap.add(EdsImageQuality_LJN,    "Large Normal Jpeg");
        _imagequalityMap.add(EdsImageQuality_MJF,    "Middle Fine Jpeg");
        _imagequalityMap.add(EdsImageQuality_MJN,    "Middle Normal Jpeg");
        _imagequalityMap.add(EdsImageQuality_SJF,    "Small Fine Jpeg");
        _imagequalityMap.add(EdsImageQuality_SJN,    "Small Normal Jpeg");
        _imagequalityMap.add(EdsImageQuality_S1JF,   "Small1 Fine Jpeg");
        _imagequalityMap.add(EdsImageQuality_S1JN,   "Small1 Normal Jpeg");
        _imagequalityMap.add(EdsImageQuality_S2JF,   "Small2 Jpeg");
        _imagequalityMap.add(EdsImageQuality_S3JF,   "Small3 Jpeg");

        _imagequalityMap.add(EdsImageQuality_LJ,     "Large Jpeg");
        _imagequalityMap.add(EdsImageQuality_M1J,    "Middle1 Jpeg");
        _imagequalityMap.add(EdsImageQuality_M2J,    "Middle2 Jpeg");
        _imagequalityMap.add(EdsImageQuality_SJ,     "Small Jpeg");

        _imagequalityMap.add(kEdsImageQualityForLegacy_LR,    "RAW");
        _imagequalityMap.add(kEdsImageQualityForLegacy_LRLJF, "RAW + Large Fine Jpeg");
        _imagequalityMap.add(kEdsImageQualityForLegacy_LRMJF, "RAW + Middle Fine Jpeg");
        _imagequalityMap.add(kEdsImageQualityForLegacy_LRSJF, "RAW + Small Fine Jpeg");
        _imagequalityMap.add(kEdsImageQualityForLegacy_LRLJN, "RAW + Large Normal Jpeg");
        _imagequalityMap.add(kEdsImageQualityForLegacy_LRMJN, "RAW + Middle Normal Jpeg");
        _imagequalityMap.add(kEdsImageQualityForLegacy_LRSJN, "RAW + Small Normal Jpeg");
        _imagequalityMap.add(kEdsImageQualityForLegacy_LJF,   "Large Fine Jpeg");
        _imagequalityMap.add(kEdsImageQualityForLegacy_MJF,   "Middle Fine Jpeg");
        _imagequalityMap.add(kEdsImageQualityForLegacy_SJF,   "Small Fine Jpeg");
        _imagequalityMap.add(kEdsImageQualityForLegacy_LJN,   "Large Normal Jpeg");
        _imagequalityMap.add(kEdsImageQualityForLegacy_MJN,   "Middle Normal Jpeg");
        _imagequalityMap.add(kEdsImageQualityForLegacy_SJN,   "Small Normal Jpeg");

        _imagequalityMap.add(kEdsImageQualityForLegacy_LR2,   "RAW");
        _imagequalityMap.add(kEdsImageQualityForLegacy_LR2LJ, "RAW + Large Jpeg");
        _imagequalityMap.add(kEdsImageQualityForLegacy_LR2M1J,"RAW + Middle1 Jpeg");
        _imagequalityMap.add(kEdsImageQualityForLegacy_LR2M2J,"RAW + Middle2 Jpeg");
        _imagequalityMap.add(kEdsImageQualityForLegacy_LR2SJ, "RAW + Small Jpeg");
        _imagequalityMap.add(kEdsImageQualityForLegacy_LJ,    "Large Jpeg");
        _imagequalityMap.add(kEdsImageQualityForLegacy_M1J,   "Middle1 Jpeg");
        _imagequalityMap.add(kEdsImageQualityForLegacy_M2J,   "Middle2 Jpeg");
        _imagequalityMap.add(kEdsImageQualityForLegacy_SJ,    "Small Jpeg");

        _isoMap.add(0x00,"Auto");
        _isoMap.add(0x28,"6");
        _isoMap.add(0x30,"12");
        _isoMap.add(0x38,"25");
        _isoMap.add(0x40,"50");
        _isoMap.add(0x48,"100");
        _isoMap.add(0x4b,"125");
        _isoMap.add(0x4d,"160");
        _isoMap.add(0x50,"200");
        _isoMap.add(0x53,"250");
        _isoMap.add(0x55,"320");
        _isoMap.add(0x58,"400");
        _isoMap.add(0x5b,"500");
        _isoMap.add(0x5d,"640");
        _isoMap.add(0x60,"800");
        _isoMap.add(0x63,"1000");
        _isoMap.add(0x65,"1250");
        _isoMap.add(0x68,"1600");
        _isoMap.add(0x70,"3200");
        _isoMap.add(0x78,"6400");
        _isoMap.add(0x80,"12800");
        _isoMap.add(0x88,"25600");
        _isoMap.add(0x90,"51200");
        _isoMap.add(0x98,"102400");
        _isoMap.add(0xffffffff,"unknown");

        _metermodeMap.add(1,"Spot");
        _metermodeMap.add(3,"Evaluative");
        _metermodeMap.add(4,"Partial");
        _metermodeMap.add(5,"Center-Weighted Average");
        _metermodeMap.add(0xffffffff,"unknown");
    }
 
    PropertyBag _tvMap;
    PropertyBag _avMap;
    PropertyBag _aemodeMap;
    PropertyBag _evfafmodeMap;
    PropertyBag _exposurecompMap;
    PropertyBag _imagequalityMap;
    PropertyBag _isoMap;
    PropertyBag _metermodeMap;
    
};
