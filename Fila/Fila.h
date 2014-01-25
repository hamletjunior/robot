/*
 * Fila.h
 *
 *  Created on: 21/01/2014
 *      Author: hamletpessoafariasjunior
 */

#ifndef FILA_H_
#define FILA_H_
#include "No.h"
#define TAMANHO_MAXIMO_FILA 15

class Fila {
	No *primeiro;
	No *ultimo;
public:
	Fila();
	void adicionaValor(int valor);
	int tamanho();
	int maximo();
	int minimo();
};

#endif /* FILA_H_ */
