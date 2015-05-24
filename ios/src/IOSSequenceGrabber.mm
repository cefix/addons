/*
 *  IOSSequenceGrabber.cpp
 *  DiePhysiker
 *
 *  Created by Stephan Huber on 14.10.10.
 *  Copyright 2010 Digital Mind. All rights reserved.
 *
 */

#include "IOSSequenceGrabber.h"
#include <cefix/Log.h>

#include "IOSUtils.h"
#import <AVFoundation/AVFoundation.h>


@interface CaptureDelegate : NSObject<AVCaptureVideoDataOutputSampleBufferDelegate>
{
	cefix::IOSSequenceGrabber* _grabber;
	
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput 
         didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer 
         fromConnection:(AVCaptureConnection *)connection;
- (void)setGrabber: (cefix::IOSSequenceGrabber*) grabber;
@end


@implementation CaptureDelegate

- (void)setGrabber: (cefix::IOSSequenceGrabber*) grabber {
	_grabber = grabber; 
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput 
         didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer 
         fromConnection:(AVCaptureConnection *)connection
{ 
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    // Lock the base address of the pixel buffer
    CVPixelBufferLockBaseAddress(imageBuffer,0);

    // Get the number of bytes per row for the pixel buffer
    // size_t bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer); 
    // Get the pixel buffer width and height
    size_t width = CVPixelBufferGetWidth(imageBuffer); 
    size_t height = CVPixelBufferGetHeight(imageBuffer); 

    // Get the base address of the pixel buffer
    void *baseAddress = CVPixelBufferGetBaseAddress(imageBuffer);
    // Get the data size for contiguous planes of the pixel buffer.
    size_t bufferSize = CVPixelBufferGetDataSize(imageBuffer); 

	osg::Image* img = _grabber->getImage();
	if ((width != img->s()) || (height != img->t())) {
		cefix::log::error("IOSSequenceGrabber") << "resizing image to " << width << " x " << height << std::endl;
		img->allocateImage(width, height, 1, GL_BGRA, GL_UNSIGNED_BYTE);
	}
	
	memcpy(img->data(), baseAddress, bufferSize);
    CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
	_grabber->newFrameAvailable();
}

@end

namespace cefix {


struct IOSSequenceGrabber::Data {

AVCaptureSession* session;
CaptureDelegate* delegate;

};

void IOSSequenceGrabber::populateDeviceInfoList(SequenceGrabberDeviceInfoList &deviceList) 
{
    NSArray* devices = [AVCaptureDevice devices];
    for(unsigned int i = 0; i < [devices count]; ++i) {
        AVCaptureDevice* device = [devices objectAtIndex:i];
        if ([device hasMediaType: AVMediaTypeVideo])
        {
            deviceList.push_back(cefix::SequenceGrabber::DeviceInfo(getGrabberId(), IOSUtils::toString([device uniqueID]), IOSUtils::toString([device localizedName])));
        }
    }
}

IOSSequenceGrabber::IOSSequenceGrabber(const std::string &name, unsigned int w, unsigned int h, unsigned int rate)
:	cefix::SequenceGrabber(name, w, h, rate),
    _data(NULL),
    _valid(false)
{
	_data = new Data;
	init(name, w,h,rate);
}
	
    
    
void IOSSequenceGrabber::init(const std::string& params, unsigned int w, unsigned int h, unsigned int rate) {
	NSError *error = nil;

    // Create the session
    _data->session = [[AVCaptureSession alloc] init];

    // Configure the session to produce lower resolution video frames, if your 
    // processing algorithm can cope. We'll specify medium quality for the
    // chosen device.
    _data->session.sessionPreset = AVCaptureSessionPresetMedium;

    // Find a suitable AVCaptureDevice
    
    AVCaptureDevice *device = NULL;
    if ((params == "front") || (params == "back")) 
    {
        AVCaptureDevicePosition needed_position = (params == "front") ? AVCaptureDevicePositionFront : AVCaptureDevicePositionBack;
        NSArray* devices = [AVCaptureDevice devices];
        for(unsigned int i = 0; i < [devices count]; i++) 
        {
            AVCaptureDevice* temp = [devices objectAtIndex: i];
            if (([temp hasMediaType: AVMediaTypeVideo]) && ([temp position] == needed_position)) {
                device = temp;
                break;
            }
        }
    
    } else if (!params.empty()) {
        device = [AVCaptureDevice deviceWithUniqueID: cefix::IOSUtils::toNSString(params)];
    }
    if (device == NULL) 
    {
        if (!params.empty()) 
        {
            cefix::log::error("IOSSequenceGrabber") << "could not find device: " << params << std::endl;
        }
        device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    }
    if (device) {
        // Create a device input with the device and add it to the session.
        AVCaptureDeviceInput *input = [AVCaptureDeviceInput deviceInputWithDevice:device 
                                                                        error:&error];
        _valid = (input != NULL);
        [_data->session addInput:input];

        // Create a VideoDataOutput and add it to the session
        AVCaptureVideoDataOutput *output = [[[AVCaptureVideoDataOutput alloc] init] autorelease];
        [_data->session addOutput:output];
        
        _data->delegate = [[CaptureDelegate alloc] init];
        [_data->delegate setGrabber: this];

        // Configure your output.
        dispatch_queue_t queue = dispatch_queue_create("myQueue", NULL);
        [output setSampleBufferDelegate: _data->delegate queue:queue];
        dispatch_release(queue);

        // Specify the pixel format
        output.videoSettings = 
                    [NSDictionary dictionaryWithObject:
                        [NSNumber numberWithInt:kCVPixelFormatType_32BGRA] 
                        forKey:(id)kCVPixelBufferPixelFormatTypeKey];


        // If you wish to cap the frame rate to a known value, such as 15 fps, set 
        // minFrameDuration.
        output.minFrameDuration = CMTimeMake(1, rate);
    }
	setImage(new osg::Image());
	getImage()->setInternalTextureFormat(GL_RGBA);
	getImage()->allocateImage(w, h, 1, GL_BGRA, GL_UNSIGNED_BYTE);
    getImage()->setOrigin(osg::Image::TOP_LEFT);
    
}



void IOSSequenceGrabber::start()
{
	// Start the session running to start the flow of data
    if (_valid)
        [_data->session startRunning];

}


void IOSSequenceGrabber::stop ()
{
	if (_valid)
        [_data->session stopRunning];
}


void IOSSequenceGrabber::showSettings ()
{

}



IOSSequenceGrabber::~IOSSequenceGrabber()
{
	stop();
    
    [_data->delegate setGrabber: NULL];
    [_data->delegate release];
    [_data->session release];
    delete _data;
}


}