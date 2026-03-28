#include "main.h"
#define MAXSIZE 1024

//2D array for storing external commands
char *ext_cmd[153]={NULL};
//2D array for storing Internal commands
char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",	"set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source","exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", "jobs", "fg", "bg", NULL};
int pid;//Variable for pid
int last_status;//Variable to store last process status
int stop_ct=0;//Variable to store number of stopped process count
char prompt_str[20]="minishell$";//Default minishell prompt
char buffer[MAXSIZE];//Input buffer
struct stop stopped[25];//Array of structure to store details of stopped processes

//Handler function to change properties of Signals
void handler(int signum)
{
    //Handler for SIGINT signal
    if(signum==SIGINT)
    {
        if(pid==0)
        {
            printf("\n"ANSI_COLOR_GREEN"%s: "ANSI_COLOR_RESET, prompt_str);
            fflush(stdout);
            return;
        }
    }
    else if(signum==SIGTSTP)//Handler for SIGTSTP signal
    {
        if(pid==0)
        {
            printf("\n"ANSI_COLOR_GREEN"%s: "ANSI_COLOR_RESET, prompt_str);
            fflush(stdout);
            return;
        }
        else if(pid>0)
        {
            stopped[stop_ct].pid=pid;
            strcpy(stopped[stop_ct].cmd,buffer);
            stop_ct++;
            printf("\n[%d]   Stopped            %s\n",stop_ct,stopped[stop_ct-1].cmd);
        }
    }
    else if(signum==SIGCHLD)//Handler for SIGCHLD signal
    {
        int status;
        waitpid(-1,&status,WNOHANG);
        if(WIFEXITED(status))
        {
            last_status=WEXITSTATUS(status);
        }
    }
}

//Function to extract external commands from external file
void extract_external_commands(char **external_commands)
{
    char buffer[100];
    FILE *fp=fopen("external.txt","r");//Opening file
    if(fp==NULL)
    {
        perror("fopen");
        return ;
    }
    int i=0;
    //Extracting commands till it reaches end of file
    while(fgets(buffer,sizeof(buffer),fp)!=NULL)
    {
        int len=strlen(buffer);
        if(buffer[len-1]=='\n')
        {
            buffer[len-2]='\0';
            len=len-2;
        }
        external_commands[i]=(char*)malloc((len+1)*sizeof(char));
        strcpy(external_commands[i],buffer);
        i++;
    }
    external_commands[i]=NULL;
    fclose(fp);
}

//Function to extract only the command without option from the input buffer 
char *get_command(char *input_string)
{
    int len=strlen(input_string);
    char *cmd=malloc((len+1)*sizeof(char));
    strcpy(cmd,input_string);
    for(int i=0;i<len;i++)
    {
        if(cmd[i]==' ')
        {
            cmd[i]='\0';
            break;
        }
    }
    return cmd;
}

//Function to check whether command is internal or external
int check_command_type(char *command)
{
    //Loop to check whether command is internal or not
    for(int i=0;builtins[i]!=NULL;i++)
    {
        if(strcmp(command,builtins[i])==0)
        {
            return BUILTIN;//Returning if its builtin
        }
    }
    //Loop to check whether command is external or not
    for(int i=0;ext_cmd[i]!=NULL;i++)
    {
        if(strcmp(command,ext_cmd[i])==0)
        {
            return EXTERNAL;//Returning if its external
        }
    }
    return NO_COMMAND;//Returning if its not a command
}

