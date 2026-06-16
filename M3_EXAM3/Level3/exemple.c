main()
i = 0
status = 0

while (argv[0] = "./microshell" && argv[++i] = argv[1] = "/bin/ls")
{
--> argv += 1
    argv pointe maintenant sur :
    [0] "/bin/ls"
    [1] "|"
    [2] "/usr/bin/grep"
    [3] "microshell"
    [4] ";"
    [5] "/bin/echo"
    [6] "i"
    [7] "love"
    [8] "my"
    [9] "microshell"
    [10] NULL
    i = 0

    while (argv[0] = "/bin/ls"
        && strcmp("/bin/ls", "|") != 0
        && strcmp("/bin/ls", ";") != 0)
        i++ → i = 1

    while (argv[1] = "|"
        && strcmp("|", "|") == 0)
    STOP → i = 1

    if (!strcmp("/bin/ls", "cd")) → faux
    else if (i = 1)
--> status = exec(argv, env, 1)

    ========== exec() ==========

    has_pipe = argv[1] = "|" && strcmp("|", "|") == 0 → has_pipe = 1

    pipe(fd) → fd[0] = 3 (lecture)
               fd[1] = 4 (écriture)

    pid = fork()

    ---- ENFANT (pid == 0) ----
    argv[1] = NULL → argv = { "/bin/ls", NULL }

    has_pipe = 1 :
    dup2(fd[1], 1) → stdout redirigé vers fd[1] (écriture pipe)
    close(fd[0])   → ferme lecture (inutile ici)
    close(fd[1])   → ferme fd original (déjà dupliqué)

    execve("/bin/ls", {"/bin/ls", NULL}, env)
    → /bin/ls s'exécute, écrit sa sortie dans le pipe

    ---- PARENT ----
    waitpid(pid, &status, 0) → attend la fin de /bin/ls

    has_pipe = 1 :
    dup2(fd[0], 0) → stdin redirigé vers fd[0] (lecture pipe)
    close(fd[0])
    close(fd[1])

    return status
    ============================
}

while (argv[1] = "|" && argv[++i] = argv[2] = "/usr/bin/grep")
{
--> argv += 2
    argv pointe maintenant sur :
    [0] "/usr/bin/grep"
    [1] "microshell"
    [2] ";"
    [3] "/bin/echo"
    [4] "i"
    [5] "love"
    [6] "my"
    [7] "microshell"
    [8] NULL
    i = 0

    while (argv[0] = "/usr/bin/grep"
        && strcmp("/usr/bin/grep", "|") != 0
        && strcmp("/usr/bin/grep", ";") != 0)
        i++ → i = 1

    while (argv[1] = "microshell"
        && strcmp("microshell", "|") != 0
        && strcmp("microshell", ";") != 0)
        i++ → i = 2

    while (argv[2] = ";"
        && strcmp(";", "|") != 0
        && strcmp(";", ";") == 0)
    STOP → i = 2

    if (!strcmp("/usr/bin/grep", "cd")) → faux
    else if (i = 2)
--> status = exec(argv, env, 2)

    ========== exec() ==========

    has_pipe = argv[2] = ";" && strcmp(";", "|") != 0 → has_pipe = 0

    pas de pipe()

    pid = fork()

    ---- ENFANT (pid == 0) ----
    argv[2] = NULL → argv = { "/usr/bin/grep", "microshell", NULL }

    has_pipe = 0 → pas de dup2

    execve("/usr/bin/grep", {"/usr/bin/grep", "microshell", NULL}, env)
    → grep lit stdin (= lecture du pipe de /bin/ls)
    → affiche : "microshell"

    ---- PARENT ----
    waitpid(pid, &status, 0)

    has_pipe = 0 → pas de dup2

    return status
    ============================
}

while (argv[2] = ";" && argv[++i] = argv[3] = "/bin/echo")
{
--> argv += 3
    argv pointe maintenant sur :
    [0] "/bin/echo"
    [1] "i"
    [2] "love"
    [3] "my"
    [4] "microshell"
    [5] NULL
    i = 0

    while (argv[0] = "/bin/echo"
        && strcmp("/bin/echo", "|") != 0
        && strcmp("/bin/echo", ";") != 0)
        i++ → i = 1

    while (argv[1] = "i" ...)    i++ → i = 2
    while (argv[2] = "love" ...) i++ → i = 3
    while (argv[3] = "my" ...)   i++ → i = 4
    while (argv[4] = "microshell" ...) i++ → i = 5

    while (argv[5] = NULL)
    STOP → i = 5

    if (!strcmp("/bin/echo", "cd")) → faux
    else if (i = 5)
--> status = exec(argv, env, 5)

    ========== exec() ==========

    has_pipe = argv[5] = NULL → has_pipe = 0

    pas de pipe()

    pid = fork()

    ---- ENFANT (pid == 0) ----
    argv[5] = NULL → (déjà NULL, rien ne change)
    argv = { "/bin/echo", "i", "love", "my", "microshell", NULL }

    execve("/bin/echo", {"/bin/echo", "i", "love", "my", "microshell", NULL}, env)
    → affiche : "i love my microshell"

    ---- PARENT ----
    waitpid(pid, &status, 0)

    return status
    ============================
}

while (argv[5] = NULL)
STOP → on sort du while extérieur

return status → 0