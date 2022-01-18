#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>

/* prototypes des fonctions */

void exit_code(int,struct timespec, struct timespec);

char** split_cmd(char*,int*); //Séparer la commande en mots pour recupérer les différents arguments 

void free_cmd(char**,int*); // Libérer la mémoire allouée pour la séparation en mots de la commande après l'exécution de cette commande 

void full_exec(char**, int*); // On crée une fonction d'execution que l'on va appeler dans le fils et qui va executer la commande en prenant compte des > et <. 

/*  */
#define BUFFER_SIZE 1024
#define PROMPT_SIZE 128
const char message_bienvenue[BUFFER_SIZE] = "Bienvenue dans le Shell ENSEA. \nPour quitter, tapez 'exit'.\n";
const char prompt[PROMPT_SIZE]="enseash []% ";
const char message_bye[PROMPT_SIZE]= "Bye Bye ! \n";
const char exit_txt[PROMPT_SIZE]="exit";
const char * separators=" ";

char buffer_retour[PROMPT_SIZE];

int main(){
	char cmd_buffer[BUFFER_SIZE];
	int cmd_size;
	char** cmd;
	int pid, status,nb_bits_red;
	struct timespec child_start, child_stop;

	
	write(STDOUT_FILENO, message_bienvenue, BUFFER_SIZE); //afficher le message d'acceuil
	write(STDOUT_FILENO, prompt, PROMPT_SIZE); //afficher le prompt simple
	
	while(1){
		
		
		strncat(buffer_retour,prompt,9);  // ajout des 9 premiers caractères du prompt
		
		nb_bits_red = read(STDIN_FILENO, cmd_buffer, sizeof(cmd_buffer));
		
		if(nb_bits_red ==-1){perror("read impossible");exit(EXIT_FAILURE);}
		
		if(strncmp(cmd_buffer,exit_txt,strlen(exit_txt)) == 0 || nb_bits_red == 0){ //on teste si l'utilisateur effectue la commande "exit" ou un <ctrl>+d
			write(STDOUT_FILENO, message_bye, PROMPT_SIZE);
			return(EXIT_SUCCESS);		
		}	
		
		cmd_buffer[nb_bits_red-1]=0;  //on transforme \n par \0 pour indiquer la fin de la commande 
		
		cmd = split_cmd(cmd_buffer, &cmd_size); // On sépare la commande en mots pour récupérer les arguments 
		
		if(clock_gettime(CLOCK_MONOTONIC, &child_start)==-1){perror("clock_gettime start impossible"); exit(EXIT_FAILURE);} // On prend l'heure de début du processus fils
		
		
		pid = fork(); //création d'un processus fils qui va executer la commande
		
		if(pid<0){perror("fork impossible");exit(EXIT_FAILURE);}
		
		else if(pid != 0){ //father code
			wait(&status); // attente de la fin du processus fils 
			if(clock_gettime(CLOCK_MONOTONIC, &child_stop)==-1){perror("clock_gettime stop impossible"); exit(EXIT_FAILURE);} // On prend l'heure de fin du processus fils
			
			exit_code(status,child_start,child_stop); // affiche le code de retour du fils
			free_cmd(cmd,&cmd_size); // On libère la mémoire  alouée à la commande
			
		}
		else{			   // child code
			full_exec(cmd,&cmd_size);//executer la commande saisie sur le terminal
			perror(" impossible d'executer la commande");
			exit(EXIT_FAILURE);
		}
		strcat(buffer_retour,prompt+9); //ajout des derniers caratères du prompt, à partir du crochet fermé
		write(STDOUT_FILENO,buffer_retour,PROMPT_SIZE); //afficher le message de retour 
		memset(buffer_retour,0,sizeof(buffer_retour));  // clear le buffer
	}
	exit(EXIT_SUCCESS);
}


