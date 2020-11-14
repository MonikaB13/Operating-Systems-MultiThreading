#include "multi-lookup.h"

#define BUFFERSIZE 20
#define MAXSTR "%1024s"
#define MAXTHR 20
#define MIN_CMD_ARGS 6
#define NON_READ_FILE_ARGS 5
#define PTH_ARGV_NUM 1 
#define CTH_ARGV_NUM 2
#define RFILE_ARGV 3

char *DomainList[BUFFERSIZE];
pthread_mutex_t lock;
pthread_mutex_t parserloglock;
pthread_mutex_t counterlock;
pthread_mutex_t ofplock;
sem_t semlock;
sem_t buffer_full;
int i=0;/*number of items in the shared buffer*/
int input_files;/*number of input files*/
int counter = 0; /*number of input files that have been read*/
int Done = 0; /*indicator that all lines of all input files have been read*/
int Empty = 0; /*indicator that all entries have been read from the buffer*/
FILE* ofp = NULL;
FILE* parser_log = NULL;


int main(int argc, char* argv[])
{
	struct timeval start_time;
	struct timeval finish_time;
	int time = 0;
	if(gettimeofday(&start_time, NULL) != 0){
		printf("time function failed, run time will not be reported\n");
		time = 1;
	}
	int reader_thr = atoi(argv[PTH_ARGV_NUM]);/*turn ascii input into int*/
	int converter_thr = atoi(argv[CTH_ARGV_NUM]); /*turn ascii input into int*/
	int result_argv = argc - 2;
	int parser_log_argv = argc - 1;
	input_files = argc - NON_READ_FILE_ARGS;
	if(argc < MIN_CMD_ARGS) {/*make sure there is at least the min amt of cmd line args to run program*/
		fprintf(stderr, "Missing input or output file\n");
		return EXIT_FAILURE;
	}
	if(reader_thr < 1){
		fprintf(stderr, "need at least 1 reader thread\n");
		return EXIT_FAILURE;
	}
	if(reader_thr > (input_files)){ /*if more threads requested than input files, update reader_th to the number input files since there is supposed to be 1 thread per file per the project requirements*/
		fprintf(stderr, "too many parser threads requested, amount updated to number of input files\n");
		reader_thr = input_files;
	}
	if(converter_thr < 1){
		fprintf(stderr, "need at least one converter thread\n");
		return EXIT_FAILURE;
	}
	if (converter_thr > MAXTHR){
		fprintf(stderr, "too many converter threads requested, only 20 will be created\n");
		converter_thr = MAXTHR;
	}
	if(!(ofp = fopen(argv[result_argv],"w"))){
		fprintf(stderr, "Output file did not open\n");
		return EXIT_FAILURE;
	}
	if(!(parser_log = fopen(argv[parser_log_argv],"w"))){
		fprintf(stderr, "Parser log did not open\n");
		return EXIT_FAILURE;
	}
	if(pthread_mutex_init(&lock, NULL) != 0){
		fprintf(stderr, "mutex lock not initialized\n");
		return EXIT_FAILURE;
	}
	if(pthread_mutex_init(&ofplock, NULL) != 0){
		fprintf(stderr, "mutex lock not initialized\n");
		return EXIT_FAILURE;
	}
	if(pthread_mutex_init(&parserloglock, NULL) != 0){
		fprintf(stderr, "mutex lock not initialized\n");
		return EXIT_FAILURE;
	}
	if(pthread_mutex_init(&counterlock, NULL) != 0){
		fprintf(stderr, "mutex lock not initialized\n");
		return EXIT_FAILURE;
	}
	if(sem_init(&semlock, 0, 0) != 0){
		fprintf(stderr, "semaphore semlock did not initialize\n");
		return EXIT_FAILURE;
	}
	if(sem_init(&buffer_full, 0, BUFFERSIZE) != 0){
		fprintf(stderr, "semaphore buffer full did not initialize\n");
		return EXIT_FAILURE;
	}
	pthread_t tid[reader_thr]; /*thread identifier*/
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	for(int j = 0; j < reader_thr; j++){
		if(pthread_create(&tid[j], &attr, ReadDomainNames, (void*) argv) != 0){
			fprintf(stderr, "error creating parser threads\n");
			return EXIT_FAILURE;
		}
	}

	pthread_t tid2[MAXTHR]; /*thread identifier*/
	for(int j = 0; j < converter_thr; j++){
		if(pthread_create(&tid2[j], &attr, WriteIPAddr, NULL) != 0){
			fprintf(stderr, "error creating converter threads\n");
			return EXIT_FAILURE;
		}
	}
	for(int j = 0; j < reader_thr; j++){
		if(pthread_join(tid[j], NULL) != 0){
			fprintf(stderr, "error joining parser threads\n");
		}
	}
	Done = 1;/*all parser threads have returned*/
	for(int j = 0; j < converter_thr; j++){
		if(pthread_join(tid2[j], NULL) != 0){
			fprintf(stderr, "error joining converter threads\n");
		}
	}
	if(fclose(ofp) != 0){
		fprintf(stderr, "output file was not successfully closed\n");
	}
	if(fclose(parser_log) != 0){
		fprintf(stderr, "parser log file was not successfully closed\n");
	}
	if(pthread_mutex_destroy(&lock) != 0){
		fprintf(stderr, "mutex lock not destroyed\n");
	}
	if(pthread_mutex_destroy(&parserloglock) != 0){
		fprintf(stderr, "mutex parserloglock not destroyed\n");
	}
	if(pthread_mutex_destroy(&counterlock) != 0){
		fprintf(stderr, "mutex counterlock not destroyed\n");
	}
	if(sem_destroy(&semlock) != 0){
		fprintf(stderr, "semaphore semlock not destroyed\n");
	}
	if(gettimeofday(&finish_time, NULL) != 0){
		printf("time function error, runtime will not be reported\n");
		time = 1;
	}
		
	if(!time){
		printf("Program runtime was: %lu seconds\n", finish_time.tv_sec - start_time.tv_sec);
	}
	return 0;
}	

