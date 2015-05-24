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

#ifndef FIELD_OF_VIEW_WIDGET
#define FIELD_OF_VIEW_WIDGET

#include <cefix/TransformWidget.h>

namespace cefix {

	class ApplicationWindow;
	class PropertyList;
	
	/** this widget allows you to control the field of view, you can control the horizontal and the vertical field of view, the near-plane and the far-plane */
	class CEFIX_EXPORT FieldOfViewWidget : public TransformWidget, public AbstractWidget::Responder {
		
		public:
			class Actions : public AbstractWidget::Actions {
			public:
				static const char* fieldOfViewChanged() { return "fieldofviewchanged"; }
			protected:
				Actions(): AbstractWidget::Actions() {}
			};
			
			class Responder : public virtual AbstractWidget::Responder {
			
				public:
					Responder() : AbstractWidget::Responder() {}
					
					virtual void fieldOfViewChanged() {}
					
					virtual void respondToAction(const std::string& action, AbstractWidget* w)
					{
						_fovwidget = dynamic_cast<FieldOfViewWidget*>(w);
						if (_fovwidget) {
							if (action == Actions::fieldOfViewChanged() )
								fieldOfViewChanged();
						}
					}
				protected:
					FieldOfViewWidget* getFieldOfViewWidget() { return _fovwidget; }
				private:
					FieldOfViewWidget* _fovwidget; 
			};
	
		public:
			/** ctor */
			FieldOfViewWidget(const std::string& identifier, const osg::Vec3& pos, const osg::Vec2& size, bool vertical = true);
			
			/** ctor */
			FieldOfViewWidget(cefix::PropertyList* pl);
			
			/** get the horizontal field of view */
			inline float getHorizontalFieldOfView() const { return _hfov; }
			
			/** get the vertical field of view */
			inline float getVerticalFieldOfView() const { return _vfov; }
			
			/** get the near plane */
			inline float getNearPlane() const { return _nearPlane; }
			
			/** get the far plane */
			inline float getFarPlane() const { return _farPlane; }
			
			/** compute the projection matrix for the given window (win needed for display-rotation, width and height) */
			osg::Matrix computeProjectionMatrix(ApplicationWindow* win);
			
			/** apply projectionMatrix to window win */
			void applyProjectionMatrix(ApplicationWindow* win);
			
			/** respond to the diverse widgets */
			virtual void respondToAction(const std::string& action, AbstractWidget* widget);
			
			/** register window win to the widget, so modifications of the field of views get applied directly to the given window */
			void applyToWindow(ApplicationWindow* win);
			
		protected:
			void saveToPrefs();
			
					
		private:
			void computeOffsets(const osg::Vec2& size, bool vertical);
			void createWidgets(const osg::Vec2& size, bool vertical, const std::string& widgetPrefsKey = "");
			
			float _hfov, _vfov, _nearPlane, _farPlane;
			osg::Vec3 _hSliderOffset, _vSliderOffset, _nearSliderOffset, _farSliderOffset, _checkboxOffset;
			float	_sliderWidth;
			osg::Vec2	_thumbSize;
			std::string _captionFont;
			float _captionFontSize;
			osg::Vec4	_captionFontColor;
			float		_textYOffset;

			std::string		_windowPrefsKey;
			
			
	};

}


#endif