/*
* Autor: Saúl Ibáñez Cerro
* Grado en Telemática
* Programa: fifocmd
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

static int error;

static int
tokenize(char *str, char **arg, char *sep)
{
	char *token;
	int i;

	token = strtok(str, sep);
	for(i=0; token!=NULL; i++){
		arg[i]=token;
		if((token=strtok(NULL, sep))==NULL){
			break;
		}
	}
	return i;
}

static void
searchroute(char *path, char *cmd)
{
	char *p;
	char *aux[1024];
	int ntoken, i;

	if(access(cmd, X_OK)==0){
		strcpy(path, cmd);
	}else{
		p=getenv("PATH");
		ntoken=tokenize(p,aux,":");
		for(i=0; i<ntoken;i++){
			sprintf(path, "%s/%s", aux[i], cmd);
			if(access(path, X_OK)==0){
				break;
			}
		}
	}
}

static void
errorredirect(){
	int fd;

	fd = creat("/dev/null", 0660);
	if(fd<0)
		err(1, "/dev/null fail");
	if(dup2(fd, 2)<0)
		err(1, "dup2 fd error failed");
	close(fd);
}

static void
openfifocmdout(){
	int fd;

	fd = open("fifocmd.out", O_WRONLY|O_CREAT|O_APPEND, 0660);
	if(fd < 0){
		err(1, "err");
	}
	if (dup2(fd, 1)<0)
		err(1, "dup2 fd failed");
	close(fd);
}

static int
execute(char *command1[], char *command2[])
{
	int p[2];
	int pid, pid2;
	char path[1024];
	int statuspid;
	int statuspid2;

	if (pipe(p)<0)
		err(1, "pipe failed");

	pid = fork();
	switch(pid){
	case -1:
		err(1, "fork failed");
	case 0:
		close(p[0]);
		if(dup2(p[1],1)<0)
			err(1, "dup2 pipe 1 failed");
		close(p[1]);
		errorredirect();
		searchroute(path, command1[0]);
		execv(path, command1);
		err(1, "exev failed, command: %s, path: %s", command1[0], path);
	}

	pid2=fork();
	switch(pid2){
	case -1:
		err(1, "fork failed");
	case 0:
		close(p[1]);
		if(dup2(p[0], 0)<0)
			err(1, "dup2 pipe 0 failed");
		close(p[0]);
		openfifocmdout();
		errorredirect();		
		searchroute(path, command2[0]);
		execv(path, command2);
		err(1, "exev failed, command: %s, path: %s", command2[0], path);
	}

	close (p[0]);
	close (p[1]);

	while(wait(&statuspid) != pid)
		;

	while(wait(&statuspid2) != pid2)
		;

	return ((statuspid==0)&&(statuspid2==0));
}

static int
openfifo(char **argv)
{
	FILE *fp;
	char buffer[1024];
	char *aux[1024];
	char *linea;
	int n;
	
	if ((fp = fopen(argv[1], "r"))==NULL){
		warn("%s", argv[1]);
		return -1;
	}

	for(;;){
		linea=fgets(buffer, 1024, fp);
		if(linea==NULL)
			break;
		n = tokenize(linea, aux, " \n");
		aux[n+1]=NULL;
		if(execute(aux, &argv[2])==0)
			error = 1;
	}

	fclose(fp);
	return 0;
}

static int 
searchfifocmdout(char *name)
{
	char *findocurrence;
	
  	findocurrence = strchr (name, 'f');
	if (findocurrence != NULL)
		return (strcmp(findocurrence,"fifocmd.out")==0);
	return 0;
}

static int
delfifocmdout(char *route)
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
		if (((st.st_mode & S_IFMT)==S_IFREG)&&(searchfifocmdout(name))){
			unlink(name);
		}
	}
	closedir(d);
	return 0;
}

int
main(int argc, char *argv[])
{
	char route[1024];

	if(argc<3){
		errx(1, "fail execute, how use the program:\nfifocmd <path> <ncommand>");
	}

	getcwd(route, 1024);
	if(delfifocmdout(route)<0){
		err(1, "%s", route);
	}

	if (mkfifo(argv[1], 0660)<0){
		if(errno == EEXIST){
			unlink(route);
		}else
			err(1, "mkfifo failed");
	}

	for(;;){
		if(openfifo(argv)<0)
			exit(EXIT_FAILURE);
	}
	if(error != 0)
		exit(EXIT_FAILURE);
	exit(EXIT_SUCCESS);
}
