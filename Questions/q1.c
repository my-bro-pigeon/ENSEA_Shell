#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define PROMPT_SIZE 128
const char message_bienvenue[BUFFER_SIZE] = "Bienvenue dans le Shell ENSEA. \nPour quitter, tapez 'exit'.\n";
const char prompt[PROMPT_SIZE]="enseash % \n";

int main(){
	write(STDOUT_FILENO, message_bienvenue, BUFFER_SIZE); //afficher le message d'acceuil
	write(STDOUT_FILENO, prompt, PROMPT_SIZE); //afficher le prompt simple
}