//Function to execute internal commands
void execute_internal_commands(char *input_string)
{
    //Condition to check if its cd and change directory
    if(strncmp(input_string,"cd",2)==0)
    {
        if(chdir(input_string+3)==-1)
        {
            perror("chdir");
            return;
        }
    }
    else if(strncmp(input_string,"pwd",3)==0)//Condition to print present directory
    {
        char path[MAXSIZE];
        if(getcwd(path,sizeof(path))!=NULL)
        {
            printf("%s\n",path);
        }
        else
        {
            perror("getcwd");
            return;
        }
    }
    else if(strncmp(input_string,"exit",4)==0)//Condition to terminate the minishell terminal
    {
        exit(0);
    }
    else if(strncmp(input_string,"echo",4)==0)//Condition for echo command
    {
        if(strncmp(input_string+5,"$SHELL",6)==0)//Checking option to print executable path of minishell
        {
            char *shell=getenv("SHELL");
            printf("%s\n",shell);
        }
        else if(strncmp(input_string+5,"$$",2)==0)//check option to print pid of terminal
        {
            printf("%d\n",getpid());
        }
        else if(strncmp(input_string+5,"$?",2)==0)//Check option to print last process status
        {
            printf("%d\n",last_status);
        }
        else//Invalid option for echo
        {
            perror("Invalid option\n");
        }
    }
    else if(strncmp(input_string,"jobs",4)==0)//Condition to display stopped processes
    {
        for(int i=0;i<stop_ct;i++)
        {
            printf("[%d]   Stopped            %s\n",i+1,stopped[i].cmd);
        }
    }
    else if(strncmp(input_string,"fg",2)==0)//Condition to move a stopped process to background 
    {
        //Check whether there are processes stopped are there or not
        if(stop_ct==0)
        {
            printf("fg: current: no such jobs\n");
            return ;
        }
        else
        {
            printf("%s\n",stopped[stop_ct-1].cmd);
        }
        int status;
        //Continuing the process foreground
        kill(stopped[stop_ct-1].pid,SIGCONT);
        waitpid(stopped[stop_ct-1].pid,&status,WUNTRACED);
        //Removing the stopped process
        stop_ct--;
        if(WIFEXITED(status))
        {
            last_status=WEXITSTATUS(status);
        }
    }
    else if(strncmp(input_string,"bg",2)==0)//Condition to move a stopped process to foreground 
    {
        //Check whether there are processes stopped are there or not
        if(stop_ct==0)
        {
            printf("bg: current: no such jobs\n");
            return ;
        }
        //Continuing the process background
        kill(stopped[stop_ct-1].pid,SIGCONT);
        //Removing the stopped process
        stop_ct--;
    }
    return;
}

//Function to execute external commands
void execute_external_commands(char *input_string)
{
    char * args[50];
    int argc=0,pip_ct=0;
    //Converting 1D array input to 2D Array input to implement command using strtok
    char *token=strtok(input_string," ");
    while(token!=NULL)
    {
        args[argc]=token;
        token=strtok(NULL," ");
        argc++;
    }
    args[argc]=NULL;
    //Checking if there is pipe present or not
    for(int i=0;i<argc;i++)
    {
        if(strcmp(args[i],"|")==0)
        {
            pip_ct++;
        }
    }
    //Execute command directly if there is no pipe
    if(pip_ct==0)
    {
        execvp(args[0],args);
    }
    else//Execute command using child process if it has n pipes
    {
        int cmd_ind[argc],cmd_ct=1;
        //Checking for invalid pipe position
        if(strcmp(args[0],"|")!=0)
        {
            cmd_ind[0]=0;
        }
        else
        {
            perror("pipe");
            return;
        }
        for(int i=0;i<argc;i++)
        {
            if(strcmp(args[i],"|")==0)
            {
                //Checking for invalid pipe position
                if(i+1<argc && strcmp(args[i+1],"|")==0)
                {
                    perror("pipe");
                    return;
                }
                //Checking for invalid pipe position
                if(i==0 || i==argc-1)
                {
                    perror("pipe");
                    return;
                }
                args[i]=NULL;
                //Getting command and option index
                cmd_ind[cmd_ct]=i+1;
                //Getting count of commands
                cmd_ct++;
            }
        }
        //Pipe implementation for multiple command
        int old_pip=-1,pid2;
        for(int i=0;i<cmd_ct;i++)
        {
            int pip[2];
            //Creating n-1 pipe for n command
            if(i!=cmd_ct-1)
            {
                if(pipe(pip)==-1)
                {
                    perror("Pipe");
                    return ;
                }
            }
            //Creating child processes
            pid2=fork();
            if(pid2>0)//Parent process to redirect input from pipe to child
            {
                if(old_pip!=-1)
                {
                    close(old_pip);
                }
                if(i!=cmd_ct-1)
                {
                    close(pip[1]);
                    old_pip=pip[0];
                }
            }
            else if(pid2==0)//Child process to execute command and store it to pipe
            {
                if(old_pip!=-1)
			    {
				    dup2(old_pip,0);
				    close(old_pip);
			    }
                if(i!=cmd_ct-1)
                {
                    close(pip[0]);
				    dup2(pip[1],1);
				    close(pip[1]);
                }
                if((execvp(args[cmd_ind[i]],args+cmd_ind[i]))==-1)
                {
                    perror("execvp failed");
			        _exit(1);
                }
                exit(0);
            }
            else
		    {
			    perror("fork failed");
			    _exit(1);
		    }
        }
        //Waiting for child process to clear resources and get last process status
        for(int i = 0; i < cmd_ct; i++)
        {
		    int status;
            waitpid(pid2,&status,0);
            if(WIFEXITED(status))
            {
                last_status=WEXITSTATUS(status);
            }
        }
    }
}

