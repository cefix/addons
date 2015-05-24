/*
 *  LibdcSequenceGrabber.cpp
 *  libdc_grabber
 *
 *  Created by Stephan Huber on 27.10.09.
 *  Copyright 2009 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#include "Libdc1394SequenceGrabber.h"
#include <cefix/StringUtils.h>
#include <dc1394/dc1394.h>
#include <osg/ImageStream>


// ----------------------------------------------------------------------------------------------------------
// 
// ----------------------------------------------------------------------------------------------------------

class Libdc1394Context : public osg::Referenced {

public:
	Libdc1394Context() : osg::Referenced() { _context = dc1394_new (); }
	~Libdc1394Context() { dc1394_free(_context); }
	
	static dc1394_t* get();

private:
	dc1394_t* _context;
	

};



// ----------------------------------------------------------------------------------------------------------
// 
// ----------------------------------------------------------------------------------------------------------

dc1394_t* Libdc1394Context::get() 
{
	static osg::ref_ptr<Libdc1394Context> s_context = new Libdc1394Context();
	return s_context->_context;
}



// ----------------------------------------------------------------------------------------------------------
// 
// ----------------------------------------------------------------------------------------------------------

Libdc1394SequenceGrabber::Libdc1394SequenceGrabber(const std::string& name, unsigned int w, unsigned int h, unsigned int rate)
:	cefix::SequenceGrabber(name, w, h, rate),
	_context(Libdc1394Context::get()),
	_camera(NULL),
	_firstFrame(true),
	_bayerMethod(DC1394_BAYER_METHOD_BILINEAR),
	_bayerPattern(DC1394_COLOR_FILTER_RGGB)

{
	msg(osg::INFO) << "LibdcSequenceGrabber " << name << std::endl;
	
	setImage(new osg::ImageStream());
	getImage()->setOrigin(osg::Image::TOP_LEFT);
	
	uint64_t uid = 0;
	
	bool grey = false;
    _format7 = false;
	int format_7_format_delta(0);
	if (name.empty() == false) {
		std::vector<std::string> parts;
		
		cefix::strTokenize(name,parts,":");
		uid=cefix::hexToLong(parts[0]);
		
		for(unsigned int i = 1; i < parts.size(); ++i) {
			std::string part = cefix::strToLower(parts[i]);
			if (part == "grey")
				grey = true;
            else if (part == "format7") {
                _format7 = true;
                if (i+1 < parts.size()) {
                    format_7_format_delta = atoi(parts[i+1].c_str());
                    ++i;
                }
            }
		}
	}
    
    
    if (_roi.height == 0) _roi.height = h;
    if (_roi.width == 0) _roi.width = w;
    
	initCamera(uid);
    
	if (_camera) {
        unsigned int video_mode(0);
        unsigned int color_mode(0);
        if (_format7)
        {
            video_mode = DC1394_VIDEO_MODE_FORMAT7_0+format_7_format_delta;
            color_mode = (!grey) ?  DC1394_COLOR_CODING_RGB8 : DC1394_COLOR_CODING_MONO8;
        }
		if (!initVideoMode(w,h, grey, video_mode, color_mode)) {
			grey  = !grey;
			msg(osg::INFO) << " could not find suitable video mode, toggling grey/color " << std::endl;
			if (!initVideoMode(w,h, grey)) {
				msg(osg::WARN) << "could not find suitable video mode for " << w << "x" << h << std::endl;
			}
		}
	
		initFrameRate(rate);
	
        dc1394error_t  err;
        if (_format7) {
             err = dc1394_format7_set_color_coding(_camera, _videomode, (dc1394color_coding_t)color_mode);
             checkSuccess(err, "dc1394_format7_set_color_coding failed");
        }
        
        if (_format7) {
            err = dc1394_format7_get_image_size(_camera, _videomode, &w, &h);
            checkSuccess(err, "dc1394_format7_get_image_size failed");
        }
        else
        {
            err = dc1394_get_image_size_from_video_mode(_camera, _videomode, &w, &h);
            checkSuccess(err, "dc1394_get_image_size_from_video_mode failed");
        }
    }
    
	if (grey) {
		getImage()->allocateImage(w,h,1,GL_LUMINANCE, GL_UNSIGNED_BYTE);
	} else { 
		getImage()->allocateImage(w,h,1,GL_RGB, GL_UNSIGNED_BYTE);
	}
	
	_grey = grey;
}

std::ostream& Libdc1394SequenceGrabber::msg(osg::NotifySeverity severity)
{
	osg::notify(severity) << "Libdc1394SequenceGrabber ";
	if (_camera) 
	{
		osg::notify(severity) << "(" << std::hex << _camera->guid << ") " << std::dec;
	}
	
	return osg::notify(severity);
}

// ----------------------------------------------------------------------------------------------------------
// 
// ----------------------------------------------------------------------------------------------------------

void Libdc1394SequenceGrabber::initCamera(uint64_t uid) 
{
	msg(osg::INFO) << "initCamera" << std::endl;

	dc1394camera_list_t * list;
	dc1394error_t err;

	err=dc1394_camera_enumerate (_context, &list);
	
	for(unsigned int i = 0; i < list->num; ++i) {
		if (!_camera && ((uid == 0) || (uid == list->ids[i].guid))) {
			_camera = dc1394_camera_new (_context, list->ids[i].guid);
			break;
		}
	}
	dc1394_camera_free_list(list);
	
	if (!_camera) {
		return;
	}
		
	if (_camera) {
		
		//dc1394_reset_bus(_camera);
		/*
		dc1394error_t err = dc1394_iso_release_bandwidth(_camera, INT_MAX);
        checkSuccess(err, "dc1394_iso_release_bandwidth failed");
		for (int channel = 0; channel < 64; ++channel) {
			err = dc1394_iso_release_channel(_camera, channel);
            //checkSuccess(err, "dc1394_iso_release_channel failed");
		}*/
	}
	

	_speed = DC1394_ISO_SPEED_400;
    
	/*
    if ((_camera->bmode_capable > 0)) {
        dc1394error_t err = dc1394_video_set_operation_mode(_camera, DC1394_OPERATION_MODE_1394B);
        checkSuccess(err,"Failed to set ISO Speed 800");
        _speed = (err!=DC1394_SUCCESS) ? DC1394_ISO_SPEED_400 : DC1394_ISO_SPEED_800;
    }
    */
}


