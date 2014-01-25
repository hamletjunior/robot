/*
 * Fila.cpp
 *
 *  Created on: 21/01/2014
 *      Author: hamletpessoafariasjunior
 */

#include <Fila/Fila.h>
#include <stdio.h>
#include <stdlib.h>

Fila::Fila() {
	this->primeiro = NULL;
	this->ultimo = NULL;
}

void Fila::adicionaValor(int valor) {
	No *no = new No(valor);
	if (this->primeiro == NULL) {
		this->primeiro = no;
		this->ultimo = no;
	} else {
		this->ultimo->proximo = no;
		this->ultimo = no;
	}
	if (this->tamanho() > TAMANHO_MAXIMO_FILA) {
		No *noTemp = this->primeiro;
		this->primeiro = this->primeiro->proximo;
		noTemp->proximo = NULL;
		free(noTemp);
	}
}

int Fila::tamanho() {
	if (this->primeiro == NULL) {
		return 0;
	}

	int i = 1;
	No *iterador = this->primeiro;
	while (iterador->proximo != NULL) {
		i++;
		iterador = iterador->proximo;
	}
	return i;
}

int Fila::maximo(){
	if (this->primeiro == NULL) {
		return 0;
	}

	int maximo = this->primeiro->valor;
	No *iterador = this->primeiro;
	while (iterador->proximo != NULL) {
		iterador = iterador->proximo;
		if (iterador->valor > maximo){
			maximo = iterador->valor;
		}
	}
	return maximo;
}

int Fila::minimo(){
	if (this->primeiro == NULL) {
		return 0;
	}

	int minimo = this->primeiro->valor;
	No *iterador = this->primeiro;
	while (iterador->proximo != NULL) {
		iterador = iterador->proximo;
		if (iterador->valor < minimo){
			minimo = iterador->valor;
		}
	}
	return minimo;
}
