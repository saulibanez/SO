/*
* Autor: Saúl Ibáñez Cerro
* Grado en Telemática
* Programa: frec
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>

enum {N=128};
enum {TOTAL_ASCII=256};

typedef struct Tfrec Tfrec;
struct Tfrec
{
	int posletter;
	int first;
	int end;
};
static Tfrec frec[TOTAL_ASCII];
static char *nameprogram;
static int flag;

void
howexecute()
{
	printf("How to use the program:\n");
	printf("./freq <filename>\n./freq -i <filename>\n./freq\n");
}

void
imprimir(int totalletter)
{
	int i;
	float porcentaje;

	for (i = 0; i < TOTAL_ASCII; i++)
	{
		if(frec[i].posletter>0){
			porcentaje = ((float)frec[i].posletter * 100) / totalletter;
			printf("%c %.2f %% %i %i\n",i, porcentaje, frec[i].first, frec[i].end);	//pongo %c para que me saque el caracter de ASCII, por ejemplo, si pongo %d, i me saca la posicion 97, sin embargo, como esta saca la letra "a"
		}
	}
}

int
ischaracter(char c, int totalletter, int isfirst){

	if (isalpha(c) && !isdigit(c)){
		if(flag!=1){
			//para convertirlo a minuscula
			c=tolower(c);
		}
		if(isfirst==1){
			frec[(int)c].first+= +1;
		}
		totalletter++;
		frec[(int)c].posletter+= +1;
	}
	return totalletter;
}

void
saveend(char c)
{
	if(flag!=1){
		c=tolower(c);
	}
	frec[(int)c].end+= +1;
}

int
readfile(int fd)
{
	char buffer[N];
	char c;
	int i;
	int totalletter;
	int nr;
	int isfirst;

	totalletter=0;
	isfirst=1;

	for(;;){
		nr = read(fd, buffer, sizeof buffer);

		if(nr == 0){
			break;
		}
		if(nr < 0){
			err(1, "read");
		}
		for(i=0;i<nr;i++){
			c=buffer[i];

			totalletter = ischaracter(c, totalletter, isfirst);

			if(isalpha(c)){
				isfirst=0;
			}
			if(!isalnum(c)){
				isfirst=1;
				if(isalpha(buffer[i-1])){
					saveend(buffer[i-1]);
				}
			}
		}
	}
	imprimir(totalletter);
	return nr;
}


int
main(int argc, char *argv[])
{
	int fd;

	nameprogram = argv[0];
	argc = argc -1;

	if (argc > 2){
		fprintf(stderr, "fail program: %s\n", nameprogram);
		howexecute();
		return 1;
	}

	if(argc==0){
		fd = 0;
		if(fd < 0){
			err(1, NULL);
		}
	}else if(strcmp(argv[1], "-i")==0){
		fd = open(argv[2], O_RDONLY);
		if(fd < 0){
			err(1, "%s", argv[2]);
		}
		flag=1;
	}else{
		fd = open(argv[1], O_RDONLY);
		if(fd < 0){
			err(1, "%s", argv[1]);
		}
	}

	if(readfile(fd)<0){
		err(1, "read");
	}
	if(fd!=0)
		close(fd);
	exit(EXIT_SUCCESS);
}