// ----------------------------------------------------------------------------------------------------------
// 
// ----------------------------------------------------------------------------------------------------------

bool Libdc1394SequenceGrabber::initVideoMode(unsigned int w, unsigned int h, bool grey, unsigned int videomode, unsigned int color_mode)
{
	msg(osg::INFO) << "initVideoMode" << std::endl;

	std::vector<dc1394video_mode_t> suitable_modes;
	if (videomode != 0) 
	{
		suitable_modes.push_back((dc1394video_mode_t)videomode);
	}
	else 
	{
		if (w == 320) {
			if (!grey) suitable_modes.push_back(DC1394_VIDEO_MODE_320x240_YUV422);
		} else if (w == 640) {
			//if (!grey) suitable_modes.push_back(DC1394_VIDEO_MODE_640x480_RGB8);
			//if (!grey) suitable_modes.push_back(DC1394_VIDEO_MODE_640x480_YUV411);
			if (!grey) suitable_modes.push_back(DC1394_VIDEO_MODE_640x480_YUV422);
			if (grey) suitable_modes.push_back(DC1394_VIDEO_MODE_640x480_MONO8);
			//if (grey) suitable_modes.push_back(DC1394_VIDEO_MODE_640x480_MONO16);
		} else if (w == 800) {
			//if (!grey) suitable_modes.push_back(DC1394_VIDEO_MODE_800x600_RGB8);
			if (!grey) suitable_modes.push_back(DC1394_VIDEO_MODE_800x600_YUV422);
			if (grey) suitable_modes.push_back(DC1394_VIDEO_MODE_800x600_MONO8);
			//if (grey) suitable_modes.push_back(DC1394_VIDEO_MODE_800x600_MONO16);
		} else if (w == 1024) {
			//if (!grey) suitable_modes.push_back(DC1394_VIDEO_MODE_1024x768_RGB8);
			if (!grey) suitable_modes.push_back(DC1394_VIDEO_MODE_1024x768_YUV422);
			if (grey) suitable_modes.push_back(DC1394_VIDEO_MODE_1024x768_MONO8);
			//if (grey) suitable_modes.push_back(DC1394_VIDEO_MODE_1024x768_MONO16);
		} else if (w == 1280) {
			//if (!grey) suitable_modes.push_back(DC1394_VIDEO_MODE_1280x960_RGB8);
			if (!grey) suitable_modes.push_back(DC1394_VIDEO_MODE_1280x960_YUV422);
			if (grey) suitable_modes.push_back(DC1394_VIDEO_MODE_1280x960_MONO8);
			//if (grey) suitable_modes.push_back(DC1394_VIDEO_MODE_1280x960_MONO16);
		} else if (w == 1600) {
			//if (!grey) suitable_modes.push_back(DC1394_VIDEO_MODE_1600x1200_RGB8);
			if (!grey) suitable_modes.push_back(DC1394_VIDEO_MODE_1600x1200_YUV422);
			if (grey) suitable_modes.push_back(DC1394_VIDEO_MODE_1600x1200_MONO8);
			//if (grey) suitable_modes.push_back(DC1394_VIDEO_MODE_1600x1200_MONO16);
		}
	
	}
	dc1394video_modes_t video_modes;
	
    dc1394error_t err=dc1394_video_get_supported_modes(_camera,&video_modes);
    checkSuccess(err, "dc1394_video_get_supported_modes failed");
    
	for (unsigned int i = 0;i < video_modes.num;i++) 
	{
		for(unsigned int j=0; j < suitable_modes.size(); ++j) {
			if (video_modes.modes[i] == suitable_modes[j])
			{
				// videmodus gefunden, gleich setzen
				_videomode = video_modes.modes[i];
				err = dc1394_video_set_mode(_camera, video_modes.modes[i]);
				checkSuccess(err,"dc1394_video_set_mode failed");
                
                if (color_mode == 0) {
                    err = dc1394_get_color_coding_from_video_mode(_camera, _videomode, &_sourceFormat);
                    checkSuccess(err, "dc1394_get_color_coding_from_video_mode failed");
                } else {
                    _sourceFormat = (dc1394color_coding_t)(color_mode);
                }
                return true;
			}
		}
	}
	
	return false;
}
				



