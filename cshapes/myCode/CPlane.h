/*
 * CScreen.h
 *
 *  Created on: 21 Nov 2013
 *      Author: DheerajGuptha
 */

#ifndef CPLANE_H_
#define CPLANE_H_

#include "CGraphicElement.h"
#include "CRectangle.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <stdlib.h>
#include <cmath>

using namespace std;

namespace GraSys
{

template<class T>
class CPlane
{
private:
	//CGraphicElement* m_shapes;
	vector<CGraphicElement<T>*> m_shapes;

public:
	CPlane();
	/**
	 * Adds a valid element to the plane, else throws an error.
	 * @param: element to be added to the plane
	 * @return:true if valid element is added
	 */
	bool addElement(CGraphicElement<T>& element);

	/**
	 * Return the bounding box for all elements on the plane that match
	 * the given criteria.
	 *
	 * @param type take only shapes with the given type into account
	 * (use all shapes if type == "").
	 * @param color take only shapes with the given color into account
	 * (use all shapes if color == "").
	 */
	CRectangle<T> boundingBox(string type = "", string color = "");
	/**
	 * Displays all the elements on the screen
	 * @param: none
	 */
	void printScreen();
};

template<class T>
CPlane<T>::CPlane()
{

}

template<class T>
bool CPlane<T>::addElement(CGraphicElement<T>& element)
{
	CCoordinate<T> coord1;
	CCoordinate<T> coord2;
	CCoordinate<T> coord3;

	//If radius is zero it is not a valid element
	if (element.getTypeName() == "Circle")
	{
		coord1 = element.getCoordinate(0);
		coord2 = element.getCoordinate(1);
		if (coord1.getX() == coord2.getX())
		{
			cerr
					<< "Circle cannot have zero radius:element cannot be added to the plane"
					<< endl;
			return false;
		}
		else
		{
			m_shapes.push_back(&element);

		}

	}

	//If both the co-ordinates of the rectangle are lying on same line then a rectangle cannot be formed
	if (element.getTypeName() == "Rectangle")
	{
		coord1 = element.getCoordinate(0);
		coord2 = element.getCoordinate(1);
		if ((coord1.getX() == coord2.getX())
				|| (coord1.getY() == coord2.getY()))
		{
			cerr
					<< "Rectangle cannot be formed: element cannot be added to the plane"
					<< endl;
			return false;
		}
		else
		{
			m_shapes.push_back(&element);

		}

	}

	//If the 3rd co-ordinate lies on the line joining the 1st 2 co-ordinates then triangle cannot be formed
	if (element.getTypeName() == "Triangle")
	{
		coord1 = element.getCoordinate(0);
		coord2 = element.getCoordinate(1);
		coord2 = element.getCoordinate(2);

		if ((coord1.getX() == coord2.getX())
				&& (coord1.getX() == coord3.getX()))//line parallel to y-axis:slope infinity
		{
			cerr
					<< "Triangle cannot be formed: element cannot be added to the plane"
					<< endl;
			return false;
		}

		T m1 = (coord1.getY() - coord2.getY())
				/ (coord1.getX() - coord2.getX());
		T m2 = (coord1.getY() - coord3.getY())
				/ (coord1.getX() - coord3.getX());

		if (m1 < 0)
		{
			m1 = m1 * -1;
		}

		if (m2 < 0)
		{
			m2 = m2 * -1;
		}

		if (m1 == m2)// checking if slopes of the line from coord1 to coord2 and coord1 to coord3 are same
		{
			cerr
					<< "Triangle cannot be formed: element cannot be added to the plane"
					<< endl;
			return false;
		}
		else
		{
			m_shapes.push_back(&element);

		}

	}

	return true;// if a valid element is added
}

template<class T>
CRectangle<T> CPlane<T>::boundingBox(string type, string color)
{

	unsigned numofcoord = 0;
	bool error = false, found = false;
	vector<T> my_y;
	vector<T> my_x;
	CCoordinate<T> my_coord;
	unsigned i = 0, j = 0;
	T min_x_shape, max_x_shape;
	T min_y_shape, max_y_shape;

	if (type == "Rectangle")
	{
		numofcoord = 2;
	}
	if (type == "Triangle")
	{
		numofcoord = 3;
	}
	if (type == "Circle")
	{
		numofcoord = 5;
	}

	for (i = 0; i < m_shapes.size(); i++)//for each element
	{
		if ((type == "") && (color == ""))
		{
			if (m_shapes[i]->getTypeName() == "Circle")
			{
				numofcoord = 5;
			}
			if (m_shapes[i]->getTypeName() == "Rectangle")
			{
				numofcoord = 2;
			}
			if (m_shapes[i]->getTypeName() == "Triangle")
			{
				numofcoord = 3;
			}

			for (j = 0; j < numofcoord; j++)// for co-ordinates of ith element
			{
				found = true;
				my_coord = (m_shapes[i]->getCoordinate(j));
				my_x.push_back(my_coord.getX());
				my_y.push_back(my_coord.getY());

			}

		}

		if ((type != "") && (color == ""))
		{
			if (m_shapes[i]->getTypeName() == type)
			{

				for (j = 0; j < numofcoord; j++)
				{
					found = true;
					my_coord = (m_shapes[i]->getCoordinate(j));
					my_x.push_back(my_coord.getX());
					my_y.push_back(my_coord.getY());

				}
			}

		}

		if ((type == "") && (color != ""))
		{
			if (m_shapes[i]->getTypeName() == "Circle")
			{
				numofcoord = 5;
			}
			if (m_shapes[i]->getTypeName() == "Rectangle")
			{
				numofcoord = 2;
			}
			if (m_shapes[i]->getTypeName() == "Triangle")
			{
				numofcoord = 3;
			}
			if (m_shapes[i]->getColor() == color)
			{

				for (j = 0; j < numofcoord; j++)
				{
					found = true;
					my_coord = (m_shapes[i]->getCoordinate(j));
					my_x.push_back(my_coord.getX());
					my_y.push_back(my_coord.getY());

				}
			}
		}

		if ((type != "") && (color != ""))
		{

			if ((m_shapes[i]->getTypeName() == type)
					&& (m_shapes[i]->getColor() == color))
			{
				for (j = 0; j < numofcoord; j++)
				{
					found = true;
					my_coord = (m_shapes[i]->getCoordinate(j));
					my_x.push_back(my_coord.getX());
					my_y.push_back(my_coord.getY());

				}

			}

		}

	}

	if (found == false)
	{
		min_x_shape = 0;
		max_x_shape = 0;
		min_y_shape = 0;
		max_y_shape = 0;
		error = true;
		cerr
				<< "Could not find the specified element in the plane and assigning the plane NULL (0,0)"
				<< endl;
		cerr << " Could not draw the bounding box" << endl;
	}

	if (error == false)
	{
		std::sort(my_x.begin(), my_x.end());
		min_x_shape = my_x[0];// min of all x-axis co-ordinates
		max_x_shape = my_x[(my_x.size() - 1)];// min of all y-axis co-ordinates

		std::sort(my_y.begin(), my_y.end());
		min_y_shape = my_y[0];// max of all y-axis co-ordinates
		max_y_shape = my_y[(my_y.size() - 1)];// max of all y-axis co-ordinates
	}

	CRectangle<T> myplane(color, CCoordinate<T>(min_x_shape, min_y_shape),
			CCoordinate<T>(max_x_shape, max_y_shape));// bounding box formed by min and max co-ordinates

	//addElement(myplane);// adding this to as an element to access in printscreen()

	return myplane;

}

template<class T>
void CPlane<T>::printScreen()
{
	/*int i = (m_shapes.size() - 1);
	CCoordinate<T> coord1 = m_shapes[i]->getCoordinate(0);
	CCoordinate<T> coord2 = m_shapes[i]->getCoordinate(1);

	cout << "My screen is rectangle bounded by coordinates: " << coord1
			<< " and " << coord2 << endl;*/

	cout << "My screen has the following elements" << endl;

	unsigned numofcoord;
	CCoordinate<T> my_coord;
	T radius;
	T centrex;
	for (int j = 0; j < m_shapes.size() ; j++)
	{
		cout << "Element" << j + 1 << ":" << m_shapes[j]->getTypeName() << "["
				<< m_shapes[j]->getColor() << "]" << "with: " << endl;
		if (m_shapes[j]->getTypeName() == "Circle")
		{
			numofcoord = 2;
		}
		if (m_shapes[j]->getTypeName() == "Rectangle")
		{
			numofcoord = 2;
		}
		if (m_shapes[j]->getTypeName() == "Triangle")
		{
			numofcoord = 3;
		}

		for (int k = 0; k < numofcoord; k++)
		{

			my_coord = (m_shapes[j]->getCoordinate(k));
			if ((m_shapes[j]->getTypeName() == "Circle"))
			{
				if (k == 0)
				{
					centrex = my_coord.getX();
					cout << "centre =" << m_shapes[j]->getCoordinate(k);
				}
				if (k == 1)
				{
					radius = my_coord.getX() - centrex;
					cout << " and radius =" << radius << endl;
				}

			}

			else
			{
				cout << "coordinate" << "[" << k + 1 << "] = " << my_coord
						<< endl;

			}

		}

	}

}
}
#endif /* CPLANE_H_ */
