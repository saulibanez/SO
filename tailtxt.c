/*
* Autor: Saúl Ibáñez Cerro
* Grado en Telemática
* Programa: tailtxt
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

static int nbytes;
static int flag;

static void
howexecute()
{
	fprintf(stderr, "How to use the program:\n");
	fprintf(stderr, "./tailtxt \n./tailtxt <N bytes positives that you want to read> \n");
}

static int 
istxt(char *name)
{
	char *findocurrence;

	findocurrence=strstr(name, ".txt");
	return (findocurrence!=NULL)&&(strcmp(findocurrence, ".txt")==0);
}

static int
openreadtxt(char *file)
{
	int fd;
	int nr;
	char buffer[1024];

	fd=open(file, O_RDONLY);
	if(fd<0){
		warn("%s", file);
		return 0;
	}
	if(flag){
		lseek(fd, nbytes * -1, SEEK_END);
	}
	for(;;){
		nr=read(fd, buffer, 1024);
		if(nr < 0){
			warn("%s", file);
		}
		if(nr==0){
			break;
		}
		if(write(1, buffer, nr)<0){
			warnx("%s", strerror(errno));
			return 0;
		}
	}
	if(fd!=0)
		close(fd);
	return 0;
}

static int
opendirectory(char *route)
{
	DIR *d;
	struct dirent *reader;
	struct stat st;
	char *name;

	d = opendir(route);
	if (d==NULL){
		return -1;
	}

	for(;;){
		reader=readdir(d);
		if (reader==NULL){
			break;
		}
		name = reader->d_name;

		if (stat(name, &st) < 0){
			err(1, "name");
		}

		if (((st.st_mode & S_IFMT)==S_IFREG) && (istxt(name))){
			if(openreadtxt(name)<0){
				return -1;
			}
		}
	}
	closedir(d);
	return 0;
}

int 
main(int argc, char *argv[])
{
	int n;
	char route[1024];

	if(argc>2){
		fprintf(stderr, "fail: too many arguments\n");
		howexecute();
		exit(EXIT_FAILURE);
	}
	
	if (argv[1]!=NULL){
		flag=1;
		n=atoi(argv[1]);
		if(n>0)
			nbytes=n;
		else{
			fprintf(stderr, "fail: %s\n", argv[1]);
			howexecute();
			exit(EXIT_FAILURE);
		}
	}

	getcwd(route, 1024);
	if(opendirectory(route)<0){
		err(1, "%s", route);
	}

	exit(EXIT_SUCCESS);
}
