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

#ifndef VIEWPOINT_MANAGER_WIDGET_HEADER
#define VIEWPOINT_MANAGER_WIDGET_HEADER

#include <osg/Referenced>
#include <osgGA/CameraManipulator>
#include <cefix/TransformWidget.h>

namespace cefix {
/** with the viewpointManagerWidget you can record viewpoints, store and load them via xml and jump to a previously stored viewpoint. To work correctly you have to implement a Responder, 
   which listens to the following actions: recordNewViewPointRequested and viewpointChangeRequested. youget the current selected viewpoint with getCurrentSelectedViewPoint 
   and add a new viewpoint via addNewViewPoint. A much simpler method is to register a cameraManipulator with the widget, the widget will update the manipulator when needed.
*/
class CEFIX_EXPORT ViewPointManagerWidget : public TransformWidget, public AbstractWidget::Responder {

	public:
	
		/// Actions:
	class Actions : public AbstractWidget::Actions {
			public:
			static const char* recordNewViewPointRequested() { return "recordNewViewPointRequested"; }
			static const char* statusChanged() { return "statuschanged"; }
			static const char* viewpointChangeRequested() { return "viewpointChangeRequested"; }
		private:
		Actions() : AbstractWidget::Actions()	{};
		};
		class Responder : public AbstractWidget::Responder {
		
			public:
					
				Responder() : AbstractWidget::Responder() {}
				
				virtual void recordNewViewPoint() {};
				virtual void changeViewPoint() {};
				
				virtual void respondToAction(const std::string& action, AbstractWidget* w)
				{
					_vpmanager = dynamic_cast<ViewPointManagerWidget*>(w);
					if (_vpmanager) {
						if (action == Actions::recordNewViewPointRequested() )
							recordNewViewPoint();
						else if (action == Actions::viewpointChangeRequested() )
							changeViewPoint();
					}
				}
			protected:
				ViewPointManagerWidget* getViewPointManager() { return _vpmanager; }
			private:
				ViewPointManagerWidget* _vpmanager; 
		
		};
		
		typedef std::vector<osg::Matrix>	ViewPointList;
		
		/** ctor */
		ViewPointManagerWidget(const std::string& identifer, const osg::Vec3& position, const osg::Vec2&size, const float btn_delta = 5);
		
		/** ctor */
		ViewPointManagerWidget(cefix::PropertyList* pl);
		
		/** respond to the actions of the embedded widgets */
		virtual void respondToAction(const std::string& action, AbstractWidget* widget); 
		
		/** sets the prefs key to use to store the list of viewpoints, they are stored into preferences.xml */
		void setViewPointsDestinationPrefsKey(const std::string& prefskey) { _destinationPrefsKey = prefskey; }
		
		/** get a list of viewpoints from the prefs-file, respecting the prefs-key given  with setViewPointsDestinationPrefsKey */
		void loadViewPoints();
		
		/** adds a new viewpoint to the list of stored viewpoints, commonly called from a responder handling the recordNewViewPointRequested-action */
		void addNewViewPoint(const osg::Matrix& m) { _viewpoints.push_back(m); updateButtonStates(); saveViewPoints(); }
		
		/** regsiter a cameramanipulator with this widget, if a new viewpoint is selected, the cameramanipulator gets updated, if a new viewpoint soul dbe recorde, 
		    it is taken from the current cameramanipulator's matrix */
		void applyToCameraManipulator(osgGA::CameraManipulator* manip) { _manip = manip; }
		
		/** get the current selected viewpoint, usually called from a responder handling the viewpointChangeRequested-action */
		osg::Matrix getCurrentSelectedViewPoint() { return (_currentViewPointNdx < _viewpoints.size()) ? osg::Matrix::identity() : _viewpoints[_currentViewPointNdx]; }
	
		/** provide a status text */
		std::string getStatusText();
	protected:
		
		void saveViewPoints();
		
		void createWidgets(const osg::Vec2& size, float btn_delta, const std::string& widgetprefskey ="");

		
		void updateButtonStates();
		
	private:
		unsigned int _currentViewPointNdx;
		ViewPointList	_viewpoints;
		std::string	_destinationPrefsKey;
		osg::observer_ptr<osgGA::CameraManipulator> _manip;
		
	

};

}

#endif