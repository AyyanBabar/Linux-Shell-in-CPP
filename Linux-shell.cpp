#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
using namespace std;
void copy(char *carr, string s)
{
    int i = 0;
    for (i = 0; i < s.length(); i++)
    {
        carr[i] = s[i];
    }
    carr[i] = '\0';
}
void execute(char **argv)
{
    int pid = fork();
    if (pid > 0)
    {
        wait(NULL);
    }
    else if (pid == 0)
    {
        execvp(*argv, argv);
        cout << "Command failed";
        exit(0);
    }
    else
    {
        cerr << "Maslay in fork\n";
    }
}

void exepipes(char **argv)
{
    int **pipes = NULL;
    int i = 0;
    int n_pipes = 0;
    while (argv[i] || argv[i + 1])
    {
        if (argv[i] && argv[i][0] == '|')
        {
            n_pipes++;
        }
        i++;
    }
    int org[2];
    dup2(0, org[0]);
    dup2(1, org[1]);
    //cout << n_pipes << endl;
    pipes = new int *[n_pipes];
    for (int j = 0; j < n_pipes; j++)
    {
        pipes[j] = new int[2];
        pipe(pipes[j]);
        fcntl(pipes[j][0], F_SETFL, O_NONBLOCK);
    }
    i = 0;
    int pipe_id = 0;
    while (argv[i] || argv[i + 1])
    {
        int pid = fork();
        if (pid > 0)
        {
            wait(NULL);
        }
        else if (pid == 0)
        {
            int j = i;
            while (argv[j])
                j++;
            j++;
            if (argv[j] && argv[j][0] == '<')
            {
                int file = open(argv[j + 2], O_RDONLY);
                dup2(file, 0);
            }
            if (pipe_id)
                dup2(pipes[pipe_id - 1][0], 0);
            if (argv[j] && argv[j][0] == '>')
            {
                int file = open(argv[j + 2], O_CREAT | O_WRONLY | O_TRUNC, 00666);
                dup2(file, 1);
            }

            else if (pipe_id < n_pipes)
            {
                dup2(pipes[pipe_id][1], 1);
            }

            execvp(*(argv + i), (argv + i));
            cout << "Command failed";
            exit(0);
        }
        else
        {
            cerr << "Maslay in fork\n";
        }

        while (argv[i])
            i++;
        if ((!argv[i] && !argv[i + 1]) || ((!argv[i] && argv[i + 1][0] == '>')))
            break;
        i++;
        if (argv[i] && argv[i][0] != '>')
            i += 2;
        pipe_id++;
    }
    dup2(org[0], 0);
    dup2(org[1], 1);
}
int main()
{
    while (1)
    {
        string s;
        char cwd[1000];
        getcwd(cwd, sizeof(cwd));
        cout << cwd << ">>>>";
        getline(cin, s);
        bool pipes_present = false;
        if (s == "exit")
            break;
        else
        {
            string word = "";
            char *argv[50];
            int i = 0;
            char x;
            for (int j = 0; j < s.size(); j++)
            {
                x = s[j];
                if (x == ' ')
                {
                    argv[i] = new char[word.size() + 1];
                    copy(argv[i], word);
                    i++;
                    word = "";

                    while (s[++j] == ' ')
                        ;
                    j--;
                }
                else if (x == '|' || x == '>' || x == '<')
                {
                    if (word.size())
                    {
                        argv[i] = new char[word.size() + 1];
                        copy(argv[i], word);
                        i++;
                        word = "";
                    }
                    argv[i++] = NULL;
                    argv[i] = new char[2];
                    argv[i][0] = x;
                    argv[i][1] = '\0';
                    i++;
                    argv[i++] = NULL;
                    j++;
                    while (s[j] == ' ')
                        j++;
                    j--;
                    pipes_present = true;
                }
                else
                {
                    word = word + x;
                }
            }
            argv[i] = new char[word.size() + 1];
            copy(argv[i], word);
            i++;
            argv[i++] = NULL;
            argv[i] = NULL;
            if (strcmp(argv[0], "cd") == 0)
            {
                char temp[250];
                copy(temp, argv[1]);
                if (chdir(temp) == -1)
                    cout << "Operation failed\n";
            }
            else if (pipes_present)
            {
                exepipes(argv);
            }
            else
            {
                execute(argv);
            }
        }
    }
    return 0;
}