/* Generated by Together */

#ifndef CCIRCLE_H
#define CCIRCLE_H

#include <string>
#include <iostream>

using namespace std;

#include "CGraphicElement.h"

namespace GraSys
{

template<class T>
class CCircle: public CGraphicElement<T>
{
private:
	/**
	 * Holds the radius of the circle
	 */
	T m_radius;

protected:
	/**
	 * Holds the radius of the circle
	 */
	virtual string getTypeName();

public:
	// CCircle(std::string color);
	/**
	 * @brief: parameterized constructor used to initialize the characteristics of circle.
	 * @Param:color specifies the element color
	 * @Param:corner1 specifies the circle centre
	 * @Param:radius specifies the circle radius
	 * @Return:none
	 */
	CCircle(string color, const CCoordinate<T>& corner1, T radius);
	/**
	 * @brief: specific print function for circle: to print centre and radius
	 * @Param:none
	 * @Return:none
	 */
	void print();
	/**
	 * @brief: virtual destructor that indicates to the compiler that when memory
	 * freeing for parent object is done even child class object memory is freed.
	 * @Param:none
	 * @Return:none
	 */
	virtual ~CCircle();
	//string getTypeName();
};

template<class T>
CCircle<T>::CCircle(string color, const CCoordinate<T>& corner1, T radius) :
		CGraphicElement<T>("Circle", 5, color), m_radius(radius)
{
	T x, y;
	this->m_coordinates.push_back(corner1);//storing the centre
	x = corner1.getX() + radius;
	y = corner1.getY();
	CCoordinate<T> circumference;
	circumference.setCartesian(x, y);
	this->m_coordinates.push_back(circumference);//storing ((centrex+radius),y)
	x = corner1.getX() - radius;
	y = corner1.getY();
	circumference.setCartesian(x, y);
	this->m_coordinates.push_back(circumference);//storing ((centrex-radius),y)
	x = corner1.getX();
	y = corner1.getY() + radius;
	circumference.setCartesian(x, y);
	this->m_coordinates.push_back(circumference);//storing ((centrex-radius),y)
	x = corner1.getX();
	y = corner1.getY() - radius;
	circumference.setCartesian(x, y);
	this->m_coordinates.push_back(circumference);//storing ((centrex-radius),y)

	//m_radius = radius;

}

template<class T>
void CCircle<T>::print()
{
	cout << "Circle with center: [" << this->getColor() << "] ["
			<< this->m_coordinates[0].getX() << ","
			<< this->m_coordinates[0].getY() << "] and radius = " << m_radius
			<< endl;
	//cout<<"The color of circle is"<<this->getColor()<<endl;

}

template<class T>
string CCircle<T>::getTypeName()
{
	return "Circle";
}

template<class T>
CCircle<T>::~CCircle()
{
//	cout << "Destructing object at " << this << endl;
}

}

#endif //CCIRCLE_H