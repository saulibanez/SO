/*
* Autor: Saúl Ibáñez Cerro
* Grado en Telemática
* Programa: bigrams
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <err.h>
#include <errno.h>

static pthread_mutex_t mutex;
static unsigned char *matrix;

static void
howexecute()
{
	fprintf(stderr,"How to use the program:\n");
	fprintf(stderr,"./bigrams <new file(map)> <files>\n./bigrams -p <new file(map)>\n");
}

static void
printmap()
{
	int i;
	int k;

	for(i=0;i<128;i++){
		for(k=0;k<128;k++){
			printf("(%d,%d): %d\n",i,k,matrix[i*128+k]);
		}
	}
}

static void
firstintroduceinmatrix(unsigned char buffer[], int line)
{
	unsigned char coordx, coordy;
	int i;

	for(i=0;i<line-1;i++){
		coordx=buffer[i];
		coordy=buffer[i+1];
		pthread_mutex_lock(&mutex);
		if ((matrix[coordx*128+coordy]<255)&&(coordx>=0)&&(coordx<128)&&(coordy>=0)&&(coordy<128)){
			matrix[coordx*128+coordy]=matrix[coordx*128+coordy]+1;
		}
		pthread_mutex_unlock(&mutex);
	}
}

static void
continueintroduceinmatrix(unsigned char buffer[], int line, unsigned char coordx)
{
	unsigned char coordy;
	int i;

	for(i=0;i<line;i++){
		if(i==0){
			coordy=buffer[i];
			pthread_mutex_lock(&mutex);
			if ((matrix[coordx*128+coordy]<255)&&(coordx>=0)&&(coordx<128)&&(coordy>=0)&&(coordy<128)){
				matrix[coordx*128+coordy]=matrix[coordx*128+coordy]+1;
			}
			pthread_mutex_unlock(&mutex);
		}else{
			coordx=buffer[i-1];
			coordy=buffer[i];
			pthread_mutex_lock(&mutex);
			if ((matrix[coordx*128+coordy]<255)&&(coordx>=0)&&(coordx<128)&&(coordy>=0)&&(coordy<128)){
				matrix[coordx*128+coordy]=matrix[coordx*128+coordy]+1;
			}
			pthread_mutex_unlock(&mutex);
		}
	}
}

static void*
tmain(void *a)
{
	char *file=(char*)a;
	unsigned char coordx;
	unsigned char buffer[1024];
	FILE *fp;
	int line;
	int firstenter;
	
	firstenter=1;
	fp=fopen(file,"r");
	if (fp==NULL)
		pthread_exit(NULL);

	do{
		line=fread(buffer,sizeof(unsigned char),1024,fp);
		if (firstenter){
			firstintroduceinmatrix(buffer, line);
			firstenter=0;
		}else{
			continueintroduceinmatrix(buffer, line, coordx);
		}
		coordx=buffer[line-1];

	}while(!feof(fp)&&(line>0));
	
	fclose(fp);
	pthread_exit(NULL);
	return NULL;
}

static int
createmap(char **argv)
{
	int fd;
	struct stat st;
	int totalsize;

	fd = open (argv[1], O_RDWR|O_CREAT|O_TRUNC, 0660);
	if (fd<0){
		warn("%s", argv[1]);
		return -1;
	}

	lseek (fd, (128*128)-1, SEEK_SET);
	if(write(fd, " ", 1)<0){
		warnx("%s", strerror(errno));
		return -1;
	}

	if (stat(argv[1], &st) < 0){
		err(1, "%s", argv[1]);
	}

	totalsize=st.st_size;
	matrix = mmap (0, totalsize, PROT_READ|PROT_WRITE,MAP_FILE|MAP_SHARED, fd, 0);
	if(matrix == MAP_FAILED){
		err(1, "mmap");
	}
	
	memset(matrix, 0, totalsize);
	close(fd);
	
	return totalsize;
}

static void
thereisflag(char **argv)
{
	int fd;
	struct stat st;
	int totalsize;

	fd = open(argv[1], O_RDONLY);
	if(fd<0){
		err(1, "%s", argv[1]);
	}

	if (stat(argv[1], &st) < 0){
		err(1, "%s", argv[1]);
	}

	totalsize=st.st_size;
	matrix = mmap (0, totalsize, PROT_READ, MAP_FILE|MAP_SHARED, fd, 0);
	if(matrix == MAP_FAILED){
		err(1, "map failed");
	}

	printmap();
	close(fd);
	if(munmap(matrix,totalsize)<0){
		err(1, "munmap failed");
	}
}

int
main(int argc, char *argv[])
{
	pthread_t *thr;
	int i;
	int totalsize;

	if (argc<3){
		howexecute();
		exit(EXIT_FAILURE);
	}

	if(strcmp(argv[1], "-p")==0){
		argv++;
	 	thereisflag(argv);
	}else{

		if((totalsize=createmap(argv))<0){
			exit(EXIT_FAILURE);
		}

		argc=argc-2;
		argv=argv+2;
		thr=malloc(sizeof(pthread_t)*argc);
		pthread_mutex_init(&mutex, NULL);

		for(i=0;i<argc;i++){
			if(pthread_create(&thr[i], NULL, tmain, (void*)argv[i])!=0){
				err(1, "thread failed");
			}
		}

		for(i=0;i<argc;i++){
			pthread_join(thr[i],NULL);
		}

		if(munmap(matrix,totalsize)<0)
			err(1, "munmap failed");
		free(thr);
	}

	exit(EXIT_SUCCESS);
}