// ----------------------------------------------------------------------------------------------------------
// 
// ----------------------------------------------------------------------------------------------------------

bool Libdc1394SequenceGrabber::initFrameRate(unsigned int rate) 
{
	msg(osg::INFO) << "initFrameRate" << std::endl;
	
	if (!_camera) return false;
    
    if (_format7) {
        uint32_t bit_size;
        dc1394_get_color_coding_bit_size(_sourceFormat,&bit_size);
        int packet_size = DC1394_USE_MAX_AVAIL;
        
        if(rate != 0) {
            double bus_period;
			if(_speed == DC1394_ISO_SPEED_800) {
				bus_period = 0.0000625;
			}
			else {
				bus_period = 0.000125;
			}

            int num_packets = (int)(1.0/(bus_period*rate)+0.5);
            packet_size = ((_roi.width - _roi.x)*(_roi.height - _roi.y)*bit_size + (num_packets*8) - 1) / (num_packets*8);
        }
        
        dc1394error_t err = dc1394_format7_set_packet_size(_camera, _videomode, packet_size);
        checkSuccess(err, "dc1394_format7_set_packet_size failed");
        
        err = dc1394_format7_set_roi(_camera, _videomode, _sourceFormat, packet_size, _roi.x,_roi.y,_roi.width,_roi.height);
        checkSuccess(err, "dc1394_format7_set_roi failed");
        return (err == DC1394_SUCCESS);
    }
	
	dc1394framerates_t framerates;
	dc1394error_t err=dc1394_video_get_supported_framerates(_camera,_videomode, &framerates);
    checkSuccess(err, "dc1394_video_get_supported_framerates failed");
    
	dc1394framerate_t framerate=framerates.framerates[framerates.num-1];
	
	switch (rate) {
		case 15:
			framerate = DC1394_FRAMERATE_15;
			break;
		case 30:
			framerate = DC1394_FRAMERATE_30;
			break;
		case 60:
			framerate = DC1394_FRAMERATE_60;
			break;
		case 120:
			framerate = DC1394_FRAMERATE_120;
			break;
		case 240:
			framerate = DC1394_FRAMERATE_240;
	}
	
	err=dc1394_video_set_framerate(_camera, framerate);
    checkSuccess(err, "dc1394_video_set_framerate failed");
    
	return (err == DC1394_SUCCESS);
}



