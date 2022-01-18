#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define PROMPT_SIZE 128
const char message_bienvenue[BUFFER_SIZE] = "Bienvenue dans le Shell ENSEA. \nPour quitter, tapez 'exit'.\n";
const char prompt[PROMPT_SIZE]="enseash % ";
const char message_bye[PROMPT_SIZE]= "Bye Bye ! \n";
const char exit_txt[PROMPT_SIZE]="exit";

int main(){
	char cmd_buffer[BUFFER_SIZE];
	int pid, status,nb_bits_red;
	
	write(STDOUT_FILENO, message_bienvenue, BUFFER_SIZE); //afficher le message d'acceuil
	
	while(1){
		write(STDOUT_FILENO, prompt, PROMPT_SIZE); //afficher le prompt simple
		
		nb_bits_red = read(STDIN_FILENO, cmd_buffer, sizeof(cmd_buffer));
		
		if(nb_bits_red ==-1){perror("read impossible");exit(EXIT_FAILURE);}
		
		if(strncmp(cmd_buffer,exit_txt,strlen(exit_txt)) == 0 || nb_bits_red == 0){ //on teste si l'utilisateur effectue la commande "exit" ou un <ctrl>+d
			write(STDOUT_FILENO, message_bye, PROMPT_SIZE);
			return(EXIT_SUCCESS);		
		}	
		
		cmd_buffer[nb_bits_red-1]=0;  //on transforme \n par \0 pour indiquer la fin de la commande 
		
		pid = fork(); //création d'un processus fils qui va executer la commande
		
		if(pid<0){perror("fork impossible");exit(EXIT_FAILURE);}
		
		else if(pid != 0){ //father code
			wait(&status); // attente de la fin du processus fils 
		}
		else{			   // child code
			execlp(cmd_buffer,cmd_buffer,(char*)NULL);//executer la commande saisie sur le terminal
			perror(" impossible d'executer la commande");
			exit(EXIT_FAILURE);
		}
	}
	exit(EXIT_SUCCESS);
}

