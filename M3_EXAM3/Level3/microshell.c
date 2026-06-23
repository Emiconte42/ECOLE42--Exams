# include "microshell.h"
/*x
int err(char *str)
{
	while (*str)
		write(2, str++, 1); // ne pas oublier le ++ sans le *
}

int	cd(char **argv, int i)
{
	if (i != 2)
		return err("error : cd : bad arguments\n");
	else if (chdir(argv[1]) == -1)
		return (err("error : cd : cannot change directory to"), err(argv[1]), err("\n"));
	return 0;
}

int exec(char **argv, char **envp, int i)
{
	int	fd[2]; // lecture[0] et ecriture[1]
	int	status; // status de sortie
	int	has_pipe; // si suivi d'un pipe
	int pid;

	has_pipe = argv[i] && strcmp(argv[i], "|") == 0; // ou !strcmp(argv[i], "|")
	if ( has_pipe && pipe(fd) == -1) // ne pas oublier le pipe
		return (err("error : fatal\n"));
	
	pid = fork();
	if (pid == 0)
	{
		argv[i] = 0;
		if (has_pipe && (dup2(fd[1], 1) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
			return (err("error : fatal\n"));
		execve(*argv, argv, envp); // attention a mettre * au premier et pas au second
		return (err("error : cannot execute"), err(*argv), err("\n"));
	}
	
	waitpid(pid, &status, 0);

	if (has_pipe && (dup2(fd[0], 0) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
		return (err("error : fatal\n"));
	
	return (WIFEXITED(status) && WEXITSTATUS(status));
}


int main (int argc, char ** argv, char **envp)
{
	int i;
	int	status;

	i = 0;
	status = 0;
	if (argc > 1)
	{
		while (argv[i] && argv[++i])
		{
			argv += i; // ne pas oublier que c'est i
			i = 0;
			while (argv[i] && strcmp(argv[i], "|") != 0 && strcmp(argv[i], ";") != 0)
				i++;
			if (!strcmp(*argv, "cd")) // ne pas oublier le ! et le *
					status = cd(argv, i);
			else if (i)
					status = exec(argv, envp, i);
			}
		}
	return status;
}
*/

int err(char *str)
{
	while (*str)
		write (2, str++, 1);
	return 1;
}

int	cd(char **argv, int i)
{
	if (i != 2)
		return (err("error : cd : bad arguments"));
	else if (chdir(argv[1]) == -1)
		return (err("error : cd : cannot change directory to"), err(argv[1]), err ("\n"));
	return 0;
}
int exec(char **argv, char **envp, int i)
{
	int	fd[2];
	int	status;
	int	has_pipe;
	int pid;

	has_pipe = argv[i] && !strcmp(argv[i], "|");
	if ( has_pipe && pipe(fd) == -1)
		return (err("error : fatal\n"));
	
	pid = fork();
	if (pid == 0)
	{
		argv[i] = 0;
		if (has_pipe && (dup2(fd[1], 1) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
			return (err("error : fatal\n"));
		execve(*argv, argv, envp);
		return (err("error : cannot execute"), err(*argv), err("\n"));
	}
	
	waitpid(pid, &status, 0);

	if (has_pipe && (dup2(fd[0], 0) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
		return (err("error : fatal\n"));
	
	return (WIFEXITED(status) && WEXITSTATUS(status));
}
int main (int argc, char **argv, char **envp)
{
	int	i;
	int	status;

	i = 0;
	status = 0;
	if (argc > 1)
	{
		while (argv[i] && argv[++i])
		{
			argv += i;
			i = 0;

			while (argv[i] && strcmp(argv[i], "|") && strcmp(argv[i], ";"))
				i++;
			if (!strcmp(*argv, "cd"))
				status = cd(argv, i);
			else if (i)
				status = exec(argv, envp, i);
		}
	}

	return status;
}