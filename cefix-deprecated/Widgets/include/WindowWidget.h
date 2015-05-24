/*
 *
 *      (\(\
 *     (='.')           cefix --
 *    o(_")")           a multipurpose library on top of OpenSceneGraph
 *  
 *
 *                      Copyright 2001-2011, stephanmaximilianhuber.com 
 *                      Stephan Maximilian Huber. 
 *
 *                      All rights reserved.
 *
 */

#ifndef WINDOW_WIDGET_HEADER
#define WINDOW_WIDGET_HEADER
#include <cefix/ViewWidget.h>
#include <cefix/GroupWidget.h>
#include <cefix/TextWidget.h>
#include <cefix/DragWidget.h>
#include <cefix/PropertyList.h>

namespace cefix {



class CEFIX_EXPORT WindowWidget : public ViewWidgetT<GroupWidget> {

	public:
		
		class Actions: public AbstractWidget::Actions {
		public:
			static const char* windowDragged() { return "windowDragged"; }
		protected:
			Actions(): AbstractWidget::Actions() {}
		};
	
		class Responder : public virtual AbstractWidget::Responder {
			public:
				Responder() : AbstractWidget::Responder() {}
				
				virtual void windowDragged() {}
				
				virtual void respondToAction(const std::string& action, AbstractWidget* widget)
				{
					WindowWidget* _window = dynamic_cast<WindowWidget*>(widget);
					if (_window && (action == Actions::windowDragged() ))
						windowDragged();
				};
			protected:
				WindowWidget* getWindow() { return _window; }
			private:
				WindowWidget* _window;
				
		};

		
	protected:
		WindowWidget(const std::string& identifier, const std::string& windowTitle, const osg::Vec3& pos, const osg::Vec2& size, Mode mode = ORTHO, RenderMode rendermode = DIRECT);
		WindowWidget(cefix::PropertyList* pl);
	public:	
		void setTitleBarWidget(DragWidget* w) { 
			addDecorationWidget(w, "Titlebar"); 
		}
		
		void setTitleTextWidget(TextWidget* w) { 
			addDecorationWidget(w, "TitleText"); 
			if (w) w->setCaption(getWindowTitle());
		}
		
		void setShadowWidget(RectControlWidget* w) {
			addDecorationWidget(w, "Shadow");
		}
		
		virtual void update();
		float getTitleBarHeight() const { return _titlebarHeight; }
		void setTitlebarHeight(float v) { _titlebarHeight = v; dirty(); }
		void setTitleTextDelta(const osg::Vec3& v) { _titleTextDelta = v; dirty(); }
		
		virtual void move(const osg::Vec3& pos) { ViewWidgetT<GroupWidget>::move(pos); dirty(); }
		virtual void resize(float width, float height) { ViewWidgetT<GroupWidget>::resize(width, height); dirty(); }
		
		void windowDragged(const osg::Vec3& newpos) {
			// std::cout << "newpos " << newpos << std::endl;
			move(newpos -  osg::Vec3(0, getHeight(), 0) - _titleBarDelta);
			update();
			this->informAttachedResponder( Actions::windowDragged() );
		}
		
		virtual osg::Node* getNode() { return _container.get(); }
		
		osg::Group* getContainer() { return _container.get(); }
		
		const std::string& getWindowTitle() { return _windowTitle; }
		void setWindowTitle(const std::string& t) { _windowTitle = t; update(); }
		
		void setTitleBarDelta(const osg::Vec3& d) { _titleBarDelta = d; update(); }
		
		
	protected:
		
		void addDecorationWidget(AbstractWidget* w, const std::string& identifier) 
		{
			w->setIdentifier(identifier);
			_decoration->add(w);
			dirty();
		}
		
		void setBackgroundIncludesTitleBarFlag(bool f) { _backgroundIncludesTitleBar = f; }
		
	private:
		void dirty() { _dirty = true; }
		std::string					_windowTitle;
		osg::ref_ptr<GroupWidget>	_decoration;
		bool						_dirty;
		float						_titlebarHeight;
		osg::Vec3					_titleTextDelta, _titleBarDelta;
		osg::ref_ptr<osg::Group>	_container;
		bool						_firstRun, _backgroundIncludesTitleBar;
};


class SimpleWindowWidget: public WindowWidget {

	public:
		SimpleWindowWidget(const std::string& identifier, const std::string& windowTitle, const osg::Vec3& pos, const osg::Vec2& size, Mode mode = ORTHO, RenderMode rendermode = DIRECT) :
			WindowWidget(identifier, windowTitle, pos, size, mode, rendermode)
		{
			init(pos, size);
		}
		
		SimpleWindowWidget(cefix::PropertyList* pl) :
			WindowWidget(pl)
		{
			init(pl->get("position")->asVec3(), pl->get("size")->asVec2());
		}
		
	private:
		void init(osg::Vec3 pos, osg::Vec2 size);
};




} // end of namespace

#endif