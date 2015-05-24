/*
 *  WindowWidget.cpp
 *  cefixWidgets
 *
 *  Created by Stephan Huber on 19.06.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "WindowWidget.h"
#include <cefix/WidgetFactory.h>

#include <osgDB/FileUtils>

namespace cefix {

class WindowShadowWidget: public RectControlWidget {

	public:
		WindowShadowWidget(const std::string& identifier, const osg::Vec3& pos, const osg::Vec2& sizem, const osg::Vec2& delta = osg::Vec2(3,-3));
		
		virtual void update() 
		{
			osg::Vec3 pos = getPosition();
			osg::Vec2 size = getSize(); 
			
			if (_shadow.valid()) {
				_shadow->setRect(osg::Vec4(pos[0]+_delta[0], pos[1]+_delta[1] , pos[0] + size[0]+_delta[0], pos[1] + size[1]+_delta[1]));
				_shadow->setLocZ(pos[2] - 1);
			}
		}
		
		virtual osg::Node* getNode() { return _geode.get(); }
		
	private:
		osg::ref_ptr<DropShadowGeometry>	_shadow;
		osg::ref_ptr<osg::Geode>			_geode;
		osg::Vec2							_delta;
};

WindowShadowWidget::WindowShadowWidget(const std::string& identifier, const osg::Vec3& pos, const osg::Vec2& size, const osg::Vec2& delta) :
	RectControlWidget(identifier, pos, size),
	_delta(delta) 
{
	_geode = new osg::Geode();
	std::string fn = osgDB::findDataFile("dropshadow.png");
	if (!fn.empty()) {
		_shadow = new cefix::DropShadowGeometry(osg::Vec4(pos[0], pos[1] + size[1], pos[0] + size[0], pos[1]), pos[2], 30, "dropshadow.png", 15);
		_geode->addDrawable(_shadow.get());
		_geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		_geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		_geode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	} else {
		log::error("WindowShadowWidget")<< "could not locate dropshadow.png" << std::endl;
	}
}



WindowWidget::WindowWidget(const std::string& identifier, const std::string& windowTitle, const osg::Vec3& pos, const osg::Vec2& size, Mode mode, RenderMode rendermode) 
:	ViewWidgetT<GroupWidget>(identifier, pos, size, mode, rendermode),
	_windowTitle(windowTitle),
	_titlebarHeight(16),
	_titleTextDelta(5,5,1),
	_titleBarDelta(0,0,0),
	_firstRun(true),
	_backgroundIncludesTitleBar(true)
{
	_decoration = new GroupWidget("WindowDecoration");
	
	_container = new osg::Group();
	_container->addChild(_decoration->getNode());
	getCamera()->addChild(GroupWidget::getGroupNode());
	_container->addChild(ViewWidgetT<GroupWidget>::getNode());
	
	//getCamera()->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

WindowWidget::WindowWidget(cefix::PropertyList* pl) 
:	ViewWidgetT<GroupWidget>(pl),
	_windowTitle(pl->get("title")->asString()),
	_titlebarHeight(16),
	_titleTextDelta(5,5,1),
	_titleBarDelta(0,0,0),
	_firstRun(true),
	_backgroundIncludesTitleBar(true)
{
	if (pl->hasKey("titleBarHeight")) _titlebarHeight = pl->get("titleBarHeight")->asFloat();
	if (pl->hasKey("titleTextDelta")) _titleTextDelta = pl->get("titleTextDelta")->asVec3();
	if (pl->hasKey("titleBarDelta")) _titleTextDelta = pl->get("titleBarDelta")->asVec3();
	
	_decoration = new GroupWidget("WindowDecoration");
	
	_container = new osg::Group();
	_container->addChild(_decoration->getNode());
	getCamera()->addChild(GroupWidget::getGroupNode());
	_container->addChild(ViewWidgetT<GroupWidget>::getNode());
	
	if (pl->hasKey("clearColor")) {
		getCamera()->setClearColor(pl->get("clearColor")->asVec4());
		getCamera()->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}


void WindowWidget::update() {
	
	if (_dirty) 
	{
		DragWidget* dw = _decoration->get<DragWidget*>("Titlebar", false);
		if (dw) {
			dw->setPositionAndSize(getPosition() + _titleBarDelta + osg::Vec3(0,getHeight(), 1), osg::Vec2(getWidth(), _titlebarHeight));
			if (_firstRun) {
				//cefix::Functor1ParamT<void, const osg::Vec3&, WindowWidget>* cb = new cefix::Functor1ParamT<void, const osg::Vec3&, WindowWidget>(this, &WindowWidget::windowDragged);
				dw->setDragCallback(cefix::Functor::create(this, &WindowWidget::windowDragged));
				_firstRun = false;
			}
		}
		
		TextWidget* tw = _decoration->get<TextWidget*>("TitleText", false);
		if (tw){
			tw->getTextGeode()->setText(_windowTitle);
			tw->setPosition(getPosition() + _titleTextDelta + osg::Vec3(0,getHeight(),2));
		}
		
		RectControlWidget* sw = _decoration->get<RectControlWidget*>("Shadow", false);
		if (sw) {
			sw->setPositionAndSize(getPosition(),  osg::Vec2(getWidth(), (_backgroundIncludesTitleBar) ? getHeight() + _titlebarHeight : getHeight()));
		}		
		
	}
	
	_decoration->update();
	
	ViewWidgetT<GroupWidget>::update();
}


void SimpleWindowWidget::init(osg::Vec3 pos, osg::Vec2 size) 
{
	setTitleBarWidget(new SimpleDragWidget("Titlebar", pos, size));
	
	setShadowWidget(new WindowShadowWidget("Shadow", pos, size+osg::Vec2(0, getTitleBarHeight())));
	TextWidget* tw = new TextWidget("TitleText", pos);
	tw->getTextGeode()->setTextColor(osg::Vec4(1,1,1,0.7));
	setTitleTextWidget(tw);
	update();
}


}


