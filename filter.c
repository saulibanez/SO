/*
* Autor: Saúl Ibáñez Cerro
* Grado en Telemática
* Programa: filter
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

static int error = 0;

static int
execute(char *namefile, char *regularexpression, char *command[])
{
	int p[2];
	int fd;
	int pid, pidgrep;
	int statuspid;
	int statuspidgrep;

	if (pipe(p)<0)
		err(1, "pipe failed");

	pid = fork();
	switch(pid){
	case -1:
		err(1, "fork failed");
	case 0:
		fd = open(namefile, O_RDONLY);
		if(fd<0){
			err(1,"file %s", namefile);
		}
		if(dup2(fd, 0)<0)
			err(1,"dup2 failed");
		close(fd);
		close(p[0]);
		if(dup2(p[1],1)<0)
			err(1, "dup2 failed");
		close(p[1]);
		execvp(command[0], command);
		err(1, "execvp failed, command %s", command[0]);
	}

	pidgrep = fork();
	switch(pidgrep){
	case -1:
		err(1, "fork failed");
	case 0:
		close(p[1]);
		if(dup2(p[0], 0)<0)
			err(1, "dup2 failed");
		close(p[0]);
		execlp("grep", "grep", regularexpression, NULL);
		err(1, "execlp failed, command grep");
	}

	close(p[1]);
	close(p[0]);

	while(wait(&statuspid) != pid)
		;

	while(wait(&statuspidgrep) != pidgrep)
		;

	return ((statuspid==0)&&(statuspidgrep==0));

}

static int
opendirectory(char *route, char **argv)
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

		if ((st.st_mode & S_IFMT)==S_IFREG){
			if(execute(name, argv[1], &argv[2])==0){
				error = 1;
			}
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
		errx(1, "fail use program:\nfilter <regular expression> <command> n_indeterminate parametres");
	}

	getcwd(route, 1024);
	if(opendirectory(route, argv)<0){
		err(1, "%s", route);
	}

	if(error != 0){
		exit(EXIT_FAILURE);
	}
	
	exit(EXIT_SUCCESS);
}
