/* Generated by Together */

#include "CNode.h"

CNode::CNode(node_t nodeType)// initialized by the constructor of child class
{
	m_nodeType = nodeType;

}

node_t CNode::getNodeType()
{
	return m_nodeType;
}
