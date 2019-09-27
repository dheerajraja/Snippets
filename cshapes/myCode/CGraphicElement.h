/*
 * CGraphicElement.h
 *
 *  Created on: 7 Nov 2013
 *      Author: Dheeraj Guptha
 */

#ifndef CGRAPHICELEMENT_H_
#define CGRAPHICELEMENT_H_

#include <string>
#include <vector>
#include <iostream>
using namespace std;
#include "CCoordinate.h"

namespace GraSys
{
template<class T>
class CGraphicElement
{

private:
	/*
	 * holds the type of the graphic element(triangle,rectangle,circle)
	 */
	string m_type;

	/*
	 * holds the color of the Graphic element
	 */
	string m_color;

protected:

	/*
	 * holds the number of co-ordinates for a graphic element
	 */
	int m_numberCoordinates;

	/*
	 * holds all the co-ordinates of a particular graphic element
	 */
	vector<CCoordinate<T> > m_coordinates;

public:

	/**
	 * Paramtrised constructor for initialising with the particular graphic element data
	 * @param:type initializes weather the graphic element is triangle,rectangle or circle
	 * @param:numberOfCoordinates initializes the number of co-ordinates that forms the shape
	 * @param:color initializes the color of the shape
	 */
	CGraphicElement(string type, int numberOfCoordinates, string color);
	/**
	 * This method moves the co-ordinates by specified offset
	 * @param:xoffset offsets m_x
	 * @param:yoffset offsets m_y
	 * @return:none
	 */
	void move(T xoffset, T yoffset);
	/**
	 * This method prints the graphicelement [color] with the co-ordinates
	 * @param:none
	 * @return:none
	 */
	void print();

	/**
	 * retrieves the ith co-ordinate of an element
	 * @param:index of the co-ordinate to be accessed
	 * @return:the co-ordinate to be retrived
	 */
	const CCoordinate<T> getCoordinate(unsigned int index);
	/**
	 * retrieves the type of shape of an object.This is delegated to the child with the help of virtual keyword.
	 * @param:None
	 * @return:string indicating the type of object
	 */
	virtual string getTypeName()=0;

	/**
	 * retrives the color of the graphicelement
	 * @param:None
	 * @return:string indicating the color
	 */
	string getColor();

	/**
	 * Virtual destructor indicating the compiler that the memory allocated for the child
	 * elements should also be destroyed before the parent
	 * @param:None
	 * @return:None
	 */
	virtual ~CGraphicElement();
};

template<class T>
CGraphicElement<T>::CGraphicElement(string type, int numberOfCoordinates,
		string color)
{
	m_type = type;
	m_color = color;
	m_numberCoordinates = numberOfCoordinates;
	//m_coordinates = new CCoordinate[m_numberCoordinates];
}

template<class T>
void CGraphicElement<T>::CGraphicElement::move(T xoffset, T yoffset)
{
	for (int i = 0; i < m_numberCoordinates; i++)
	{
		T x, y;
		m_coordinates[i].getCartesian(x, y);
		x += xoffset;
		y += yoffset;
		m_coordinates[i].setCartesian(x, y);
	}
}

template<class T>
void CGraphicElement<T>::print()
{
	cout << m_type << "(color: " << m_color << ") with coordinates: ";
	for (int i = 0; i < m_numberCoordinates; i++)
	{
		cout << "[" << m_coordinates[i].getX() << "," << m_coordinates[i].getY()
				<< "]";
		if (i < m_numberCoordinates - 1)
		{
			cout << ", ";
		}
	}
	cout << endl;
}

template<class T>
const CCoordinate<T> CGraphicElement<T>::getCoordinate(unsigned int index)
{

	return m_coordinates[index];
}

template<class T>
string CGraphicElement<T>::getColor()
{
	return m_color;
}

template<class T>
CGraphicElement<T>::~CGraphicElement()
{

}

}
#endif /* CGRAPHICELEMENT_H_ */
