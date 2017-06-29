/*
* Autor: Saúl Ibáñez Cerro
* Grado en Telemática
* Programa: myecho
*
* compilar: gcc -c -Wall -Wshadow -g myecho.c
* enlazar: gcc -o myecho myecho.o
* ejecutar: ./myecho
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>



//funcion para sacar el pid
void 
buscarPID(){
	pid_t p;						
	p = getpid();
	printf("%d", p);
}

void
buscarHOME(){
	char *home;
	home = getenv("HOME");
	printf("%s", home);
}

void
buscarPATH(){
	char *pwd;
	pwd = getenv("PWD");
	printf("%s", pwd);
}

void
buscarUSER(){
	char *user;
	user = getenv("USER");
	printf("%s", user);

}

void
comparar(int i, char *argv[]){
	if (strcmp(argv[i],"*")==0){
		buscarPID();
	}else if(strcmp(argv[i],"CASA")==0){
		buscarHOME();
	}else if(strcmp(argv[i],"DIRECTORIO")==0){
		buscarPATH();
	}else if (strcmp(argv[i],"USUARIO")==0){
		buscarUSER();
	}else{
		printf("%s",argv[i]);
	}
}

int
main(int argc, char *argv[])
{
	int i;

	if (argc == 1){
		printf("\n");
		exit(EXIT_SUCCESS);
	}

	if(strcmp(argv[1], "-n")==0){
		for (i = 2; i < argc; i++){
			comparar(i, argv);
			if(i<argc -1)
				printf(" ");
		}
	}else{
		for (i = 1; i < argc; i++){
			comparar(i, argv);
			printf(" ");
		}
		printf("\n");
	}
	exit(EXIT_SUCCESS);
}