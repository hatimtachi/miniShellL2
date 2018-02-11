/* minishell.c

  Un shell elementaire (fork + exec et c'est tout)
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include "decouper.c"
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>
#include <dirent.h>

enum {
  MaxLigne = 1024,              // longueur max d'une ligne de commandes
  MaxMot = MaxLigne / 2,        // nbre max de mot dans la ligne
  MaxDirs = 100,		// nbre max de repertoire dans PATH
  MaxPathLength = 512,		// longueur max d'un nom de fichier
};

int decouper(char *, char *, char **, int);

int contient (char * mot, int c){
    int i;
    for (i=0; i < strlen(mot); i++){
        if (mot[i] == c)
            return 1;
    }
    return 0;
}
# define PROMPT "? "

int main(int argc, char * argv[]){
  char ligne[MaxLigne];
  char pathname[MaxPathLength];
  char * mot[MaxMot];
  char * dirs[MaxDirs];
  int i, tmp, cdir_,cpt=0,ei=1,res=0;
  char * dir;
  bool wait_ = true;
  struct dirent *d;
  DIR *dirss;

  /* Decouper UNE COPIE de PATH en repertoires */
  decouper(strdup(getenv("PATH")), ":", dirs, MaxDirs);

  /* Lire et traiter chaque ligne de commande */
  for(printf(PROMPT); fgets(ligne, sizeof ligne, stdin) != NULL; printf(PROMPT)){
    decouper(ligne, " \t\n", mot, MaxMot);
    if (mot[0] == NULL)            // ligne vide
      continue;

    //*****************
    // la commande cd
    //******************   
    if (strcmp(mot[0], "_cd") == 0) { 
      if (mot[1] == NULL) {
	       dir = getenv("home");
	       if (dir == 0) {
	          dir = "/home/";
	           }
      }
      else if (mot[2] != NULL) {
      	fprintf(stderr, "usage : %s [dir]\n", mot[0]);
      	continue;
      }
      else dir = mot[1];
         cdir_ = chdir(dir);
      if (cdir_ < 0) {
      	perror(dir);
      }
      continue;
    }
    //*****************
    // la commande ls
    //******************
     if ((strcmp(mot[0],"_ls")==0)||(strcmp(mot[0],"_l")==0)){ 
        if (mot[1] ==  NULL) {
          if((dirss=opendir("."))==NULL)
            return (0);
          while(d=readdir(dirss)){
            if(cpt<=5){
              fprintf(stderr,"%s\n",d->d_name );
              cpt++;
            }
            else{
              fprintf(stderr,"\n");
              cpt=0;
            }
          }
          fprintf(stderr,"\n");
        }
        continue;
      }
    //*****************
    // la commande echo
    //******************
    if(strcmp(mot[0],"_echo")==0){ 
        res=545454;
        while(mot[ei]!=NULL){
          if(mot[ei][0]=='$'){
            char *res;
            res=mot[ei]+1;
            if((res=getenv(res))!=NULL)
              fprintf(stderr,"%s\n",res);
             else
              fprintf(stderr,"%s\n",mot[ei]);
              }
          else{
            fprintf(stderr,"%s ",mot[ei]);
          }
          ei++;
        }
        fprintf(stderr, "\n" );
        continue;
      }     
    //*******************
    //la commande sleep
    //*******************
    if (strcmp(mot[0], "_sleep") == 0) { 
      if (mot[1] ==  NULL) {
          continue;
          }
      else if (mot[1] != NULL){
          sleep (atoi(mot[1]));
        continue;
        }
      }
      
 
    tmp = fork();               // lancer le processus enfant

    if (tmp < 0){
      perror("fork");
      continue;
    }
    
    if (tmp != 0){                    // parent : attendre la fin de l'enfant
      if(wait_)
	while(wait(NULL) != tmp)
      ;
      continue;
    }				      // enfant : exec du programme
    for(i = 0; dirs[i] != NULL; i++) {
      snprintf(pathname, sizeof pathname, "%s/%s", dirs[i], mot[0]);
      execv(pathname, mot);
    }
                                      // aucun exec n'a fonctionne
    fprintf(stderr, "%s: not found\n", mot[0]);
    exit(1);
  }
  
  printf("Bye\n");
  return 0;
 
}
