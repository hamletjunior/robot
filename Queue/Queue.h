/*
 * Queue.h
 *
 *  Created on: 21/01/2014
 *      Author: hamletpessoafariasjunior
 */

#ifndef QUEUE_H_
#define QUEUE_H_
#include "Node.h"
#define MAX_QUEUE_SIZE 15

class Queue {
	Node *first;
	Node *last;
public:
	Queue();
	void addValue(int value);
	int size();
	int maxVal();
	int minVal();
};

#endif /* QUEUE_H_ */
