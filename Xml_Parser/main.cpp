// GIT-Labor
// main.h

////////////////////////////////////////////////////////////////////////////////
// Header-Dateien
#include <iostream>		// Header für die Standard-IO-Objekte (z.B. cout, cin)
#include <stdlib.h>

// TODO: Fügen Sie hier weitere benötigte Header-Dateien der
// Standard-Bibliothek ein z.B.
// #include <string>

//using namespace std;
// Erspart den scope vor Objekte der
// C++-Standard-Bibliothek zu schreiben
// z.B. statt "std::cout" kann man "cout" schreiben

// Inkludieren Sie hier die Header-Files Ihrer Klassen, z.B.
#include "CElement.h"
//<nestagain>\n  yet" "again </nestagain>

// Hauptprogramm
// Dient als Testrahmen, von hier aus werden die Klassen aufgerufen
int main(void)
{
	// TODO: Fügen Sie ab hier Ihren Programmcode ein
	cout << "XMLparser gestarted." << endl << endl;

	unsigned pos = 0;
	CElement doc;
	/*string tag;
	 bool isstart;*/
	/*//doc.parseStartOrEndTag("...</hi>...",pos,isstart,tag);
	 //cout<<pos<<endl;
	 //cout<<isstart<<endl;
	 //cout<<tag<<endl;*/

	/*	if (!doc.parseInput("<top>\n  <nested>some data<@style>some datat1</style>text2</nested><element1>asdf</element1>text<a>asd</a>asdf</top>", pos)) {
	 cout << "Parsing failed at position " << pos << endl;
	 } else {
	 doc.print(0);
	 }*/
//	if (!doc.parseInput("<top>\n <nested>\n<test2>sadf</test2>  some data</nested>\n more data\n <ok1more> ok </ok1more>\n no more data</top>", pos)) {
//	cout << "Parsing failed at position " << pos << endl;
//	}
//	else {
//		cout<<"check"<<endl;
//	doc.print(0);
//	}
	/*
	 if (!doc.parseInput("<top>\n <nested>some "
	 "data</nested1>\n more data\n</top>", pos)) {
	 cout << "Parsing failed at position " << pos << endl;
	 } else {
	 doc.print(0);
	 }*/

	if (!doc.parseInput(
			"<mylifestyle>\n Active and fresh <Adventure>\n Life's exciting <Sports>sweat it out <Badminton> goal:win always<Racket> yonex is a must </Racket></Badminton></Sports><Marathon> Healthy and fun filled life </Marathon><Trekking> Frankenstein castle </Trekking> Adventure keeps life going </Adventure> fun lifestyle </mylifestyle>",
			pos))
	{
		cout << "Parsing failed at position " << pos << endl;
	}
	else
	{
		doc.print(0);
	}
	return 0;
}
