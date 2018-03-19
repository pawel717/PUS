/*
 * libpalindrome.h
 *
 *  Created on: 15 mar 2018
 *      Author: root
 */

#ifndef LIBPALINDROME_H_
#define LIBPALINDROME_H_

/*
 * Funkcja sprawdza czy dane w buforze wskazywanym przez buff sa palindromem
 * liczbowym. Funkcja ignoruje biale znaki i wiodace zera.
 *
 * Parametry:
 * buff - wskaznik na bufor zawierajacy dane odebrane za pomoca funkcji read(),
 *        recv() lub recvfrom()
 *
 * buff_len - rozmiar danych w buforze (nie rozmiar bufora); jest to liczba
 *            bajtow zwrocona przez funkcje read(), recv() lub recvfrom()
 *
 * Wartosc zwracana:
 * -1 - dane w buforze zawieraja znaki, ktore nie sa cyframi lub znakami bialymi
 *  0 - dane w buforze nie sa palindromem liczbowym
 *  1 - dane w buforze sa palindromem liczbowym*
 *
 */
int is_palindrome(char* buff, int buff_len);

#endif /* LIBPALINDROME_H_ */