void* ReadDomainNames(void* args){
	char **cmd_args = (char**)args;
	int files_processed = 0;
	while(1){
		int index = 0;
		char hostname[BUFFERSIZE];
		FILE* ifp = NULL;
		pthread_mutex_lock(&counterlock);/*lock input file counter*/
		if(counter >= input_files){
			pthread_mutex_unlock(&counterlock);/*unlock input file counter*/
			pthread_mutex_lock(&parserloglock);/*lock parser log so threads can write to it as they finish*/
			fprintf(parser_log, "thread tid: %lu processed %d files\n", pthread_self(), files_processed);
			pthread_mutex_unlock(&parserloglock); /*unlock parser log*/
			break;
		}
		else{
			index = counter; /*save off global value counter*/
			counter = counter + 1; 
			pthread_mutex_unlock(&counterlock); /*unlock file counter lock*/
		}
		if(!(ifp = fopen(cmd_args[index+RFILE_ARGV], "r"))){
			fprintf(stderr, "input file did not open\n");
			printf("input file number %d did not open\n", index);
			exit(EXIT_FAILURE);
		}
		while(fscanf(ifp, MAXSTR, hostname)!=EOF){
			sem_wait(&buffer_full); /*if buffer full wait for a spot to open up*/
			pthread_mutex_lock(&lock); /*lock critical section*/
			DomainList[i]=malloc(1025*sizeof(char));
			strcpy(DomainList[i], hostname);
			i=i+1;
			sem_post(&semlock);/*increment the semaphore*/
			pthread_mutex_unlock(&lock); /*unlock critical section*/
		}
		if(fclose(ifp) != 0){
			fprintf(stderr, "input file was not properly closed\n");
		}
		files_processed = files_processed + 1;	
	}
	return NULL;
}

void* WriteIPAddr(){
	char firstIPstr[INET6_ADDRSTRLEN];
	char temp[1025];
	while(1){
		if(Empty){
			break;
		}
		sem_wait(&semlock);/*if semaphore value is 0, buffer is empty, wait*/
			pthread_mutex_lock(&lock); /*lock critical section*/
		i=i-1;
		strcpy(temp, DomainList[i]);/*copy buffer entry to temp*/
		free(DomainList[i]);
		sem_post(&buffer_full); /*signal readdomain names that a buffer slot has been emptied*/
		pthread_mutex_unlock(&lock);/*unlock critical section*/
		if(dnslookup(temp, firstIPstr, sizeof(firstIPstr)) == UTIL_FAILURE){/*lookup IP address*/
			
			fprintf(stderr, "IP address was not found for %s\n", temp);
			strncpy(firstIPstr, "", sizeof(firstIPstr));
		}
		pthread_mutex_lock(&ofplock);/*lock output file*/		
		fprintf(ofp, "%lu, %s, %s\n", pthread_self(), temp, firstIPstr);/*print to output file, is this open outside of the critical section?*/
		pthread_mutex_unlock(&ofplock);/*unlock output file*/
		if(Done == 1 && i == 0){
			Empty = 1; /*signal that the buffer is empty and that all parser reads are done*/
		}
	}
	return NULL;
}
