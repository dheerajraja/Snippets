/* Generated by Together */

#ifndef CELEMENT_H
#define CELEMENT_H
#include "CNode.h"

/**
 * Class that contains the attributes and methods that enables to store the
 * nested elements within an XML text format. This is a child class of CNode.
 */
class CElement: public CNode
{
public:

	/**
	 * Constructor that initializes the member attributes and initializes the parent constructor
	 */
	CElement();

	/**
	 * Destructor that frees the allocated dynamic memory for the child objects created
	 */
	~CElement();

	/**
	 * This method consumes all characters of an XML string, creates a text object
	 * upon detection of text, creates an new element object upon detection of  a new
	 * child element and continues until the end of  a tag is detected.
	 * @param: Input: contains the input XML parser string data.
	 * @param: parsePosition:Current position from which the XML data (string)
	 * has to be parsed.
	 * @Return: True is returned upon successful parsing of the entire
	 * element and if not a false is returned indicating parsing error.
	 */
	bool parseInput(const string& Input, unsigned& parsePosition);
	/**
	 * This method is used to print the entire XML string data stored in the
	 * datastructure in the right format.
	 * @Param: indent: provides the no. of times the contents in the datastrcuture
	 *  has to be indented for the right display format.
	 * @return: None
	 */
	void print(int indent);
	/**
	 *This method returns the node type by obtaining the node type from the parent
	 *the class CNode using getNodeType()  of the parent class CNode
	 * @Param: None
	 * @return: Node type that indicates whether it is element type or text type.
	 */
	node_t getNodeType();

private:
	/**
	 * This method is used to add the child nodes within an element
	 * into its own datastructure.
	 * @param:child: holds the address of the new child object created(element or text).
	 * @param: bool:returns true if the child was successfully added or else return false.
	 */
	bool addToContent(CNode* child);
	/**
	 * This method  consumes a complete start or end tag
	 * including the opening “<” or “</” and the terminating “>” and provides information
	 * as whether it is start or end of a string and also tells whether the start and
	 * end tag provided for an element are good or bad.
	 * @Param: input contains the input XML parser string data.
	 * @param: parsePosition:Current position from which the XML data (string)
	 * has to be parsed.
	 * @Param: IsStartTag:Returns true upon detection of start tag and false upon detection of
	 * endtag.
	 * @Param: tag: Stores the tag upon detecting a start tag.
	 * @return: bool: Returns true if the tag was good tag or if the endtag succesfully
	 * matched the start tag and returns false if a bad tag is detected and also if the
	 * end tag of an element failed to match the start tag.
	 */
	bool parseStartOrEndTag(const string& Input, unsigned& parsePosition,
			bool& IsStartTag, string& tag);

	/**
	 * @link Composition
	 * @supplierCardinality 0..10
	 * Creates a data structure for each element within an xml data format.
	 * each element can further contain child elements and child texts.
	 * This attribute helps in forming the ttree like datastrcuture.
	 */
	CNode* m_content[10];
	/**
	 * Holds the current no. of child nodes within an element.
	 */
	int m_contentNodes;
	/**
	 * holds the correct string of an element.
	 */
	string m_tag;
};
#endif //CELEMENT_H