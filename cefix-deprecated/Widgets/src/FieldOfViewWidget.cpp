/*
 *  FieldOfViewWidget.cpp
 *  theseuslogo
 *
 *  Created by Stephan Huber on 21.11.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "FieldOfViewWidget.h"

#include <cefix/PropertyList.h>
#include <cefix/ApplicationWindow.h>
#include <cefix/WidgetFactory.h>
#include <cefix/HorizontalSliderWidget.h>
#include <cefix/CheckboxWidget.h>
#include <cefix/TextWidget.h>
#include <cefix/DataFactory.h>
#include <cefix/WidgetDefaults.h>

namespace cefix {


// ----------------------------------------------------------------------------------------------------------
// ctor
// ----------------------------------------------------------------------------------------------------------

FieldOfViewWidget::FieldOfViewWidget(const std::string& identifier, const osg::Vec3& pos, const osg::Vec2& size, bool vertical) : 
	TransformWidget(identifier, osg::Matrix::translate(pos)),
	_nearPlane(0.1),
	_farPlane(1000),
	_sliderWidth(WidgetDefaults::getSliderHeight()),
	_thumbSize(WidgetDefaults::getSliderThrobberSize()),
	_captionFont("system.xml"),
	_captionFontSize(0.0f),
	_captionFontColor(osg::Vec4(0.5,0.5,0.5,1)),
	_textYOffset(2)
	
{
	computeOffsets(size, vertical);
	createWidgets(size, vertical);
}


// ----------------------------------------------------------------------------------------------------------
// ctor
// ----------------------------------------------------------------------------------------------------------

FieldOfViewWidget::FieldOfViewWidget(cefix::PropertyList* pl) :
	TransformWidget(pl->get("id")->asString(), osg::Matrix::translate(pl->get("position")->asVec3())),
	_nearPlane(0.1),
	_farPlane(1000),
	_sliderWidth(WidgetDefaults::getSliderHeight()),
	_thumbSize(WidgetDefaults::getSliderThrobberSize()),
	_captionFont("system.xml"),
	_captionFontSize(0.0f),
	_captionFontColor(osg::Vec4(0.5,0.5,0.5,1)),
	_textYOffset(2)

{
	osg::Vec2 size = pl->get("size")->asVec2();
	bool vertical = true;
	if (pl->hasKey("vertical")) 
		vertical = pl->get("vertical")->asInt() != 0;

	computeOffsets(size, vertical);
	
	if (pl->hasKey("sliderHeight")) 
		_sliderWidth = pl->get("sliderHeight")->asFloat();
	if (pl->hasKey("throbberWidth")) 
		_thumbSize = pl->get("throbberSize")->asVec2();
	if (pl->hasKey("captionFont")) 
		_captionFont = pl->get("captionFont")->asString();
	if (pl->hasKey("captionFontSize")) 
		_captionFontSize = pl->get("captionFontSize")->asFloat();
	if (pl->hasKey("captionFontColor")) 
		_captionFontColor = pl->get("captionFontColor")->asVec4();
	if (pl->hasKey("captionYOffset")) 
		_textYOffset = pl->get("captionYOffset")->asFloat();

	createWidgets(size, vertical, pl->get("prefsKey")->asString());
		
}

// ----------------------------------------------------------------------------------------------------------
// computeOffsets
// ----------------------------------------------------------------------------------------------------------

void FieldOfViewWidget::computeOffsets(const osg::Vec2& size, bool vertical) {

	if (!vertical) {
		// horizontal layout
		_hSliderOffset		= osg::Vec3(20 + size[0] * 1/3.0, size[1] / 2,0);
		_vSliderOffset		= osg::Vec3(20 + size[0] * 1/3.0, 0, 0);
		_checkboxOffset		= osg::Vec3(00 + size[0] * 2/3.0, 0, 0); 
		_nearSliderOffset	= osg::Vec3(20, size[1] / 2, 0); 
		_farSliderOffset	= osg::Vec3(20,	0, 0); 

	} else {
		_hSliderOffset		= osg::Vec3(20,size[1] * 4/5.0,0);
		_vSliderOffset		= osg::Vec3(20,size[1] * 3/5.0, 0);
		_checkboxOffset		= osg::Vec3(0, size[1] * 2/5.0, 0); 
		_nearSliderOffset	= osg::Vec3(20, size[1] * 1/5.0, 0); 
		_farSliderOffset	= osg::Vec3(20, 0        , 0); 
		
	}

}


// ----------------------------------------------------------------------------------------------------------
// createWidget
// ----------------------------------------------------------------------------------------------------------

void FieldOfViewWidget::createWidgets(const osg::Vec2& size, bool vertical, const std::string& widgetprefskey) {

	cefix::WidgetFactory* wf = cefix::WidgetFactory::instance();
	float w;
	if (!vertical) {
		w = size[0]  / 3.0 - 70;
	} else
		w = size[0] - 60;
	
	osg::ref_ptr<PropertyList> pl;
	
	pl = new cefix::PropertyList();
	pl->add("id", "hfov_slider");
	pl->add("position", _hSliderOffset);
	pl->add("size", osg::Vec2(w, _sliderWidth));
	pl->add("throbbersize", _thumbSize);
	pl->add("min", 10.0f);
	pl->add("max", 180.0f);
	pl->add("value", 60.0f);
	if (!widgetprefskey.empty()) pl->add("prefskey", widgetprefskey+"/slider");
	
	osg::ref_ptr<HorizontalSliderWidget> hslider = wf->createWidget<HorizontalSliderWidget>("HorizontalSliderWidget",pl.get());
	
	pl = new cefix::PropertyList();
	pl->add("id", "vfov_slider");
	pl->add("position", _vSliderOffset);
	pl->add("size", osg::Vec2(w, _sliderWidth));
	pl->add("throbbersize", _thumbSize);
	pl->add("min", 10.0f);
	pl->add("max", 180.0f);
	pl->add("value", 60.0f);
	if (!widgetprefskey.empty()) pl->add("prefskey", widgetprefskey+"/slider");
	
	osg::ref_ptr<HorizontalSliderWidget> vslider = wf->createWidget<HorizontalSliderWidget>(std::string("HorizontalSliderWidget"),pl.get());
	
	pl = new cefix::PropertyList();
	pl->add("id", "proportional_checkbox");
	pl->add("position", _checkboxOffset);
	pl->add("size", osg::Vec2(_sliderWidth, _sliderWidth));
	pl->add("selected", 1);
	pl->add("caption", "proportional");
	if (!widgetprefskey.empty()) pl->add("prefskey", widgetprefskey+"/checkbox");
	
	osg::ref_ptr<CheckboxWidget> checkbox = wf->createWidget<CheckboxWidget>("CheckBoxWidget", pl.get());

	pl = new cefix::PropertyList();
	pl->add("id", "caption_hfov");
	pl->add("position", _hSliderOffset+osg::Vec3(-20,_textYOffset,0));
	pl->add("caption", "H:");
	pl->add("Font", _captionFont);
	pl->add("FontSize", _captionFontSize);
	pl->add("Color", _captionFontColor);
	if (!widgetprefskey.empty()) pl->add("prefskey", widgetprefskey+"/caption");
	
	osg::ref_ptr<TextWidget> hslider_caption = wf->createWidget<TextWidget>("TextWidget", pl.get());

	pl = new cefix::PropertyList();
	pl->add("id", "caption_vfov");
	pl->add("position", _vSliderOffset + osg::Vec3(-20,_textYOffset,0));
	pl->add("caption", "V:");
	pl->add("Font", _captionFont);
	pl->add("FontSize", _captionFontSize);
	pl->add("Color", _captionFontColor);
	if (!widgetprefskey.empty()) pl->add("prefskey", widgetprefskey+"/caption");
	
	osg::ref_ptr<TextWidget> vslider_caption = wf->createWidget<TextWidget>("TextWidget", pl.get());
	
	pl = new cefix::PropertyList();
	pl->add("id", "value_hfov");
	pl->add("position", _hSliderOffset+osg::Vec3(w + 40,_textYOffset,0));
	pl->add("caption", "H:");
	pl->add("Font", _captionFont);
	pl->add("FontSize", _captionFontSize);
	pl->add("Color", _captionFontColor);
	pl->add("alignment", "right");
	if (!widgetprefskey.empty()) pl->add("prefskey", widgetprefskey+"/value");
	
	osg::ref_ptr<TextWidget> hslider_value = wf->createWidget<TextWidget>("TextWidget", pl.get());

	pl = new cefix::PropertyList();
	pl->add("id", "value_vfov");
	pl->add("position", _vSliderOffset+osg::Vec3(w + 40,_textYOffset,0));
	pl->add("caption", "V:");
	pl->add("Font", _captionFont);
	pl->add("FontSize", _captionFontSize);
	pl->add("Color", _captionFontColor);
	pl->add("alignment", "right");
	if (!widgetprefskey.empty()) pl->add("prefskey", widgetprefskey+"/value");
	osg::ref_ptr<TextWidget> vslider_value = wf->createWidget<TextWidget>("TextWidget", pl.get());
	
	
	// near slider
	
	pl = new cefix::PropertyList();
	pl->add("id", "near_slider");
	pl->add("position", _nearSliderOffset);
	pl->add("size", osg::Vec2(w, _sliderWidth));
	pl->add("throbbersize", _thumbSize);
	pl->add("min", 0.1f);
	pl->add("max", 999.0f);
	pl->add("value", 1.0f);
	if (!widgetprefskey.empty()) pl->add("prefskey", widgetprefskey+"/slider");
	
	osg::ref_ptr<HorizontalSliderWidget> nearslider = wf->createWidget<HorizontalSliderWidget>("HorizontalSliderWidget",pl.get());

	
	pl = new cefix::PropertyList();
	pl->add("id", "caption_near");
	pl->add("position", _nearSliderOffset + osg::Vec3(-20,_textYOffset,0));
	pl->add("caption", "N:");
	pl->add("Font", _captionFont);
	pl->add("FontSize", _captionFontSize);
	pl->add("Color", _captionFontColor);
	if (!widgetprefskey.empty()) pl->add("prefskey", widgetprefskey+"/caption");
	osg::ref_ptr<TextWidget> nearslider_caption = wf->createWidget<TextWidget>("TextWidget", pl.get());
	
	pl = new cefix::PropertyList();
	pl->add("id", "value_near");
	pl->add("position", _nearSliderOffset+osg::Vec3(w + 40,_textYOffset,0));
	pl->add("caption", "H:");
	pl->add("Font", _captionFont);
	pl->add("FontSize", _captionFontSize);
	pl->add("Color", _captionFontColor);
	pl->add("alignment", "right");
	if (!widgetprefskey.empty()) pl->add("prefskey", widgetprefskey+"/value");
	osg::ref_ptr<TextWidget> nearslider_value = wf->createWidget<TextWidget>("TextWidget", pl.get());
	nearslider->addResponder(nearslider_value);
	
	// far slider
	
	pl = new cefix::PropertyList();
	pl->add("id", "far_slider");
	pl->add("position", _farSliderOffset);
	pl->add("size", osg::Vec2(w, _sliderWidth));
	pl->add("throbbersize", _thumbSize);
	pl->add("min", 10.0f);
	pl->add("max", 1000.0);
	pl->add("value", 1000.0f);
	if (!widgetprefskey.empty()) pl->add("prefskey", widgetprefskey+"/slider");
	osg::ref_ptr<HorizontalSliderWidget> farslider = wf->createWidget<HorizontalSliderWidget>(std::string("HorizontalSliderWidget"),pl.get());

	pl = new cefix::PropertyList();
	pl->add("id", "caption_far");
	pl->add("position", _farSliderOffset + osg::Vec3(-20,_textYOffset,0));
	pl->add("caption", "F:");
	pl->add("Font", _captionFont);
	pl->add("FontSize", _captionFontSize);
	pl->add("Color", _captionFontColor);
	if (!widgetprefskey.empty()) pl->add("prefskey", widgetprefskey+"/caption");
	
	osg::ref_ptr<TextWidget> farslider_caption = wf->createWidget<TextWidget>("TextWidget", pl.get());
	
	pl = new cefix::PropertyList();
	pl->add("id", "value_far");
	pl->add("position", _farSliderOffset+osg::Vec3(w + 40,_textYOffset,0));
	pl->add("caption", "F:");
	pl->add("Font", _captionFont);
	pl->add("FontSize", _captionFontSize);
	pl->add("Color", _captionFontColor);
	pl->add("alignment", "right");
	if (!widgetprefskey.empty()) pl->add("prefskey", widgetprefskey+"/value");
	
	osg::ref_ptr<TextWidget> farslider_value = wf->createWidget<TextWidget>("TextWidget", pl.get());
	farslider->addResponder(farslider_value);


	
	hslider->addResponder(this);
	hslider->addResponder(hslider_value);
	vslider->addResponder(this);
	vslider->addResponder(vslider_value);
	
	checkbox->addResponder(this);
	nearslider->addResponder(this);
	farslider->addResponder(this);
	
	
	add(hslider);
	add(hslider_caption);
	add(hslider_value);
	
	add(vslider);
	add(vslider_caption);
	add(vslider_value);

	add(checkbox);
	
	add(nearslider);
	add(nearslider_caption);
	add(nearslider_value);
	
	add(farslider);
	add(farslider_caption);
	add(farslider_value);

}


void FieldOfViewWidget::respondToAction(const std::string& action, AbstractWidget* widget) {
	bool inform = false;
	if ((action == ButtonWidget::Actions::selectionChanged() ) && (widget->getIdentifier() == "proportional_checkbox")) {
		CheckboxWidget* cb = dynamic_cast<CheckboxWidget*>(widget);
		if (cb->isSelected()) {
			std::cout << "disableding v-slider " << std::endl;
			get<HorizontalSliderWidget*>("vfov_slider")->disable();
			get<TextWidget*>("value_vfov")->setCaption("auto");
			_vfov = -1;
		}
		else {
			get<HorizontalSliderWidget*>("vfov_slider")->enable();
			_vfov = get<HorizontalSliderWidget*>("vfov_slider")->getValue();
		}
		inform = true;
	
	}
	if ((action == SliderWidget::Actions::valueChanged() )) {
		HorizontalSliderWidget* slider = dynamic_cast<HorizontalSliderWidget*>(widget);
		if (widget->getIdentifier() == "vfov_slider")
			_vfov = slider->getValue();
		else if (widget->getIdentifier() == "hfov_slider")
			_hfov = slider->getValue();
		else if (widget->getIdentifier() == "near_slider")
			_nearPlane = slider->getValue();
		else if (widget->getIdentifier() == "far_slider")
			_farPlane = slider->getValue();


		inform = true;
	}
	if (inform) {
		informAttachedResponder( Actions::fieldOfViewChanged() );
		saveToPrefs();
	}
}



osg::Matrix FieldOfViewWidget::computeProjectionMatrix(ApplicationWindow* win) 
{ 
	WindowRotation w_rot(win->getRotation());
	w_rot.setViewport(0,0, win->getActualWidth(), win->getActualHeight());
	return w_rot.computeProjectionMatrix(_hfov, _vfov, _nearPlane, _farPlane);
}

void FieldOfViewWidget::applyProjectionMatrix(ApplicationWindow* win)
{
	if (get<CheckboxWidget*>("proportional_checkbox")->isSelected()) {
		double ratio = win->getGraphicsWindow()->getTraits()->height / (double) win->getGraphicsWindow()->getTraits()->width;
		_vfov =  osg::RadiansToDegrees(2.0 * atan (tan(osg::DegreesToRadians(_hfov)/2.0) * (ratio) ));
		get<HorizontalSliderWidget*>("vfov_slider")->setValue(_vfov);
	}
	
	osg::Matrix m = computeProjectionMatrix(win);
	win->getCamera()->setProjectionMatrix(m);
	// win->getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
}

class FieldOfViewResponder : public virtual AbstractWidget::Responder {
	public:
		FieldOfViewResponder(ApplicationWindow* win) : AbstractWidget::Responder(), _win(win) {}
		
		void respondToAction(const std::string& action, AbstractWidget* widget) {
			if (_win.valid() && action == FieldOfViewWidget::Actions::fieldOfViewChanged() ) {
				FieldOfViewWidget* w = dynamic_cast<FieldOfViewWidget*>(widget);
				w->applyProjectionMatrix(_win.get());
			}
		}
	private:
		osg::observer_ptr<ApplicationWindow> _win;
};


void FieldOfViewWidget::saveToPrefs() {

	if (_windowPrefsKey.empty())
		return;
		
	cefix::DataFactory* df = cefix::DataFactory::instance();
 
	df->setPreferencesFor(_windowPrefsKey+"/perspective/nearplane", _nearPlane);
	df->setPreferencesFor(_windowPrefsKey+"/perspective/farplane", _farPlane);
	df->setPreferencesFor(_windowPrefsKey+"/perspective/horizontalFieldOfView", _hfov);
	df->setPreferencesFor(_windowPrefsKey+"/perspective/verticalFieldOfView", _vfov);
	

}
		

void FieldOfViewWidget::applyToWindow(ApplicationWindow* win) {
	_windowPrefsKey = win->getPrefsKey();	
	
	_nearPlane			=  DataFactory::instance()->getPreferencesFor(_windowPrefsKey+"/perspective/nearplane", 1.0f);
	_farPlane			=  DataFactory::instance()->getPreferencesFor(_windowPrefsKey+"/perspective/farplane", 1000.0f);
	_hfov				=  DataFactory::instance()->getPreferencesFor(_windowPrefsKey+"/perspective/horizontalFieldOfView", 60.0f);
	_vfov				=  DataFactory::instance()->getPreferencesFor(_windowPrefsKey+"/perspective/verticalFieldOfView", -1.0f);
	
	get<HorizontalSliderWidget*>("hfov_slider")->setValue(_hfov);
	get<HorizontalSliderWidget*>("vfov_slider")->setValue(_hfov);
	get<HorizontalSliderWidget*>("near_slider")->setValue(_nearPlane);
	get<HorizontalSliderWidget*>("far_slider")->setValue(_farPlane);

	
	addResponder(new FieldOfViewResponder(win));

}

}


