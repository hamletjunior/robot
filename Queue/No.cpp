/*
 * Node.cpp
 *
 *  Created on: 21/01/2014
 *      Author: hamletpessoafariasjunior
 */

#include <Queue/Node.h>
#include <stdio.h>

Node::Node(int value) {
	this->next = NULL;
	this->value = value;
}