// ----------------------------------------------------------------------------------------------------------
// 
// ----------------------------------------------------------------------------------------------------------

void Libdc1394SequenceGrabber::initCapture()
{
	msg(osg::INFO) << "initCapture" << std::endl;

	if (!_camera) return;
	
	dc1394error_t err;
	err=dc1394_video_set_iso_speed(_camera, _speed);
    checkSuccess(err, "dc1394_video_set_iso_speed failed");
	
    err=dc1394_capture_setup(_camera, 4, DC1394_CAPTURE_FLAGS_DEFAULT);
    checkSuccess(err, "dc1394_capture_setup failed");
		
}



// ----------------------------------------------------------------------------------------------------------
// 
// ----------------------------------------------------------------------------------------------------------

Libdc1394SequenceGrabber::~Libdc1394SequenceGrabber() 
{
	msg(osg::INFO) << "~LibdcSequenceGrabber" << std::endl;

	if (_camera) dc1394_camera_free(_camera);

}


	
// ----------------------------------------------------------------------------------------------------------
// 
// ----------------------------------------------------------------------------------------------------------

void Libdc1394SequenceGrabber::idle()
{
	dc1394error_t err;
	dc1394video_frame_t *last_frame(NULL), *frame(NULL);
	if (!_camera) {
		fakeTracking();
		return;
	}
    do{     
        err = dc1394_capture_dequeue(_camera, DC1394_CAPTURE_POLICY_POLL, &frame);
        if (frame) { 
            if (last_frame)
                err=dc1394_capture_enqueue(_camera, last_frame);
            last_frame = frame; 
        }
    } while (frame);
    
	checkSuccess(err, "dc1394_capture_dequeue failed");
	
    if (_firstFrame) {
		setupBayer();
		_firstFrame = false;
	}
	
	if (last_frame) {
		processCameraImageData( last_frame->image );
		newFrameAvailable();
		
		err=dc1394_capture_enqueue(_camera, last_frame);
        checkSuccess(err, "dc1394_capture_enqueue failed");
	}
}


// ----------------------------------------------------------------------------------------------------------
// 
// ----------------------------------------------------------------------------------------------------------

void Libdc1394SequenceGrabber::start()
{
	msg(osg::INFO) << "start" << std::endl;

	dc1394error_t err;
	initCapture();
	if (!_camera) return;
	err=dc1394_video_set_transmission(_camera, DC1394_ON);
    checkSuccess(err, "dc1394_video_set_transmission failed");
	
	setRunning(true);
}


// ----------------------------------------------------------------------------------------------------------
// 
// ----------------------------------------------------------------------------------------------------------

void Libdc1394SequenceGrabber::stop()
{
	msg(osg::INFO) << "stop" << std::endl;

	setRunning(false);
	dc1394error_t err;
	err=dc1394_video_set_transmission(_camera, DC1394_OFF);
	checkSuccess(err, "dc1394_video_set_transmission failed");
	
	dc1394_capture_stop(_camera);
}


// ----------------------------------------------------------------------------------------------------------
// 
// ----------------------------------------------------------------------------------------------------------

