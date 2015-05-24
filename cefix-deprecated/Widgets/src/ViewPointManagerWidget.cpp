/*
 *  ViewPointManagerWidget.cpp
 *  theseus_texo_illo
 *
 *  Created by Stephan Huber on 22.11.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "ViewPointManagerWidget.h"
#include <cefix/PropertyList.h>
#include <cefix/TextButtonWidget.h>
#include <cefix/WidgetFactory.h>
#include <cefix/DataFactory.h>
#include <cefix/WidgetDefaults.h>

namespace cefix {

ViewPointManagerWidget::ViewPointManagerWidget(const std::string& identifier, const osg::Vec3& position, const osg::Vec2&size, const float btn_delta) :
	TransformWidget(identifier, osg::Matrix::translate(position)),
	_currentViewPointNdx(0),
	_manip(NULL)
{
	createWidgets(size, btn_delta);
}

ViewPointManagerWidget::ViewPointManagerWidget(cefix::PropertyList* pl) :
	TransformWidget(pl->get("id")->asString(), osg::Matrix::translate(pl->get("position")->asVec3())),
	_currentViewPointNdx(0),
	_manip(NULL)
{
	osg::Vec2 size = pl->get("size")->asVec2();
	float btn_delta = 5;
	if (pl->hasKey("btnDelta")) btn_delta = pl->get("btnDelta")->asFloat();
	if (pl->hasKey("viewpointsPrefKey")) setViewPointsDestinationPrefsKey(pl->get("viewpointsPrefKey")->asString());
	createWidgets(size, btn_delta);
	
	loadViewPoints();
}
		
void ViewPointManagerWidget::createWidgets(const osg::Vec2& size, float btn_delta, const std::string& widgetprefskey) {
	
	cefix::WidgetFactory* wf = cefix::WidgetFactory::instance();
	
	float width = (size[0] - (2*btn_delta)) / 3.0;
	float height = WidgetDefaults::getButtonHeight();
	
	osg::ref_ptr<cefix::PropertyList> pl;
	
	pl = new cefix::PropertyList();
	pl->add("id", "rewind_btn");
	pl->add("size", osg::Vec2(width, height));
	pl->add("position", osg::Vec3(0,0,0) );
	pl->add("caption", "<");
	if (!widgetprefskey.empty()) pl->add("prefskey", widgetprefskey+"/value");
	
	osg::ref_ptr<TextButtonWidget> rewind_btn = wf->createWidget<TextButtonWidget>("TextButtonWidget", pl.get());


	pl = new cefix::PropertyList();
	pl->add("id", "record_btn");
	pl->add("position", osg::Vec3(width + btn_delta,0,0) );
	pl->add("size", osg::Vec2(width, height));
	pl->add("caption", "O");
	if (!widgetprefskey.empty()) pl->add("prefskey", widgetprefskey+"/value");
	
	osg::ref_ptr<TextButtonWidget> record_btn = wf->createWidget<TextButtonWidget>("TextButtonWidget", pl.get());

	pl = new cefix::PropertyList();
	pl->add("id", "forward_btn");
	pl->add("position", osg::Vec3(2*width + 2*btn_delta,0,0) );
	pl->add("size", osg::Vec2(width, height));
	pl->add("caption", ">");
	if (!widgetprefskey.empty()) pl->add("prefskey", widgetprefskey+"/value");
	
	osg::ref_ptr<TextButtonWidget> forward_btn = wf->createWidget<TextButtonWidget>("TextButtonWidget", pl.get());
	
	rewind_btn->addResponder(this);
	record_btn->addResponder(this);
	forward_btn->addResponder(this);
	
	add(rewind_btn);
	add(record_btn);
	add(forward_btn);
	
	
	

}		
void ViewPointManagerWidget::respondToAction(const std::string& action, AbstractWidget* widget) {

	
	bool update = false;
	TextButtonWidget* button = dynamic_cast<TextButtonWidget*>(widget);
	if (!button || (button->getState() != cefix::AbstractWidget::DEPRESSED)) return;
	
	if (widget->getIdentifier() == "record_btn") {
		if (_manip.valid())
			addNewViewPoint(_manip->getMatrix());
		
		informAttachedResponder( Actions::recordNewViewPointRequested() );
	} else if ((widget->getIdentifier() == "rewind_btn") && (_currentViewPointNdx > 0)) {
		--_currentViewPointNdx;
		update = true;
	} else if ((widget->getIdentifier() == "forward_btn") && (_currentViewPointNdx+1 < _viewpoints.size() )) {
		++_currentViewPointNdx;
		update = true;
	}
	
	if (update) {
		if (_manip.valid())
			_manip->setByMatrix(_viewpoints[_currentViewPointNdx]);
		
		informAttachedResponder( Actions::viewpointChangeRequested() );
		updateButtonStates();
	}
	
	informAttachedResponder( Actions::statusChanged() );
}


void ViewPointManagerWidget::saveViewPoints() 
{	
	cefix::PropertyList* pl = new cefix::PropertyList();
		
	for(unsigned int i = 0; i < _viewpoints.size(); ++i) {
		pl->add("viewpoint", _viewpoints[i]);
	}
	
	if (!cefix::DataFactory::instance()->setPreferencesFor<osg::Referenced*>(_destinationPrefsKey, pl)) {
		log::error("ViewPointManagerWidget") << "could not save viewpoints to prefs-file " << std::endl;
	}

	
}


void ViewPointManagerWidget::loadViewPoints() 
{	
	_viewpoints.clear();
	cefix::PropertyList* pl = cefix::DataFactory::instance()->getPreferencesPropertyListFor(_destinationPrefsKey);
	if (pl == NULL) {
		osg::notify(osg::ALWAYS) << "ViewPointManagerWidget :: no viewpoints found for prefskey: "<< _destinationPrefsKey << std::endl;
		updateButtonStates();
		return;
	}
	
	for(unsigned int i = 0; i < pl->size(); ++i) {
		if ((pl->getKeyAt(i) == "viewpoint")) {
			osg::Matrix m = pl->get(i)->asMatrix();
			_viewpoints.push_back(m);
		}
	}
	_currentViewPointNdx = 0;
	
	updateButtonStates();
	informAttachedResponder( Actions::statusChanged() );
}

void ViewPointManagerWidget::updateButtonStates() {
	if (_viewpoints.size() == 0) {
		get<TextButtonWidget*>("rewind_btn")->disable();
		get<TextButtonWidget*>("forward_btn")->disable();
	}
	
	if (_currentViewPointNdx == 0)
		get<TextButtonWidget*>("rewind_btn")->disable();
	else
		get<TextButtonWidget*>("rewind_btn")->enable();
		
	if (_currentViewPointNdx+1 >= _viewpoints.size())
		get<TextButtonWidget*>("forward_btn")->disable();
	else
		get<TextButtonWidget*>("forward_btn")->enable();		
}	

std::string ViewPointManagerWidget::getStatusText() {
	std::stringstream ss;
	if (_viewpoints.size() == 0) 
		ss << "no viewpoints recorded " << std::endl;
	else
		ss << _viewpoints.size() << " recorded, current: " << (_currentViewPointNdx + 1);
	
	return ss.str();
}

}

