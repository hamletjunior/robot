/*
 * Queue.cpp
 *
 *  Created on: 21/01/2014
 *      Author: hamletpessoafariasjunior
 */

#include <Queue/Queue.h>
#include <stdio.h>
#include <stdlib.h>

Queue::Queue() {
	this->first = NULL;
	this->last = NULL;
}

void Queue::addValue(int value) {
	Node *node = new Node(value);
	if (this->first == NULL) {
		this->first = node;
		this->last = node;
	} else {
		this->last->next = node;
		this->last = node;
	}
	if (this->size() > MAX_QUEUE_SIZE) {
		Node *nodeTemp = this->first;
		this->first = this->first->next;
		nodeTemp->next = NULL;
		free(nodeTemp);
	}
}

int Queue::size() {
	if (this->first == NULL) {
		return 0;
	}

	int i = 1;
	Node *iterator = this->first;
	while (iterator->next != NULL) {
		i++;
		iterator = iterator->next;
	}
	return i;
}

int Queue::maxVal(){
	if (this->first == NULL) {
		return 0;
	}

	int maximum = this->first->value;
	Node *iterator = this->first;
	while (iterator->next != NULL) {
		iterator = iterator->next;
		if (iterator->value > maximum){
			maximum = iterator->value;
		}
	}
	return maximum;
}

int Queue::minVal(){
	if (this->first == NULL) {
		return 0;
	}

	int minimum = this->first->value;
	Node *iterator = this->first;
	while (iterator->next != NULL) {
		iterator = iterator->next;
		if (iterator->value < minimum){
			minimum = iterator->value;
		}
	}
	return minimum;
}
