#include <stdio.h> 
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <string.h> 
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#define DELIM "\t\n"

// This code is given for illustration purposes. You need not include or follow this strictly. Feel free to write better or bug free code. This example code block does not worry about deallocating memory. You need to ensure memory is allocated and deallocatedproperly so that your shell works without leaking memory. //

//void getcommand(char *prompt, char *args[], int *backgroud);
int getcommand(char *prompt, char *args[], int *background, char *history[], int *numcom, char *input)
{
	int length, i = 0, j = 0, k = 0;
	char *token, *loc; 
	char *line;

	//size_t used when sure that return value can't be negative  
	size_t size = 0;
	
	printf("%s", prompt);
	length = getline(&line, &size, stdin);

	//keep a copy of the input
	strcpy(input,line);
	
	//if empty input 
	if(length <= 0) { exit(-1);}
	
	//check if there is &, ie. background is specified
	if((loc = index(line,'&')) != NULL) //checks for the 1st occurence of & in buffer
	{
		*background = 1; //the bg is specified, we need to return this 
		*loc = ' '; 
	}
	else *background = 0; 
	
	if(*numcom < 10)
	{ //if less than 10 add to history 
        	history[*numcom] = strdup(line);
    	} 
	else
	{ //if more than 10 commands have been made, need to push
           	for(j=0; j<9; j++)
		{
                	history[j] = history[j+1];
            	}
            	history[9] = strdup(line); 
        }
	while ((token = strsep(&line, " \t\n")) != NULL) 
	{
        	for (j = 0; j < strlen(token); j++)
		{
            		if (token[j] <= 32) token[j] = '\0';
		}
		if (strlen(token) > 0) args[i++] = token;
    	}

	args[i++] = NULL;
	return i;          
}

//free the commands
void setfree(char *args[])
{
    memset(args, 0, 255);


}

int main(void) 
{
	char input[] = "";
	char *args[1024];
	char *history[10];
	int numcom = 0; //will hold number of commands 
	int bg, in;
	int i = 0, j = 0, k = 0;
	pid_t pid, pid1, pid2;
	int fd[2];
	char cwd[1024];
	int status; 
		
	in = getcommand("\n>> ", args, &bg, history, &numcom, input);
	
	int fp = -1;

	char *token, *loc, *tokn, *tokn1, *tokn2, *tokn3;
        char *line;

	while(1) 
	{
		numcom++;
	
		args[in] = '\0'; // in gets the number of commands that args has 
	 	
		/** what does args look like?
		for (i=0; i<in; i++)
		{
            		printf("\nArg[%d] = %s", i, args[i]);
		}**/
		
		 //printf("\n\n");

		//exit command -> working 
		if(strcmp(args[0] , "exit") == 0)
		{
			printf("\n\n");
			exit(0);
		}

		//history command -> working
		else if(strcmp(args[0], "history") == 0)
		{
			numcom--; //is history a command that should be counted?
			printf("\n HISTORY:recent commands");
			printf("\n Number of commands executed: %d\n", numcom);
			
			if(numcom <= 10)
			{//less than 10 commands 
                		for (k=0; k<numcom; k++)
				{
                    			printf("%d. %s", (k+1), history[k]);
				}
		         }
            		else
			{ //if there are more than 10 commands
                		for (k = 10; k > 0; k--)
				{
                    			printf("%d. %s", (numcom-k+1) , history[10-k]);
				}
                   
            		}
		}	

		//pwd command -> working
		else if(strcmp(args[0], "pwd") == 0)
		{
			if(getcwd(cwd,sizeof(cwd)) != NULL)
			{
				fprintf(stdout, "Current working directory: %s\n", cwd);
			}
		}

		//cd command -> working 
		else if(strcmp(args[0], "cd") == 0)
		{
			if(args[1] == NULL)
			{
				printf("Expected directory to switch into! \n");
			}
			else
			{
				if(chdir(args[1]) != 0) 
				{
				printf("Sucessful switch\n");
				}
			}
		}
		
		//run history command -> working 
		else if(!args[1])
                { 
			numcom--;			

                        if(numcom <10)
                        {
                          	line = strdup(history[numcom-1]);
        		       	printf("\nMost recent command to be executed: %s", line);
                	}
			else
			{
                    		line = strdup(history[9]);
                   		printf("\nMost recent command to be executed: %s", line);
                	}

               		//get the last argument
                	i=0;
                	while ((token = strsep(&line, " \t\n")) != NULL) 
			{
                    		for(j=0; j < strlen(token); j++)
				{
                         		if(token[j] <= 32) token[j] = '\0';
				}
                        	if(strlen(token) > 0) args[i++] = token;
			}
	
                	args[i++]= NULL;
		
			//run the commands 
              		pid = fork(); 
        		if (pid < 0) 
			{ //if fork fails
                		printf("The fork failed!\n"); 
                		exit(1); //error exit
            		}
            		else if (pid == 0) 
			{
                		if (execvp(*args, args) < 0) 
				{     
                    			printf("Not in history log.\n"); 
                    			exit(1); 
                		} 
            		}
			
			//add command to histor
                	numcom++;
                	for(j=0; j<9; j++)
                	{
                        	history[j] = history[j+1];
                	}
		} 
		
		//output redirection command
		else if(strstr(input, " > ") != NULL)
		{
			pid = fork();
			
			if(pid < 0) return -1;

			else if(pid == 0)
			{
				fp = open(args[2], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
				if(fp < 0) return -1; 
				else
				{	
                               		dup2(fp,1); //file becomes stdout 
					execvp(args[0],args);
					printf("The process failed!");
				}
				close(fp);
			}
                        return -1;
		}
		
		/**
		Whatever is written to fd[1] will be read from fd[0].

**/
		                                                                                    
		//pipe command 
		else if(strstr(input, "|") != NULL)
		{
			//make 2 child processes and use fd to pass stuff in between them
			
			//child process 1 
			pid1 = fork();
			
			//if fork doesn't work
			if(pid1 < 0) return -1;

			//this is the child that sends its output to the pipe
			else if(pid1 == 0)
			{
				close(1); //close stdout
				dup2(fd[1],1); //this becomes stdout 
				args[i++] = NULL;
				execvp(args[0],args);
				printf("The first child process failed!");
				return -1;
			}
			
			//child process 2
			pid2 = fork();
	
			//if fork doesn't work
			if(pid2 < 0) return -1;

			//this is the child that accepts input fron the pipe
			else if(pid2 == 0)
			{	
				close(0); //close stdin
				dup2(fd[0],0); //this becomes stdin
				args[i++] = NULL;
				execvp(args[2],args);
				printf("The second child process failed!");
				return -1;
			}
			
			//close the file descriptors 	
			close(fd[0]);
			close(fd[1]);
			
			//wait for the child processes
			waitpid(pid1,NULL,0);
			waitpid(pid2,NULL,0);

			return 0;
		}

		else
		{
			pid = fork();
			if(pid == 0)
                	{
       	                	//This is the child process
                        	if(execvp(*args,args)<0)
				{
					printf("The command failed!\n"); 
                   			exit(1); 
				}
                	} 
                	
                }
	

		if(bg)
		{
		 //do jobs and fg	
		}
		else
		{
			wait(&status) != pid;
		}
		bg = 1; 
		setfree(args); //free args to take in the next command 
	        getcommand("\n>> ", args, &bg, history, &numcom, input); //get the next command
	}
} 
