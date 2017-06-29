/*
* Autor: Saúl Ibáñez Cerro
* Grado en Telemática
* Programa: cunit
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <err.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

static char route[1024];

enum{Maxcommand = 128};
typedef struct TCmd TCmd;
struct TCmd{
	char *cmd;
	char *argument[64];
};
static TCmd command[Maxcommand];
static int nfile;
static int error;
static int tflag;
static int timeout;

static void
howexecute()
{
	fprintf(stderr,"How to use the program:\n");
	fprintf(stderr,"./cunit\n./cunit -c\n./cunit -t <N seconds>\n");
}

static void
tout(int no)
{
	fprintf(stderr, "interrupted test\n");
	exit(EXIT_FAILURE);
}

static int
tokenize(char *str, char **arg, char *sep)
{
	char *token;
	int i;

	token = strtok(str, sep);
	for(i=0; token!=NULL; i++){
		arg[i]=token;
		if((token=strtok(NULL, sep))==NULL){
			i++;
			break;
		}

		if((strcmp(sep, ".")==0)&&(i>0)){
			sprintf(arg[0], "%s.%s", arg[0], arg[i]);
		}
	}
	return i;
}

static int
comparefiles(char *file1, char *file2)
{
	FILE *fp, *fp2;
	char buffer1[1024];
	char buffer2[1024];
	int sizefile1;

	sizefile1=0;
	fp=fopen(file1,"r");
	if (fp==NULL)
		err(1, "fail fopen %s", file1);

	fp2=fopen(file2,"r");
	if (fp2==NULL)
		err(1, "fail fopen %s", file2);

	do{
		sizefile1=fread(buffer1,sizeof(char),1024,fp);
		fread(buffer2,sizeof(char),1024,fp2);
		if(memcmp(buffer1, buffer2, sizefile1)!=0){
			return -1;
		}
	}while(!feof(fp) || (!feof(fp2)));
	
	fclose(fp);
	fclose(fp2);
	return 0;
}

static int
cpyfiles(char *fout, char *fok)
{
	int fd, fp2, nr;
	char buffer[1024];

	fd=open(fout, O_RDONLY);
	if(fd<0){
		warn("%s", fout);
		return -1;
	}

	fp2=creat(fok, 0660);
	if(fp2<0){
		err(1, "creat failed");
	}

	for(;;){
		nr=read(fd, buffer, 1024);
		if(nr < 0){
			warn("%s", fout);
			return -1;
		}
		if(nr==0){
			break;
		}
		if(write(fp2, buffer, nr)<0){
			warnx("%s", strerror(errno));
			return -1;
		}
	}
	
	close(fd);
	close(fp2);
	return 0;
}

static void
searchdolar(char *token[], int ninstructions)
{
	int i;

	for (i = 0; i < ninstructions; i++){
		if(token[i][0]=='$'){
			token[i]=getenv(&token[i][1]);
		}
	}
}

static int
searchtst(char *name)
{
	char *findocurrence;
	
  	findocurrence = strrchr (name, '.');
	if (findocurrence != NULL)
		return (strcmp(findocurrence,".tst")==0);
	return 0;
}

static int
searchcond(char *name)
{
	char *findocurrence;
	
  	findocurrence = strrchr (name, '.');
	if (findocurrence != NULL)
		return (strcmp(findocurrence,".cond")==0);
	return 0;
}

static int
searchextension(char *name)
{
	char *findocurrence;
	
  	findocurrence = strrchr (name, '.');
	if (findocurrence != NULL)
		return ((strcmp(findocurrence,".out")==0) || (strcmp(findocurrence,".ok")==0));
	return 0;
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
freecmd(int ncmd)
{
	int i, j;

	for(i=0;i<ncmd;i++){
		j=0;
		while(command[ncmd].argument[j]!=NULL){
			free(command[ncmd].argument[j]);
			j++;
		}	
	}
}

static void
runwithoutpipes(int ncmd, char *name)
{
	int i;
	int pid;
	int status;
	char path[1024];
	char *aux[2];
	char fout[1024];
	char fileok[1024];
	int fd;

	for (i = 0; i < ncmd; i++){
		pid=fork();
		switch(pid){
		case -1:
			err(1, "fork failed");
		case 0:
			searchroute(path, command[i].cmd);
			tokenize(name, aux, ".");

			sprintf(fout, "%s/%s.out", route, aux[0]);
			fd = creat(fout, 0660);
			if(fd<0){
				err(1, "creat failed");
			}
			dup2(fd, 1);
			dup2(fd, 2);
			close(fd);

			if (i==0){
				fd=open("/dev/null", O_RDONLY);
				if(fd<0)
					err(1, "open failed");
				dup2(fd, 0);
				close(fd);
			}

			execv(path, command[i].argument);
			err(1, "exev failed, command: %s, path: %s", command[i].cmd, path);
		default:
			wait(&status);
			if(status == 0){
				tokenize(name, aux, ".");
				sprintf(fileok, "%s/%s.ok", route, aux[0]);
				sprintf(fout, "%s/%s.out", route, aux[0]);

				if (access(fileok, F_OK)==0){
					if(comparefiles(fileok, fout)!=0){
						status=1;
					}
				}else{
					if(cpyfiles(fout, fileok)){
						error=1;
					}
				}

				if(status!=0){
					status=1;
				}

				freecmd(ncmd);
				exit(status);
			}
		}
	}

	freecmd(ncmd);
	exit(EXIT_FAILURE);
}

static void
closepipes(int ncmd, int p[128][2])
{
	int i;

	for (i = 0; i < ncmd-1; i++){
		close(p[i][0]);
		close(p[i][1]);
	}
}

static int
executepipeline(int ncmd, int iloop, int p[128][2])
{

	if(ncmd > 1){
		if(iloop==0){
			if(dup2(p[iloop][1],1)<0){
				warn("dup2 failed");
			}

		}else if(iloop==ncmd-1){
			if(dup2(p[iloop-1][0], 0)<0){
				warn("dup2 failed");
			}

		}else{
			if(dup2(p[iloop-1][0], 0)<0){
				warn("dup2 failed");
			}

			if(dup2(p[iloop][1],1)<0){
				warn("dup2 failed");
			}
		}

		closepipes(ncmd, p);
	}

	return (ncmd > 1);
}

static void
executecmd(int ncmd, char *name)
{
	int i;
	int pid;
	int status;
	char path[1024];
	int p[128][2];
	int thereispipe=0;
	char *aux[2];
	char fout[1024];
	char fileok[1024];
	int fd;

	if (ncmd>1){
		for(i=0;i<ncmd-1;i++){
			pipe(p[i]);
		}
		thereispipe=1;
	}

	for (i = 0; i < ncmd; i++){
		pid=fork();
		switch(pid){
		case -1:
			err(1, "fork failed");
		case 0:

			if(executepipeline(ncmd, i, p)){
				thereispipe=1;
			}
			tokenize(name, aux, ".");

			if(i==ncmd-1){
				sprintf(fout, "%s/%s.out", route, aux[0]);
				fd = creat(fout, 0660);
				if(fd<0){
					err(1, "creat failed");
				}
				dup2(fd, 1);
				dup2(fd, 2);
				close(fd);
			}

			if (i==0){
				fd=open("/dev/null", O_RDONLY);
				if(fd<0)
					err(1, "open failed");
				dup2(fd, 0);
				close(fd);
			}
			searchroute(path, command[i].cmd);
			execv(path, command[i].argument);
			err(1, "exev failed, command: %s, path: %s", command[i].cmd, path);
		}
	}

	if(thereispipe){
		closepipes(ncmd, p);
	}

	waitpid(pid, &status, 0);
	tokenize(name, aux, ".");
	sprintf(fileok, "%s/%s.ok", route, aux[0]);
	sprintf(fout, "%s/%s.out", route, aux[0]);

	if (access(fileok, F_OK)==0){
		if(comparefiles(fileok, fout)!=0){
			status=1;
		}
	}else{
		if(cpyfiles(fout, fileok)){
			error=1;
		}
	}

	if(status!=0){
		status=1;
	}
	
	freecmd(ncmd);
	exit(status);
}

static void
savecmdtst(char *name)
{
	FILE *fp;
	char buffer[1024];
	char *aux[1024];
	char *instructions;
	int ninstructions, ncmd;
	int i;
	char *home;

	if(tflag){
		signal(SIGALRM, tout);
		alarm(timeout);
	}

	ncmd=0;
	ninstructions=0;
	fp=fopen(name, "r");
	if(fp==NULL)
		err(1, "open fail %s", name);
	for(;;){
		instructions = fgets(buffer, 1024, fp);
		if(instructions==NULL){
			break;
		}
		if(strlen(buffer)==1){
			continue;
		}
		ninstructions=tokenize(instructions, aux, " \n");

		if(ninstructions==0)
			continue;

		searchdolar(aux, ninstructions);

		if(strcmp(aux[0], "cd")==0){
			if(ninstructions==1){
				home = getenv("HOME");
				chdir(home);
			}else{
				if(chdir(aux[1])<0){
					exit(EXIT_FAILURE);
				}
			}
			continue;
		}

		for (i = 0; i < ninstructions; i++){
			command[ncmd].argument[i]=strdup(aux[i]);
		}
		
		command[ncmd].cmd=command[ncmd].argument[0];
		command[ncmd].argument[ninstructions]=NULL;
		ncmd++;
	}

	fclose(fp);
	if(searchtst(name)){
		executecmd(ncmd, name);
	}else{
		runwithoutpipes(ncmd, name);
	}

}

static int
opendirectory()
{
	DIR *d;
	struct dirent *reader;
	struct stat st;
	int i;
	char *name;
	int status;

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
		if (((st.st_mode & S_IFMT)==S_IFREG) && (((searchtst(name))) || (searchcond(name)))){
			nfile++;
			switch (fork()){
			case -1:
				err(1,"fork failed");
			case 0:
				savecmdtst(name);
			}
		}
	}
	closedir(d);

	for (i = 0; i < nfile; i++){
		wait(&status);
		if(status != 0){
			error=1;
		}
	}

	return 0;
}

static int
deletefiles()
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
		if (((st.st_mode & S_IFMT)==S_IFREG)&&(searchextension(name))){
			unlink(name);
		}
	}
	closedir(d);
	return 0;
}

int main(int argc, char *argv[])
{
	
	getcwd(route, 1024);

	if(argc==1){
		if(opendirectory()<0){
			err(1, "%s", route);
		}
		
	}else if(argc==3){
		if(strcmp(argv[1], "-t")==0){
			tflag=1;
			timeout=atoi(argv[2]);
			if(timeout==0){
				errx(1, "if you use -t option, you must enter a number higher or equal to 1");
			}
		}else{
			howexecute();
			exit(EXIT_FAILURE);
		}

		if(opendirectory()<0){
			err(1, "%s", route);
		}

	}else if(argc==2){
		if(strcmp(argv[1], "-c")==0){
			if(deletefiles()<0){
				err(1, "%s", route);
			}
			exit(EXIT_SUCCESS);
		}
		else{
			howexecute();
			exit(EXIT_FAILURE);
		}

	}else{
		howexecute();
		exit(EXIT_FAILURE);
	}

	if (error!=0){
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
