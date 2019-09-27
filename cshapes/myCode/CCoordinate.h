/*
 * CCoordinate.h
 *
 *  Created on: 17.10.2013
 *      Author: Dheeraj Guptha
 */

#ifndef CCOORDINATE_H_
#define CCOORDINATE_H_

#include <ostream>
//#include <iostream>
//using namespace std;
using namespace std;
namespace GraSys
{

template<class T>
class CCoordinate
{
	/**
	 *Friend function for adding 2 co-ordinates
	 *@param:c1 is the object to the left of + operator
	 *@param:c2 is the object to the left of + operator
	 *@return: c1+c2
	 */
	template<class U>
	friend CCoordinate<U> operator+(const CCoordinate<U>& c1,
			const CCoordinate<U>& c2);
	/**
	 *Friend function for printing co-ordinates objects
	 *@param:ostream object which outputs the object on the terminal
	 *@param:c is the object to be printed
	 *@return:ostream object is returned to provide the next output to the stream
	 */
	template<class U>
	friend ostream& operator<<(ostream& out, const CCoordinate<U>& c);
	/**
	 *Friend function for checking if 2 co-ordinates are equal
	 *@param:c1 is the object to the left of != operator
	 *@param:c2 is the object to the left of != operator
	 *@return: true if c1!=c2
	 */
	template<class U>
	friend bool operator!=(const CCoordinate<U>& c1, const CCoordinate<U>& c2);

private:
	/**
	 * holds the x and y cartesian data
	 */
	T m_x;
	T m_y;

public:
	enum t_quadrant
	{
		Q1, Q2, Q3, Q4
	};
	/**
	 * Parameterized constructor for cartesian co-ordinates initialization
	 */
	CCoordinate(T initialX = 0, T initialY = 0);

	/**
	 * Parameterized constructor for qudrant initialization
	 */
	CCoordinate(t_quadrant quadrant);

	/**
	 * copy constructor
	 */
	CCoordinate(const CCoordinate& orig);

	/*
	 * Destructor
	 */
	~CCoordinate();
	/**
	 *Method that sets the cartesian co-ordinates
	 *@param:x initializes x-coordinate of the object
	 *@param:y initializes y-coordinate of the object
	 */
	void setCartesian(T x, T y);

	/**
	 *Method that retrieves the cartesian co-ordinates
	 *@param:x returns x-coordinate of the object to the caller
	 *@param:y returns y-coordinate of the object to the caller
	 */
	void getCartesian(T& x, T& y);
	/**
	 *Method that retrieves the x co-ordinate
	 *@param:none
	 *@return:returns x co-ordinate of the given template type
	 */
	T getX() const;
	/**
	 *Method that retrieves the y co-ordinate
	 *@param:none
	 *@return:returns y co-ordinate of the given template type
	 */
	T getY() const;
};

template<class T>
CCoordinate<T>::CCoordinate(T initialX, T initialY)
{
//	cout << "Initializing object at " << this << endl;
	m_x = initialX;
	m_y = initialY;
}

template<class T>
CCoordinate<T>::CCoordinate(t_quadrant quadrant)
{
	switch (quadrant)
	{
	case Q1:
		m_x = 1;
		m_y = 1;
		break;
	case Q2:
		m_x = -1;
		m_y = 1;
		break;
	case Q3:
		m_x = -1;
		m_y = -1;
		break;
	case Q4:
		m_x = 1;
		m_y = -1;
		break;
	}
}

template<class T>
CCoordinate<T>::CCoordinate(const CCoordinate<T>& orig)
{
//	cout << "Initializing object (copy) at " << this << endl;
	m_x = orig.m_x;
	m_y = orig.m_y;
}

template<class T>
CCoordinate<T>::~CCoordinate()
{
//	cout << "Destructing object at " << this << endl;
}

template<class T>
void CCoordinate<T>::setCartesian(T x, T y)
{
	m_x = x;
	m_y = y;
}

template<class T>
T CCoordinate<T>::getX() const
{
	return m_x;
}

template<class T>
T CCoordinate<T>::getY() const
{
	return m_y;
}

template<class T>
void CCoordinate<T>::getCartesian(T& x, T& y)
{
	//cout << "Address of parameter x: " << &x << endl;
	x = m_x;
	y = m_y;
}

template<class T>
CCoordinate<T> operator+(const CCoordinate<T>& c1, const CCoordinate<T>& c2)
{
	CCoordinate<T> result(c1.m_x + c2.m_x, c1.m_y + c2.m_y);
	return result;
}

template<class T>
ostream& operator<<(ostream& out, const CCoordinate<T>& c)
{
	out << "[" << c.m_x << "," << c.m_y << "]";
	return out;
}

template<class T>
bool operator!=(const CCoordinate<T>& c1, const CCoordinate<T>& c2)
{
	return ((c1.m_x != c2.m_x) || (c1.m_y != c2.m_y));
}

/*CCoordinate CCoordinate::operator=(const CCoordinate& c)
 {
 m_x = c.m_x;
 m_y = c.m_y;
 return *this;
 }*/

} /* namespace GraSys */

#endif /* CCOORDINATE_H_ */
