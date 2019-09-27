#include <iostream>
#include <stdlib.h>
#include <iomanip>

using namespace std;

#include "CCoordinate.h"
#include "CRectangle.h"
#include "CTriangle.h"
#include "CPlane.h"
#include "CGraphicElement.h"
#include "CCircle.h"

using namespace GraSys;

int main(void)
{
	cout << "Shapes started." << endl << endl;

	cout << "Test cases for move method" << endl;
	cout << setfill('-') << setw(80) << "-" << endl;
	CRectangle<int> rect("blue", CCoordinate<int>(1, 1),
			CCoordinate<int>(2, 2));
	CCircle<int> circle("red", CCoordinate<int>(2, 1), 3);

	CTriangle<int> tri("purple", CCoordinate<int>(1, 1), CCoordinate<int>(2, 0),
			CCoordinate<int>(3, 2));
	bool failed = false;
	cout << "Co-ordinates before move" << endl;
	rect.print();
	circle.print();
	tri.print();

	rect.move(1, 2);
	circle.move(2, 1);
	tri.move(1, 3);

	cout << "Co-ordinates after move" << endl;
	rect.print();
	circle.print();
	tri.print();

	if ((rect.getCoordinate(0) != CCoordinate<int>(2, 3) //(2,3)//(3,4)
	|| rect.getCoordinate(1) != CCoordinate<int>(3, 4)))

	{
		cerr << "Testcase move rectangle failed" << endl;
		failed = true;
	}

	if (circle.getCoordinate(0) != CCoordinate<int>(4, 2))
	{
		cerr << "Testcase move circle failed" << endl;
		failed = true;
	}

	if ((tri.getCoordinate(0) != CCoordinate<int>(1, 4)) //(2,3)//(3,4)
	|| (tri.getCoordinate(1) != CCoordinate<int>(3, 3))
			|| (tri.getCoordinate(2) != CCoordinate<int>(4, 5)))
	{
		cerr << "Testcase move Triangle failed" << endl;
		failed = true;
	}

	if (failed)
	{
		cerr << "Some test cases failed" << endl << endl;
	}
	else
	{
		cout << "All test cases completed successfully" << endl << endl;
	}

	cout << "Test cases for add element" << endl;
	cout << setfill('-') << setw(80) << "-" << endl;

	CRectangle<int> r("yellow", CCoordinate<int>(1, 1), CCoordinate<int>(1, 2));
	r.print();
	CTriangle<int> t("red", CCoordinate<int>(4, 4), CCoordinate<int>(5, 6),
			CCoordinate<int>(6, 4));
	CCircle<int> c("green", CCoordinate<int>(1, 6), 2);

	CPlane<int> myplane;

	bool addele1;

	addele1 = myplane.addElement(r);
	if (addele1 == true)
	{
		cout << "Rectangle added successfully" << endl;
	}
	else
	{
		cout << "Adding Rectangle was not successful" << endl;
	}
	addele1 = myplane.addElement(t);
	if (addele1 == true)
	{
		cout << "Triangle added successfully" << endl;
	}
	else
	{
		cout << "Adding Triangle was not successful" << endl;
	}
	addele1 = myplane.addElement(c);
	if (addele1 == true)
	{
		cout << "Circle added successfully" << endl;
	}
	else
	{
		cout << "Adding Circle was not successful" << endl;
	}

	myplane.printScreen();

	cout << endl;
	cout << setfill('-') << setw(80) << "-" << endl;
	cout << "Test cases for the bounding box on screen!" << endl;
	cout << setfill('-') << setw(80) << "-" << endl;

	//CRectangle<int> compscreen = myplane.boundingBox("", "");
	//CRectangle<int> compscreen = myplane.boundingBox("Rectangle", "");
	CRectangle<int> compscreen = myplane.boundingBox("", "indigo");
	//CRectangle<int> compscreen = myplane.boundingBox("Triangle", "red");

	if ((compscreen.getCoordinate(0) != CCoordinate<int>(0, 0))
			&& (compscreen.getCoordinate(0) != CCoordinate<int>(0, 0)))
	{
		cout << "minimum bounding co-ordinates of my plane is "
				<< compscreen.getCoordinate(0) << endl;
		cout << "maximum bounding co-ordinates of my plane is "
				<< compscreen.getCoordinate(1) << endl;
	}

	else
	{
		cout << "Sorry!! No bounding box" << endl;
	}
	/**
	 * Testing floating point inputs for mathematical plane
	 */
	cout << endl;
	cout << setfill('-') << setw(80) << "-" << endl;
	cout << "Test cases for Mathematical plane" << endl;
	cout << setfill('-') << setw(80) << "-" << endl;
	cout << endl;
	cout << setfill('-') << setw(80) << "-" << endl;
	cout << "Test cases for move method" << endl;
	cout << setfill('-') << setw(80) << "-" << endl;
	CCircle<float> circlef("green", CCoordinate<float>(0.1, 0.1), 0.1);
	//circlef.print();
	CRectangle<float> rectf("blue", CCoordinate<float>(0.1, 0.1),
			CCoordinate<float>(0.2, 0.2));

	CTriangle<float> trif("purple", CCoordinate<float>(0.1, 0.1),
			CCoordinate<float>(0.2, 0), CCoordinate<float>(0.3, 0.2));
	bool failed1 = false;
	cout << "Co-ordinates before move" << endl;
	rectf.print();
	circlef.print();
	trif.print();

	rectf.move(0.1, 0.2);
	circlef.move(0.1, 0.1);
	trif.move(0.1, 0.3);

	cout << "Co-ordinates after move" << endl;
	rectf.print();
	circlef.print();
	trif.print();

	if ((rectf.getCoordinate(0) != CCoordinate<float>(0.2, 0.3) //(0.2,0.3)//(0.3,0.4)
	|| rectf.getCoordinate(1) != CCoordinate<float>(0.3, 0.4)))

	{
		cout << "Testcase move Rectangle failed" << endl;
		failed1 = true;
	}

	if (circlef.getCoordinate(0) != CCoordinate<float>(0.2, 0.2))
	{
		cout << "Testcase move Circle failed" << endl;
		failed1 = true;
	}

	if ((trif.getCoordinate(0) != CCoordinate<float>(0.2, 0.4) //(2,3)//(3,4)
	|| trif.getCoordinate(1) != CCoordinate<float>(0.3, 0.3)
			|| trif.getCoordinate(2) != CCoordinate<float>(0.4, 0.5)))
	{
		cout << "Testcase move Triangle failed" << endl;
		failed1 = true;
	}

	if (failed1)
	{
		cout << "Some test cases failed" << endl << endl;
	}
	else
	{
		cout << "All test cases completed successfully" << endl << endl;
	}

	cout << "Test cases for add element" << endl;
	cout << setfill('-') << setw(80) << "-" << endl;
	CCircle<float> cf("green", CCoordinate<float>(0.1, 0.6), 0.2);
	cf.print();
	CRectangle<float> rf("blue", CCoordinate<float>(0.1, 0.1),
			CCoordinate<float>(0.2, 0.2));

	CTriangle<float> tf("red", CCoordinate<float>(0.4, 0.4),
			CCoordinate<float>(0.5, 0.6), CCoordinate<float>(0.6, 0.4));
	CPlane<float> myplanef;

	bool addele;

	addele = myplanef.addElement(rf);
	if (addele == true)
	{
		cout << "Rectangle added successfully" << endl;
	}
	else
	{
		cout << "Adding Rectangle was not successful" << endl;
	}
	addele = myplanef.addElement(tf);
	if (addele == true)
	{
		cout << "Triangle added successfully" << endl;
	}
	else
	{
		cout << "Adding Triangle was not successful" << endl;
	}
	addele = myplanef.addElement(cf);
	if (addele == true)
	{
		cout << "Circle added successfully" << endl;
	}
	else
	{
		cout << "Adding Circle was not successful" << endl;
	}

	myplanef.printScreen();

	cout << endl;
	cout << "Test cases for the bounding box on mathematical screen!" << endl;
	cout << setfill('-') << setw(80) << "-" << endl;

	CRectangle<float> mathematical = myplanef.boundingBox("", "");
	//CRectangle<int> compscreen = myplanef.boundingBox("Circle", "");
	//CRectangle<int> compscreen = myplanef.boundingBox("", "yellow");
	//CRectangle<int> compscreen = myplanef.boundingBox("Triangle", "red");


	 if ((mathematical.getCoordinate(0) != CCoordinate<float>(0, 0))
	 && (mathematical.getCoordinate(0) != CCoordinate<float>(0, 0)))
	 {

	 cout << "minimum bounding co-ordinates of my plane is "
	 << mathematical.getCoordinate(0) << endl;
	 cout << "maximum bounding co-ordinates of my plane is "
	 << mathematical.getCoordinate(1) << endl;
	 }
	 else
	 {
	 cout<<"Sorry!! No bounding box!"<<endl;

	 }

	return 0;
}