void exit_code(int status, struct timespec start, struct timespec stop){
	char code_fils[PROMPT_SIZE];
	
	//code de retour : 
	
	if (WIFEXITED(status)){ // tester si le processus fils s'est terminé normalement 
		sprintf(code_fils,"exit:%d",WEXITSTATUS(status)); // récupérer la valeur de sortie du fils
	}
	else if(WIFSIGNALED(status)){ //tester si le processus fils s'est fini à cause d'un signal
		sprintf(code_fils,"sign:%d",WTERMSIG(status)); //récuperer le numéro du signal qui à causé la fin du fils 
	}
	strcat(buffer_retour, code_fils);
	
	//temps d'exécution : 
	
	if (stop.tv_nsec < start.tv_nsec){ //supprimer un potentielle temps négatif qui pourrait apparaitre lorsque l'on va soustraire deux temps
		stop.tv_nsec += 1.0e9;
		stop.tv_sec--;
	}
	
	if((stop.tv_sec-start.tv_sec)>=1){ // afficher le temps en seconde si celui ci dépasse 1seconde 
		sprintf(code_fils,"|%fs",(stop.tv_sec - start.tv_sec)+(stop.tv_nsec - start.tv_nsec)/1.0e9);
	}
	else{ // sinon afficher le temps en ms
		sprintf(code_fils,"|%dms", (int) ((stop.tv_nsec-start.tv_nsec)/1.0e6)); // on récupère la valeur du temps d'exécution en ms
	}
	
	strcat(buffer_retour, code_fils);
	
	
}



void free_cmd(char** word_array,int* cmd_size){
	int i =0;
	for(i=0;i<*cmd_size; i++){  // On libère bien chaque élément du tableau avant de liberer le tableau lui-même 
		free(word_array[i]);
	}
	free(word_array);
	*cmd_size = 0;
}



char** split_cmd(char* cmd_buffer, int* cmd_size){
	char** word_array=0;
	int  i=0; 
	int j;
	int nb_mots = 0;
	char* mot;
	int taille_cmd = strlen(cmd_buffer);
	
	// Compter le nombre d'espace pour déterminer le nombre de mots. 
	for(i=0; i<taille_cmd; i++){
		if(cmd_buffer[i]== ' '){nb_mots++;}
	}
	
	
	word_array = malloc((nb_mots+1)*sizeof(char*)); 
	
	if(word_array==NULL){perror("Malloc char** impossible"); exit(EXIT_FAILURE);}
	
		/* Ensuite on parcourt la commande en remplissant mot par mot le tableau de mots sachant que strtok coupe et colle le mot qu'il lit de cmd_buffer vers mot.  */
		
	mot = strtok(cmd_buffer, separators); //on récupère le premier mot de la commande 
	j=0;
	while(mot != NULL){
		word_array[j] = malloc(strlen(mot)*sizeof(char));
		strcpy(word_array[j],mot);
		j++;
		mot = strtok(NULL, separators);
	}
	
	*cmd_size = nb_mots+1; // on renvoit le nombre d'arguments de la commande
	return word_array;
	
}



void full_exec(char** word_array, int* cmd_size){
	FILE* file;
	int i =0;
	int chevron=0; // cet entier vaudra 1 si on a un '<' et 2 si on a un '>' dans la commande
	
	for(i=0; i<*cmd_size;i++){
		if(strcmp(word_array[i],"<")==0){ //détection d'un < dans la commande
			chevron = 2;
			file = fopen(word_array[i+1],"r"); // on ouvre le fichier donné en commande qui va servir d'entrée 
			if(file == NULL){perror("impossible d'ouvrir le fichier");exit(EXIT_FAILURE);}
			dup2(fileno(file),STDIN_FILENO); // on ferme STDIN_FILENO et on copie le descripteur de fichier de "file" dessus. On redirige ainsi l'entrée du shell sur le fichier. 
			fclose(file);
		}
		if(strcmp(word_array[i],">")==0){ //détection d'un > dans la commande
			chevron = 1;
			file = fopen(word_array[i+1],"w"); // on ouvre le fichier donné en commande qui va servir de sortie
			if(file == NULL){perror("impossible d'ouvrir le fichier");exit(EXIT_FAILURE);}
			dup2(fileno(file),STDOUT_FILENO); // on ferme STDOUT_FILENO et on copie le descripteur de fichier de "file" dessus. On redirige ainsi la sortie du shell sur le fichier. 
			fclose(file);
		}
		if(chevron>0){
			word_array[i] = NULL; // On supprime les éléments lié à la redirection afin d'exécuter la commande sans problème
			*cmd_size--;
		}
	}
	
	execvp(word_array[0],word_array);	
}




