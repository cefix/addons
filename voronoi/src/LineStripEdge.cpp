/*
 *  LineStripEdge.cpp
 *  livevoronoi
 *
 *  Created by Stephan Huber on 05.06.08.
 *  Copyright 2008 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#include "LineStripEdge.h"
#include <iostream>

#include <osg/io_utils>

namespace cefix {



LineStripEdge::LineStripEdge(const osg::Vec3& p1, const osg::Vec3& p2) 
:	_left(p1), 
	_right(p2), 
	_leftPtr(NULL), 
	_rightPtr(NULL) 
{
}

void LineStripEdgeGroup::addEdge(const osg::Vec3& p1, const osg::Vec3& p2) 
{ 
	_dirty = true;
	if (!_current) {
		_current = new LineStripEdge(p1,p2);
		return;
	}
	
	LineStripEdge* result = LineStripEdge::add(_current, p1, p2);
	if (result) {
		_current = result->getMostLeft();
		return;
	}
	
	
	for(std::list<LineStripEdge*>::iterator i = _looseEnds.begin(); ( (i != _looseEnds.end()) && !result ); ++i) 
	{
		result = LineStripEdge::add( *i, p1, p2);
		if (result) {
			//combineLooseEnds();
			return;
		}
	}
	
	
	_looseEnds.push_back(new LineStripEdge(p1,p2));
	
}

LineStripEdge* LineStripEdge::add(LineStripEdge* edge, const osg::Vec3& v1, const osg::Vec3& v2) 
{
	// erst ganz links probieren
	LineStripEdge* mostLeft = edge->getMostLeft();
	if (mostLeft->fitLeft(v1))
		return mostLeft->addLeft(v2,v1);
	if (mostLeft->fitLeft(v2))
		return mostLeft->addLeft(v1,v2);
		
	// dann ganz rechts probieren
	LineStripEdge* mostRight = edge->getMostRight();
	if (mostRight->fitRight(v1))
		return mostRight->addRight(v1,v2);
	if (mostRight->fitRight(v2))
		return mostRight->addRight(v2,v1);
	
	// edge kann nicht hinzugefügt werden
	return NULL; 
}

void LineStripEdgeGroup::combineLooseEnds() {

	if (!_dirty) return;
	if (_debug) {
		if (_current) {
			std::cout << "before: " << *_current << std::endl;
		} else {
			std::cout << "before: 0x0" << std::endl;
		}
		for(std::list<LineStripEdge*>::iterator i = _looseEnds.begin(); i != _looseEnds.end(); ++i) {
			std::cout << "loose end: " << (**i) << std::endl;
		}
	}
	for(std::list<LineStripEdge*>::iterator i = _looseEnds.begin(); i != _looseEnds.end(); ) 
	{
		bool remove(true);
				
		LineStripEdge* current_ml = _current->getMostLeft();
		LineStripEdge* current_mr = _current->getMostRight();

		
		LineStripEdge* ml = (*i)->getMostLeft();
		LineStripEdge* mr = (*i)->getMostRight();
		
		
		
		if (current_ml->fitLeft(mr->right())) 
		{
			if (_debug) { std::cout << "attachToLeft :" << *mr << " onto " << *current_ml << std::endl; }
			current_ml->attachEdgeToLeft( mr );
		} 
		else if (current_ml->fitLeft(ml->left())) 
		{
			if (_debug) { std::cout << "attachToLeft :" << *ml << " onto " << *current_ml << std::endl; }
			current_ml->attachEdgeToLeft( ml );
		} 
		else if (current_mr->fitRight(mr->right())) 
		{
			if (_debug) { std::cout << "attachToRight:" << *mr << " onto " << *current_mr << std::endl; }
			current_mr->attachEdgeToRight( mr );
		} 
		else if (current_mr->fitRight(ml->left())) 
		{
			if (_debug) { std::cout << "attachToRight:" << *ml << " onto " << *current_mr << std::endl; }
			current_mr->attachEdgeToRight( ml );
		} 
		else 
			remove = false;
		
		if (remove) 
		{
			i = _looseEnds.erase(i);
		} 
		else 
		{
			++i;
		}
	}
	if (_debug) {
		if (_current) {
			std::cout << "after: " << *_current << std::endl;
		} else {
			std::cout << "after: 0x0" << std::endl;
		}
		for(std::list<LineStripEdge*>::iterator i = _looseEnds.begin(); i != _looseEnds.end(); ++i) {
			std::cout << "loose end: " << (**i) << std::endl;
		}
	}
	
	_isClosed = false;
	
	if (_current && (_looseEnds.size() == 0)) {
		LineStripEdge* l = _current->getMostLeft();
		LineStripEdge* r = _current->getMostRight();
		float delta2 = (l->left() - r->right()).length2();
		_isClosed = (delta2 < 0.01);
	}
	
	_dirty = false;
	
    /*
    if (!_isClosed) {
        osg::Vec3 p = getStartEdge()->getMostLeft()->left();
        if ((p[0] <= 100) || (p[1] < 100))
            return;
		std::cout << "not closed? " << _looseEnds.size() << std::endl;
        std::cout << getStartEdge()->getMostLeft()->left() << " - " << getStartEdge()->getMostRight()->right() << std::endl;
        
        std::cout << (*this) << std::endl;
        std::cout << std::endl;
	}
    */
}


std::ostream& operator<<(std::ostream& ostr, LineStripEdge& edge) 
{
	LineStripEdge* e = edge.getMostLeft();
	bool first(true);
	
	while (e) {
		if (!first)
			ostr << std::endl << " -> ";
			
		ostr << e->left() << "/" << e->right();
		first = false;
		e = e->getRightEdge();
	}
		
	return ostr;
}

std::ostream& operator<<(std::ostream& ostr, LineStripEdgeGroup& edgegroup)
{
	LineStripEdge* edge = edgegroup.getStartEdge();
	if (edge) {
		ostr << "start: " << (*edge) << std::endl;
		while ((edge = edgegroup.getNextEdge()))
		{
			ostr << "loose: " << (*edge) << std::endl;
		}
	} else {
		ostr << "0x0" << std::endl;
	}
	return ostr;


}
}