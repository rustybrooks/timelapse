#pragma once

#include "EDSDK.h"
#include "EDSDKTypes.h"

EdsError EDSCALLBACK ObjectCallbackFunc(EdsObjectEvent inEvent, EdsBaseRef inRef, EdsVoid *inContext);
EdsError EDSCALLBACK PropertyCallbackFunc(EdsPropertyEvent inEvent, EdsPropertyID inPropertyID, EdsUInt32 inParam, EdsVoid *inContext);
EdsError EDSCALLBACK CameraStateCallbackFunc(EdsStateEvent inEvent, EdsUInt32 inEventData, EdsVoid *incontext);