//Function to implement minishell terminal
void scan_input()
{
    //Extracting external commands from file
    extract_external_commands(ext_cmd);
    while(1)
    {
        //clear input buffer to check whether entered input is empty or not 
        buffer[0]='\0';
        printf(ANSI_COLOR_GREEN"%s: "ANSI_COLOR_RESET,prompt_str);
        __fpurge(stdin);
        scanf("%[^\n]",buffer);//Scanning input 
        //to continue if the input is empty
        if(strlen(buffer)==0)
        {
            continue;
        }
        //checking whether the input is PS1 Env variable to make minishell prompt customizable
        if(strncmp(buffer,"PS1=",4)==0)
        {
            if(strchr(buffer,' ')==NULL)//Check whether there is space in the input
            {
                int len=strlen(buffer);
                if(len==4)
                {
                    printf("INVALID COMMAND\n");
                }
                else
                {
                    int j=0;
                    //Extracting the new prompt from the input by removing "PS1="
                    for(int i=4;i<len;i++)
                    {
                        prompt_str[j]=buffer[i];
                        j++;
                    }
                    prompt_str[len-4]='\0';
                }
            }
            else
            {   
                printf("INVALID COMMAND\n");
            }
        }
        else//Condition to check whether any other command is from user
        {
            //Function call to extract only ther command 
            char *cmd=get_command(buffer);
            //Function call to check the command type
            int cmd_type=check_command_type(cmd);
            //Condition to check whether command is internal
            if(cmd_type==BUILTIN)
            {
                //Function call to execute the internal commands
                execute_internal_commands(buffer);
            }
            else if(cmd_type==EXTERNAL)//Condition to check whether command is external
            {
                //Creating child process
                pid=fork();
                if(pid==0)
                {
                    //Re-assigning default properties of signals
                    signal(SIGINT,SIG_DFL);
                    signal(SIGTSTP,SIG_DFL);
                    //Function call to execute external commands
                    execute_external_commands(buffer);
                    exit(0);
                }
                else if(pid>0)
                {
                    //Waiting for child process to terminate and clear resources
                    int status;
                    waitpid(pid,&status,WUNTRACED);
                    //Updating last process status
                    if(WIFEXITED(status))
                    {
                        last_status=WEXITSTATUS(status);
                    }
                    //Re-initializing pid value to 0
                    pid=0;
                }
            }
            else if(cmd_type==NO_COMMAND)//Condition to chech whether the given command is valid
            {
                printf("Not a command\n");
            }
        }
    }
}

int main()
{
    //Clearing the terminal
    system("clear");
    //Registering signal to change properties
    signal(SIGINT,handler);
    signal(SIGTSTP,handler);
    signal(SIGCHLD,handler);
    //Function call to implement minishell
    scan_input();
    return 0;
}