static dc1394error_t getBayerTile( dc1394camera_t* camera, dc1394color_filter_t* bayerPattern )
{

    uint32_t value;
    dc1394error_t err;

    // query register 0x1040
    // This register is an advanced PGR register called BAYER_TILE_MAPPING
    // For more information check the PGR IEEE-1394 Digital Camera Register Reference
    // err = GetCameraControlRegister( camera, 0x1040, &value );
    err = dc1394_get_register( camera, 0x1040, &value );

    if ( err != DC1394_SUCCESS )
    {
        return err;
    }

    // Ascii R = 52 G = 47 B = 42 Y = 59
    switch( value )
    {
        case 0x52474742:	// RGGB
            *bayerPattern = DC1394_COLOR_FILTER_RGGB;
            break;
        case 0x47425247:	// GBRG
            *bayerPattern = DC1394_COLOR_FILTER_GBRG;
            break;
        case 0x47524247:	// GRBG
            *bayerPattern = DC1394_COLOR_FILTER_GRBG;
            break;
        case 0x42474752:	// BGGR
            *bayerPattern = DC1394_COLOR_FILTER_BGGR;
            break;
        case 0x59595959:	// YYYY
        default:
            break;
    }

    return err;
}


// ----------------------------------------------------------------------------------------------------------
// 
// ----------------------------------------------------------------------------------------------------------

void Libdc1394SequenceGrabber::setupBayer() 
{
	msg(osg::INFO) << "setupBayer" << std::endl;
	
	if( _sourceFormat == DC1394_COLOR_CODING_RAW8 ||
		_sourceFormat == DC1394_COLOR_CODING_MONO8 || 
		_sourceFormat == DC1394_COLOR_CODING_MONO16 || 
		_sourceFormat == DC1394_COLOR_CODING_RAW16 )
	{
		getBayerTile( _camera, &_bayerPattern );
	}
}


// ----------------------------------------------------------------------------------------------------------
// 
// ----------------------------------------------------------------------------------------------------------

void Libdc1394SequenceGrabber::processCameraImageData( unsigned char* cameraImageData )
{
	unsigned char* dest = getImage()->data();
	unsigned int w = getImage()->s();
	unsigned int h = getImage()->t();
	unsigned int bpp = getImage()->getPixelSizeInBits() / 8;
	unsigned int size = w * h * bpp;
	
	if( _sourceFormat == DC1394_COLOR_CODING_RAW8 || _sourceFormat == DC1394_COLOR_CODING_MONO8 ) 
	{
		if (_grey)
			memcpy(dest, cameraImageData, size);
		else 
			dc1394_bayer_decoding_8bit( cameraImageData, dest, w, h,  _bayerPattern, _bayerMethod );
	}
	else if(_sourceFormat == DC1394_COLOR_CODING_MONO16 || _sourceFormat == DC1394_COLOR_CODING_RAW16 )
	{
	    // These are not implemented yet....
		if(!_grey)
		{
		    dc1394_convert_to_RGB8(cameraImageData, dest, w, h, 0, _sourceFormat, 16);
		}
		else {
			dc1394_convert_to_MONO8(cameraImageData, dest, w, h, 0, _sourceFormat, 16);
		}
	}
	else if(_sourceFormat == DC1394_COLOR_CODING_YUV411 || 
		_sourceFormat == DC1394_COLOR_CODING_YUV422 ||
		_sourceFormat == DC1394_COLOR_CODING_YUV444 )
	{
		if(!_grey )
		{
		    dc1394_convert_to_RGB8( cameraImageData, dest, w, h, DC1394_BYTE_ORDER_UYVY, _sourceFormat, 16);
		}
		else
		{
			msg(osg::WARN) << "could not handle sourceformat " << _sourceFormat << std::endl; 
		}

	}
	else if(  _sourceFormat == DC1394_COLOR_CODING_RGB8 )
	{
		if( !_grey )
		{
            memcpy(dest, cameraImageData, size);
		}
		else
		{
            msg(osg::WARN) << "could not handle sourceformat " << _sourceFormat << std::endl;
		}
	}
	else
	{
		msg(osg::WARN) << "could not handle sourceformat " << _sourceFormat << std::endl;
	}
}


