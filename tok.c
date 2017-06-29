/*
* Autor: Saúl Ibáñez Cerro
* Grado en Telemática
* Programa: tok
*
* compilar: gcc -c -Wall -Wshadow -g tok.c
* enlazar: gcc -o tok tok.o
* ejecutar: ./tok
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char cadena[] = "   \n Esto			es\r \n   una \t \r cadena  \r  de     prueba   	\n  ";
enum {max = 10};
char *pol[max];

// Funcion para imprimir los token de la cadena
void
print(int contador, char **args){
	int k;

	for (k = 0; k < contador; k++){
		printf(">> %s\n",args[k]);
	}
}

// Funcion que me guarda los token
void
savetoken(char*c, char**args, int pos){
	args[pos]=c;
}

// Funcion para comparar si lo que tengo es un separador
int
separadores(char c){
	return (c==' ') || (c=='\r') || (c=='\t') || (c=='\n') || (c=='\0');
}

// Funcion para separar los caracteres validos de los invalidos
int
mytokenice(char*str, char**args, int maxargs)
{
	int i;
	int contador;
	int maxCadena;
	contador = 0;
	maxCadena = strlen (str);
	for (i = 0; i < maxCadena; i++)
	{
		if (separadores(str[i])){
			str[i] = '\0'; //salto a la siguiente celda, para que se me quede guardado solo la palabra
			if (!separadores(str[i+1]) && i < maxCadena){
				savetoken(&str[i+1], args, contador);
				contador++;
			}
		}else if (i == 0 && !separadores(str[i])){
			savetoken(&str[i], args, contador);
			contador++;
		}
	}
	print(contador, args);
	return contador;
}

int
main(int argc, char *argv[])
{
	printf("La cadena es: \n");
	mytokenice(cadena, pol, max);
	exit(EXIT_SUCCESS);
}