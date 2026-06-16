/* le fichier .h : 
#ifndef MICROSHELL_H
# define MICROSHELL_H

#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#endif
*/

#include "microshell.h"
/*
** Affiche un message d'erreur sur STDERR, caractere par caractere.
** On utilise write car printf est interdit.
** Retourne toujours 1.
** Ce 1 permet d'enchainer plusieurs appels avec l'operateur virgule
** et de renvoyer directement le resultat dans un return.
*/
int err(char *str)
{
	while (*str)				// Tant que le caractere courant n'est pas '\0'
		write(2, str++, 1);		// Ecrit 1 octet sur fd 2 (STDERR), puis avance str
	return 1;					// Retourne 1 pour pouvoir enchainer : return err("..."), err("...")
}

/*
** Gere le builtin cd.
** Cas 1 : mauvais nombre d'arguments (i != 2).
**   -> "error: cd: bad arguments\n"
** Cas 2 : chdir echoue (chemin invalide).
**   -> "error: cd: cannot change directory to <chemin>\n"
** chdir est exclu de la regle "error: fatal" : son erreur
** est geree ici avec un message specifique.
*/
int cd(char **argv, int i)
{
	if (i != 2)										// cd a besoin d'exactement 1 argument : "cd <chemin>"
		return err("error: cd: bad arguments\n");
	else if (chdir(argv[1]) == -1)					// Tente de changer de repertoire ; -1 = echec
		return err("error: cd: cannot change directory to "), err(argv[1]), err("\n");
		// Affiche le message en 3 morceaux car err() ne gere qu'une chaine a la fois
	return 0; // cd a reussi
}
/*
** Execute une commande, avec ou sans pipe.
**
** 1. has_pipe : vrai si la commande est suivie de "|".
**    Si oui, on cree un pipe : fd[0] = lecture, fd[1] = ecriture.
**
** 2. fork() :
**    - Enfant : coupe argv a la position i (argv[i] = 0)
**      pour ne garder que la commande courante.
**      Si pipe, redirige stdout (fd 1) vers fd[1] avec dup2,
**      puis ferme fd[0] et fd[1] (deja dupliques, inutiles).
**      Lance execve. Si execve echoue :
**      "error: cannot execute <programme>\n"
**
**    - Parent : attend la fin de l'enfant avec waitpid.
**      Si pipe, redirige stdin (fd 0) vers fd[0] avec dup2,
**      pour que la PROCHAINE commande lise ce que cette
**      commande a ecrit. Ferme ensuite fd[0] et fd[1].
**
** 3. Toute erreur sur pipe, fork, dup2 ou close
**    -> "error: fatal\n" (sauf execve, gere a part).
**
** 4. Retour : code de sortie de l'enfant
**    (WIFEXITED && WEXITSTATUS), utilise comme code
**    de retour final du programme.
*/
int exec(char **argv, char **env, int i)
{
	int fd[2];   // fd[0] = extremite lecture du pipe, fd[1] = extremite ecriture
	int status;  // Contiendra le statut de sortie de l'enfant apres waitpid
	int has_pipe = argv[i] && !strcmp(argv[i], "|"); // Vrai si le token apres la commande est "|"

	if (has_pipe && pipe(fd) == -1)  // Cree le pipe seulement si necessaire ; -1 = echec
		return err("error: fatal\n");

	int pid = fork(); // Duplique le processus ; retourne 0 dans l'enfant, le PID dans le parent
	if (!pid)         // Bloc enfant (pid == 0)
	{
		argv[i] = 0; // Coupe le tableau argv ici pour que execve ne voie que la commande courante

		if (has_pipe && (dup2(fd[1], 1) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
		// dup2(fd[1], 1) : redirige STDOUT vers l'entree ecriture du pipe
		// close(fd[0]) : ferme la lecture (inutile dans l'enfant)
		// close(fd[1]) : ferme le fd original devenu inutile apres dup2
			return err("error: fatal\n");

		execve(*argv, argv, env); // Remplace le processus enfant par la commande ; ne revient pas si succes
		return err("error: cannot execute "), err(*argv), err("\n"); // Execve a echoue
	}

	waitpid(pid, &status, 0); // Le parent attend la fin de l'enfant et recupere son statut

	if (has_pipe && (dup2(fd[0], 0) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
	// dup2(fd[0], 0) : redirige STDIN vers la sortie lecture du pipe
	// La prochaine commande lira ainsi ce que l'enfant a ecrit
	// close() : ferme les fd du pipe devenus inutiles apres dup2
		return err("error: fatal\n");

	return WIFEXITED(status) && WEXITSTATUS(status);
	// WIFEXITED : verifie que l'enfant s'est termine normalement (pas par un signal)
	// WEXITSTATUS : extrait le code de retour reel de l'enfant
}

/*
** Point d'entree.
**
** Boucle externe :
**   argv[i] && argv[++i] avance i de 0 a 1, ce qui saute
**   "./microshell" et pointe sur le premier vrai argument.
**   Ensuite argv += i remet argv au debut de la commande
**   courante, et i repart a 0.
**
** Boucle interne :
**   i avance tant que argv[i] n'est pas "|" ni ";".
**   A la fin, i = nombre d'arguments de la commande courante,
**   et argv[i] est soit "|", soit ";", soit NULL.
**
** Selon le cas :
**   - "cd"      -> cd(argv, i)
**   - i != 0     -> exec(argv, env, i)
**   - i == 0     -> rien (separateur seul, ex: ";;")
**
** Le tour de boucle suivant recommence sur argv += i,
** donc sur argv[i] lui-meme (le "|" ou ";"), ce qui permet
** au while exterieur de passer au-dela.
*/
int main(int argc, char **argv, char **env)
{
	int i = 0;									// Indice courant dans argv
	int status = 0;								// Code de retour final du programme

	if (argc > 1)								// Ne fait rien si aucun argument n'est passe
	{
		while (argv[i] && argv[++i])			// Avance i de 0 a 1 pour sauter "./microshell" lui-meme
		{
			argv += i;							// Deplace le pointeur argv au debut de la commande courante
			i = 0;								// Repart a 0 pour parcourir les tokens de cette commande

			while (argv[i] && strcmp(argv[i], "|") != 0 && strcmp(argv[i], ";") != 0)
				i++;							// Avance tant que le token n'est ni "|" ni ";" ni NULL
												// A la fin : i == nombre d'arguments de la commande, argv[i] est le separateur

			if (!strcmp(*argv, "cd"))			// Si la commande est "cd", on gere en interne
				status = cd(argv, i);
			else if (i)							// Sinon, si la commande n'est pas vide (evite ";;")
				status = exec(argv, env, i);
												// Si i == 0 et commande != "cd", c'est un separateur seul : on ne fait rien
		}
	}
	return status;								// Retourne le code de sortie de la derniere commande executee
}