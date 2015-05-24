/*
 *  LibdcSequenceGrabber.h
 *  libdc_grabber
 *
 *  Created by Stephan Huber on 27.10.09.
 *  Copyright 2009 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#ifndef LIB_DC_SEQUENCE_GRABBER_HEADER
#define LIB_DC_SEQUENCE_GRABBER_HEADER


#include <cefix/SequenceGrabber.h>
#include <dc1394/dc1394.h>
#include <osg/Notify>
#include <iostream>


class Libdc1394SequenceGrabber: public cefix::SequenceGrabber {


public:
    struct ROI {
        unsigned int x,y, width, height; 
        ROI() : x(0), y(0), width(0), height(0) {}
    };

	Libdc1394SequenceGrabber(const std::string& name, unsigned int w, unsigned int h, unsigned int rate);
	
	virtual void idle();
	virtual void start();
	virtual void stop();

	virtual void showSettings() {}


	virtual float getGain()					{ return getFeature(DC1394_FEATURE_GAIN); }
	virtual float getExposure()				{ return getFeature(DC1394_FEATURE_EXPOSURE); }
	virtual float getShutter()				{ return getFeature(DC1394_FEATURE_SHUTTER); }
	virtual float getFocus()				{ return getFeature(DC1394_FEATURE_FOCUS); }
	virtual float getWhiteBalance()			{ return getFeature(DC1394_FEATURE_WHITE_BALANCE); }
	virtual float getContrast()				{ return getFeature(DC1394_FEATURE_GAMMA); }
	virtual float getBrightness()			{ return getFeature(DC1394_FEATURE_BRIGHTNESS); }
	virtual float getHue()					{ return getFeature(DC1394_FEATURE_HUE); }
	virtual float getSaturation()			{ return getFeature(DC1394_FEATURE_SATURATION); }
	
	virtual void setGain(float f)			{ setFeature(DC1394_FEATURE_GAIN, f); }
	virtual void setExposure(float f)		{ setFeature(DC1394_FEATURE_EXPOSURE, f); }
	virtual void setShutter(float f)		{ setFeature(DC1394_FEATURE_SHUTTER, f); }
	virtual void setFocus(float f)			{ setFeature(DC1394_FEATURE_FOCUS, f); }
	virtual void setWhiteBalance(float f)	{ setFeature(DC1394_FEATURE_WHITE_BALANCE, f); }
	virtual void setContrast(float f)		{ setFeature(DC1394_FEATURE_GAMMA, f); }
	virtual void setBrightness(float f)		{ setFeature(DC1394_FEATURE_BRIGHTNESS, f); }
	virtual void setHue(float f)			{ setFeature(DC1394_FEATURE_HUE, f); }
	virtual void setSaturation(float f)		{ setFeature(DC1394_FEATURE_SATURATION, f); }
	
	static void populateDeviceInfoList(cefix::SequenceGrabberDeviceInfoList& devices);
	
	static const char* getGrabberId() { return "libdc1394"; }
	
	float getFeature(dc1394feature_t feature);
	void setFeature(dc1394feature_t feature, float value);
	
protected:
	void initCamera(uint64_t uid);
	bool initVideoMode(unsigned int w, unsigned int h, bool grey, unsigned int videmode=0, unsigned int color_mode=0);
	bool initFrameRate(unsigned int rate);
	void initCapture();
	
	void setupBayer();
	void processCameraImageData( unsigned char* cameraImageData );
	
    inline void checkSuccess(dc1394error_t err, const std::string& message) {
        if (err != DC1394_SUCCESS) {
            msg(osg::WARN) << " err-code: " << err << " " << message << std::endl;
        }
    }
    
	std::ostream& msg(osg::NotifySeverity severity);
    
	
	~Libdc1394SequenceGrabber();
private:
	dc1394_t *				_context;
	dc1394camera_t*			_camera;
	dc1394video_mode_t		_videomode;
	dc1394color_coding_t	_sourceFormat;
	bool					_firstFrame;
	dc1394bayer_method_t	_bayerMethod;
	dc1394color_filter_t	_bayerPattern;
	bool					_grey;
	dc1394speed_t			_speed;
    bool                    _format7;
    ROI                     _roi;
	


};


#endif