static const char* getVideoMode(dc1394video_mode_t mode)
{
	switch(mode) {
			case DC1394_VIDEO_MODE_160x120_YUV444:		return  "160 x 120 YUV444"; break;
			case DC1394_VIDEO_MODE_320x240_YUV422:		return  "320 x 240 YUV422"; break;
			case DC1394_VIDEO_MODE_640x480_YUV411:		return  "640 x 480 YUV411"; break;
			case DC1394_VIDEO_MODE_640x480_YUV422:		return  "640 x 480 YUV422"; break;
			case DC1394_VIDEO_MODE_640x480_RGB8:		return  "640 x 480 RGB8"; break;
			case DC1394_VIDEO_MODE_640x480_MONO8:		return  "640 x 480 MONO8"; break;
			case DC1394_VIDEO_MODE_640x480_MONO16:		return  "640 x 480 MONO16"; break;
			case DC1394_VIDEO_MODE_800x600_YUV422:		return  "800 x 600 YUV422"; break;
			case DC1394_VIDEO_MODE_800x600_RGB8:		return  "800 x 600 RGB8"; break;
			case DC1394_VIDEO_MODE_800x600_MONO8:		return  "800 x 600 MONO8"; break;
			case DC1394_VIDEO_MODE_1024x768_YUV422:		return  "1024 x 768 YUV422"; break;
			case DC1394_VIDEO_MODE_1024x768_RGB8:		return  "1024 x 768 RGB8"; break;
			case DC1394_VIDEO_MODE_1024x768_MONO8:		return  "1024 x 768 MONO8"; break;
			case DC1394_VIDEO_MODE_800x600_MONO16:		return  "800 x 600 MONO16"; break;
			case DC1394_VIDEO_MODE_1024x768_MONO16:		return  "1024 x 768 MONO16"; break;
			case DC1394_VIDEO_MODE_1280x960_YUV422:		return  "1280 x 960 YUV422"; break;
			case DC1394_VIDEO_MODE_1280x960_RGB8:		return  "1280 x 960 RGB8"; break;
			case DC1394_VIDEO_MODE_1280x960_MONO8:		return  "1280 x 960 MONO8"; break;
			case DC1394_VIDEO_MODE_1600x1200_YUV422:	return  "1600 x 1200 YUV22"; break;
			case DC1394_VIDEO_MODE_1600x1200_RGB8:		return  "1600 x 1200 RGB8"; break;
			case DC1394_VIDEO_MODE_1600x1200_MONO8:		return  "1600 x 1200 MONO8"; break;
			case DC1394_VIDEO_MODE_1280x960_MONO16:		return  "1280 x 960 MONO16"; break;
			case DC1394_VIDEO_MODE_1600x1200_MONO16:	return  "1600 x 1200 MONO16"; break;
			case DC1394_VIDEO_MODE_EXIF:				return  "EXIF"; break;
			case DC1394_VIDEO_MODE_FORMAT7_0:			return  "FORMAT 7 / 0"; break;
			case DC1394_VIDEO_MODE_FORMAT7_1:			return  "FORMAT 7 / 1"; break;
			case DC1394_VIDEO_MODE_FORMAT7_2:			return  "FORMAT 7 / 2"; break;
			case DC1394_VIDEO_MODE_FORMAT7_3:			return  "FORMAT 7 / 3"; break;
			case DC1394_VIDEO_MODE_FORMAT7_4:			return  "FORMAT 7 / 4"; break;
			case DC1394_VIDEO_MODE_FORMAT7_5:			return  "FORMAT 7 / 5"; break;
			case DC1394_VIDEO_MODE_FORMAT7_6:			return  "FORMAT 7 / 6"; break;
			case DC1394_VIDEO_MODE_FORMAT7_7:			return  "FORMAT 7 / 7"; break;
		}

	return "UNKNOWN VIDEOMODE";
}

