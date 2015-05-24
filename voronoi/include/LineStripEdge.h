/*
 *  LineStripEdge.h
 *  livevoronoi
 *
 *  Created by Stephan Huber on 05.06.08.
 *  Copyright 2008 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#ifndef CEFIX_LINESTRIP_EDGE_HEADER
#define CEFIX_LINESTRIP_EDGE_HEADER

#include <osg/Vec3>
#include <list>

namespace cefix {

class LineStripEdge {
	public:
		LineStripEdge(const osg::Vec3& p1, const osg::Vec3& p2);
		
		bool contains(const osg::Vec3 v) { return ((_left == v) || (_right == v) ); }
		
		LineStripEdge* getLeftEdge() const { return _leftPtr; }
		LineStripEdge* getRightEdge() const { return _rightPtr; }
		
		
		const osg::Vec3& left() const  { return _left; }
		const osg::Vec3& right() const { return _right; }
		
		bool fitLeft(const osg::Vec3& p) const { return ((_leftPtr == NULL) && (_left == p)); }
		bool fitRight(const osg::Vec3& p) const { return ((_rightPtr == NULL) && (_right == p)); }
		
		LineStripEdge* addLeft(const osg::Vec3& p1, const osg::Vec3& p2) { return attachEdgeToLeft(new LineStripEdge(p1,p2)); }
		LineStripEdge* addRight(const osg::Vec3& p1, const osg::Vec3& p2) { return attachEdgeToRight(new LineStripEdge(p1,p2)); }
		
		LineStripEdge* attachEdgeToLeft(LineStripEdge* e) 
		{ 
			if (e->right() != _left) e->flipDirection();
			_leftPtr = e; 
			_leftPtr->setRightEdge(this);
            
            return _leftPtr; 
		}
		
		LineStripEdge* attachEdgeToRight(LineStripEdge* e) 
		{	
			if (e->left() != _right) e->flipDirection();
			_rightPtr = e; 
			_rightPtr->setLeftEdge(this); 
            
			return _rightPtr; 
				
		}
		
		void setLeftEdge(LineStripEdge* p) { _leftPtr = p; }
		void setRightEdge(LineStripEdge* p) { _rightPtr = p;  }
		
		LineStripEdge* getMostLeft() {
			if (_leftPtr == NULL) return this;
			
			LineStripEdge* result = _leftPtr;
			while (result) 
			{
				LineStripEdge* current = result->getLeftEdge();
				if (!current)
					return result;
				else
					result = current;
			}
			
			return NULL;
		}
		
		LineStripEdge* getMostRight() {
			if (_rightPtr == NULL) return this;
			
			LineStripEdge* result = _rightPtr;
			while (result) 
			{
				LineStripEdge* current = result->getRightEdge();
				if (!current)
					return result;
				else
					result = current;
			}
			return NULL;
		}
		
		
		
		static LineStripEdge* add(LineStripEdge* edge, const osg::Vec3& v1, const osg::Vec3& v2);
			
		virtual ~LineStripEdge() {
			// Referenzen zu diesem Objekt bei seinen Nachbarn löschen (sonst werden wir mehrmals gelöscht)
			if (_leftPtr) _leftPtr->setRightEdge(NULL);
			if (_rightPtr) _rightPtr->setLeftEdge(NULL);
			
			if (_leftPtr) delete _leftPtr;
			if (_rightPtr) delete _rightPtr; 
		}
		
		
		
	
		void flipDirection(LineStripEdge* source = NULL) 
		{
			std::swap(_left, _right);
			std::swap(_leftPtr, _rightPtr);
			if ((_leftPtr) && ( _leftPtr != source)) _leftPtr->flipDirection(this);
			if ((_rightPtr) && ( _rightPtr != source)) _rightPtr->flipDirection(this);
		}
		
	private:
		osg::Vec3 _left, _right;
		LineStripEdge* _leftPtr;
		LineStripEdge* _rightPtr;
	};
	
	
	class LineStripEdgeGroup {
		public:
			typedef std::list<LineStripEdge*> EdgeList;
			LineStripEdgeGroup() : _current(NULL), _looseEnds(), _itr(_looseEnds.end()), _dirty(true), _debug(false), _isClosed(false) {}
			void addEdge(const osg::Vec3& p1, const osg::Vec3& p2); 
			
			
			LineStripEdge* getStartEdge() { combineLooseEnds(); _itr = _looseEnds.begin(); return (_current) ? _current->getMostLeft() : NULL;  }
			LineStripEdge* getNextEdge() { return (_itr != _looseEnds.end()) ? (*_itr++) : NULL; }
		
			virtual ~LineStripEdgeGroup() {
			
				if (_current) delete _current;
				for(std::list<LineStripEdge*>::iterator i = _looseEnds.begin(); i != _looseEnds.end(); ++i) 
				{
					delete *i;
				}
				_looseEnds.clear();
			}
			void setDebug(bool b) { _debug = b; }
			
			bool isClosed() { if (_dirty) combineLooseEnds(); return _isClosed; }
						
		protected:
			void combineLooseEnds();
		private:
			LineStripEdge*		_current;
			EdgeList			_looseEnds;
			EdgeList::iterator	_itr;
			bool				_dirty,_debug, _isClosed;
	
	};
	
std::ostream& operator<<(std::ostream& ostr, LineStripEdge& edge);
std::ostream& operator<<(std::ostream& ostr, LineStripEdgeGroup& edge);

}
#endif
