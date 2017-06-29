/*
* Autor: Saúl Ibáñez Cerro
* Grado en Telemática
* Programa: proctailtxt
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
#include <sys/wait.h>

enum{Maxtxt=20};
static char filetxt[Maxtxt][512];
static int nfichtxt;
static int nbytes;
static int flag;

static void
howexecute()
{
	fprintf(stderr, "How to use the program:\n");
	fprintf(stderr, "./proctailtxt \n./proctailtxt <N bytes positives that you want to read> \n");
}

static int 
istxt(char *name)
{
	char *findocurrence;
	
  	findocurrence = strrchr (name, '.');

	if (findocurrence != NULL)
		return (strcmp(findocurrence,".txt")==0);
	return 0;
}

static int
createwritetxtout(char *file)
{
	int fd;
	int nr;
	char buffer[1024];
	int fd2;
	char out[]= ".out";
	char *endtxtout;
	
	fd=open(file, O_RDONLY);

	if(fd<0){
		warn("%s", file);
		return -1;
	}

	endtxtout = strcat(file, out);
	fd2=creat(endtxtout, 0660);
	if(fd2<0){
		warn("%s", endtxtout);
		return -1;
	}

	if(flag){
		if (lseek(fd, nbytes * -1, SEEK_END)<0){
			lseek(fd, 0, SEEK_SET);
		}
	}
	for(;;){

		nr=read(fd, buffer, 1024);
		if(nr < 0){
			warn("%s", file);
			return -1;
		}
		if(nr==0){
			break;
		}
		if(write(fd2, buffer, nr)<0){
			warnx("%s", strerror(errno));
			return -1;
		}
	}
	if(fd!=0)
		close(fd);
	if(fd2!=0)
		close(fd2);
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

		if (((st.st_mode & S_IFMT)==S_IFREG) && (istxt(name)) && (nfichtxt<=Maxtxt)){
			strcpy(filetxt[nfichtxt], name);
			nfichtxt++;
		}
	}
	closedir(d);

	return 0;
}

static int
processfork()
{
	int pid;
	int i;
	int status;

	for (i = 0; i < nfichtxt; i++){
		pid = fork();
		switch (pid){
		case -1:
			err(1, "fork failed");
			break;
		case 0:
			if(createwritetxtout(filetxt[i])<0){
				return -1;
			}
			return 0;
			break;
		}
	}

	for (i = 0; i < nfichtxt; i++){
		wait(&status);
		if(status != 0)
			return -1;
	}

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
	if(nfichtxt>Maxtxt){
		fprintf(stderr, "too files .txt\nthe directory should have less than 20 files .txt\n");
		exit(EXIT_FAILURE);
	}

	if(processfork()<0)
		exit(EXIT_FAILURE);


	exit(EXIT_SUCCESS);
}