static const char* getNameOfFeature(dc1394feature_t i) 
{
	switch(i) {
		case DC1394_FEATURE_BRIGHTNESS:
			return "DC1394_FEATURE_BRIGHTNESS";
			break;
		case DC1394_FEATURE_EXPOSURE:
			return "DC1394_FEATURE_EXPOSURE";
			break;
		case DC1394_FEATURE_SHARPNESS:
			return "DC1394_FEATURE_SHARPNESS";
			break;
		case DC1394_FEATURE_WHITE_BALANCE:
			return "DC1394_FEATURE_WHITE_BALANCE";
			break;
		case DC1394_FEATURE_HUE:
			return "DC1394_FEATURE_HUE";
			break;
		case DC1394_FEATURE_SATURATION:
			return "DC1394_FEATURE_SATURATION";
			break;
		case DC1394_FEATURE_GAMMA:
			return "DC1394_FEATURE_GAMMA";
			break;
		case DC1394_FEATURE_SHUTTER:
			return "DC1394_FEATURE_SHUTTER";
			break;
		case DC1394_FEATURE_GAIN:
			return "DC1394_FEATURE_GAIN";
			break;
		case DC1394_FEATURE_IRIS:
			return "DC1394_FEATURE_IRIS";
			break;
		case DC1394_FEATURE_FOCUS:
			return "DC1394_FEATURE_FOCUS";
			break;
		case DC1394_FEATURE_TEMPERATURE:
			return "DC1394_FEATURE_TEMPERATURE";
			break;
		case DC1394_FEATURE_TRIGGER:
			return "DC1394_FEATURE_TRIGGER";
			break;
		case DC1394_FEATURE_TRIGGER_DELAY:
			return "DC1394_FEATURE_TRIGGER_DELAY";
			break;
		case DC1394_FEATURE_WHITE_SHADING:
			return "DC1394_FEATURE_WHITE_SHADING";
			break;
		case DC1394_FEATURE_FRAME_RATE:
			return "DC1394_FEATURE_FRAME_RATE";
			break;
		case DC1394_FEATURE_ZOOM:
			return "DC1394_FEATURE_ZOOM";
			break;
		case DC1394_FEATURE_PAN:
			return "DC1394_FEATURE_PAN";
			break;
		case DC1394_FEATURE_TILT:
			return "DC1394_FEATURE_TILT";
			break;
		case DC1394_FEATURE_OPTICAL_FILTER:
			return "DC1394_FEATURE_OPTICAL_FILTER";
			break;
		case DC1394_FEATURE_CAPTURE_SIZE:
			return "DC1394_FEATURE_CAPTURE_SIZE";
			break;
		case DC1394_FEATURE_CAPTURE_QUALITY:
			return "DC1394_FEATURE_CAPTURE_QUALITY";
			break;
	}
	
	return "UNKNOWN";
}

const char* getFrameRate(dc1394framerate_t rate) 
{
	switch (rate) {
			case DC1394_FRAMERATE_1_875:	return "1.875 fps "; break;
			case DC1394_FRAMERATE_3_75:		return "3.75 fps "; break;
			case DC1394_FRAMERATE_7_5:		return "7.5 fps "; break;
			case DC1394_FRAMERATE_15:		return "15 fps "; break;
			case DC1394_FRAMERATE_30:		return "30 fps "; break;
			case DC1394_FRAMERATE_60:		return "60 fps "; break;
			case DC1394_FRAMERATE_120:		return "120 fps "; break;
			case DC1394_FRAMERATE_240:		return "240 fps "; break;
	}
	return "UNKNOWN FRAMERATE";
}


float Libdc1394SequenceGrabber::getFeature(dc1394feature_t feature)
{
	if (!_camera) return -1;
	
	
	uint32_t min, max, value;
	dc1394feature_mode_t current_mode;
	dc1394bool_t is_present;
	
	dc1394_feature_is_present(_camera, feature, &is_present);
	if (is_present == DC1394_FALSE) 
		return -1.0; 
	
	dc1394_feature_get_mode(_camera, feature, &current_mode);
	
	if (current_mode == DC1394_FEATURE_MODE_AUTO) 
		return -1.0;
		
	dc1394_feature_get_boundaries(_camera, feature, &min, &max);
	dc1394_feature_get_value(_camera, feature, &value);
	
	return 1000.0 * (value-min) / static_cast<float>(max-min); 
}



void Libdc1394SequenceGrabber::setFeature(dc1394feature_t feature, float value)
{
	if (!_camera) return;
	
	dc1394bool_t is_present;
	dc1394error_t err;
	dc1394_feature_is_present(_camera, feature, &is_present);
	
	if (is_present == DC1394_FALSE) 
		return;
	
	dc1394feature_mode_t current_mode = (value == -1.0) ? DC1394_FEATURE_MODE_AUTO : DC1394_FEATURE_MODE_MANUAL;
	err = dc1394_feature_set_mode(_camera, feature, current_mode);
	checkSuccess(err, "dc1394_feature_set_mode failed");
	
	if (value == -1.0)
		return;
		
	uint32_t min, max, int_value;
	dc1394_feature_get_boundaries(_camera, feature, &min, &max);
	
	int_value = min + value / 1000.0 * (max-min);
	// std::cout << feature << " value: " << int_value << " min: " << min << " max: " << max << std::endl;
	err = dc1394_feature_set_value(_camera, feature, int_value);
	checkSuccess(err, "dc1394_feature_set_value failed");

}



void Libdc1394SequenceGrabber::populateDeviceInfoList(cefix::SequenceGrabberDeviceInfoList& devices)
{
	dc1394_t * d;
	dc1394camera_list_t * list;
	dc1394error_t err;
	dc1394video_modes_t video_modes;
	dc1394framerates_t framerates;
	
	d = Libdc1394Context::get();
	if (!d)
		return;
	
	err=dc1394_camera_enumerate (d, &list);
	
	for(unsigned int i = 0; i < list->num; ++i) {
		osg::notify(osg::INFO) << "UID: " <<  list->ids[i].guid << std::endl;
		dc1394camera_t* camera = dc1394_camera_new (d, list->ids[i].guid);
		if (!camera) continue;
		
		devices.push_back(cefix::SequenceGrabber::DeviceInfo(getGrabberId(), cefix::longToHexString(list->ids[i].guid)));
		
		dc1394_camera_print_info(camera, stdout);
		
		err=dc1394_video_get_supported_modes(camera,&video_modes);
		osg::notify(osg::INFO) << "available video-modes: " << std::endl;

		for (int j = 0;j < video_modes.num;j++) 
		{
			osg::notify(osg::INFO) << "* " << getVideoMode(video_modes.modes[j]);
			
			if (dc1394_video_get_supported_framerates(camera, video_modes.modes[j], &framerates) == DC1394_SUCCESS) {
				osg::notify(osg::INFO) << " fps: ";
				for(unsigned int k = 0; k < framerates.num; ++k) {
					osg::notify(osg::INFO) << " " << getFrameRate(framerates.framerates[k]);
				}
			}
			osg::notify(osg::INFO) << std::endl;
		}
		
	
		dc1394featureset_t features;
		dc1394_feature_get_all(camera, &features);
		dc1394_feature_print_all(&features, stdout);
		
		/*
		for( int j = 0; j < DC1394_FEATURE_NUM; j++ )
		{
			const dc1394feature_info_t& f = features.feature[j];
			
			if( f.available ) 
			{ 
				std::cout << getNameOfFeature(f.id) << std::endl;
				std::cout << "  current mode: ";
				switch(f.current_mode) {
					case DC1394_FEATURE_MODE_MANUAL:
						std::cout << "manual";
						break;
					case DC1394_FEATURE_MODE_AUTO:
						std::cout << "auto";
						break;
					case DC1394_FEATURE_MODE_ONE_PUSH_AUTO:
						std::cout << "one-push";
						break;
				}
				std::cout << std::endl;
				std::cout << "  min: " << f.min << " max: " << f.max << " value: " << f.value << std::endl;
				if (f.absolute_capable==DC1394_TRUE) {
					std::cout << "  abs:min: " << f.abs_min << " max: " << f.abs_max << " value: " << f.abs_value << std::endl;
				}
			}
		}
		*/
		
		dc1394_camera_free(camera);
	}
	
	dc1394_camera_free_list(list);
}



