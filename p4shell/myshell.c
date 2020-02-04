#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define MAXLENGTH 512;

void myPrint(char *msg)
{
    write(STDOUT_FILENO, msg, strlen(msg));
}

void oh_no()
{
        char error_message[30] = "An error has occurred\n";
        myPrint(error_message);
        return;
}

int redirect(char* cmd)
{
    int i;
    int redir = 0;
    int adv = 0; 
    for(i=0; cmd[i]; i++)
    {
        if (cmd[i] == '>')
        {
            if(cmd[i+1] != '+')
                redir += 1;
            else
                adv += 1; 
        }
    }
    if(redir > 1 || adv > 1 || (redir && adv))
        return -1;
    else if(redir == 1 && adv == 0)
        return 1;
    else if(redir == 0 && adv == 1)
        return 2; 
    else 
        return 0;
}

int parseline(char *line, char **argv)
{
    char* buf;
    char* temp = strdup(line);
    int i = 0;
    while((buf = strtok(temp, ";\n\t")))
    {
        argv[i] = buf;
        temp = NULL;
        i++;
    }
    return i;
}

int parse_help(char *line, char **argv)
{
    char* buf;
    char* temp = strdup(line);
    int i = 0;
    while((buf = strtok(temp, " \n\t")))
    {
        argv[i] = buf;
        temp = NULL;
        i++;
    }
    return i;
}

int parse_cmd(char* cmd, char** argv, char** file)
{
    int argc;
    int redir = redirect(cmd);
   char* cmd_temp = strdup(cmd);
	char* buf;
	char* buf2; 
	
    if(redir == 0)
    {
        argc = parse_help(cmd_temp, argv); 
        file = NULL;
        argv[argc] = NULL;
    }
    else if(redir == -1)
    {
	oh_no();
        //char* redirneg = "redir = -1";
	//myPrint(redirneg); 
        argc = -1;
    }
    else
    {
        if(redir == 1)
        {
	        buf = (strtok(cmd_temp, ">\n\t"));
<<<<<<< .mine
		buf2 = (strtok(NULL, " >\n\t"));
||||||| .r72
		    buf2 = (strtok(NULL, ">\n\t"));
=======
		    buf2 = (strtok(NULL, " >\n\t"));
>>>>>>> .r77
        }
        if(redir == 2)
        {
            buf = (strtok(cmd_temp, ">+\n\t"));
	    	buf2 = (strtok(NULL, " >+\n\t"));
        }
        if(buf == NULL || buf2 == NULL)
        {
	    //char* nullbuff = "null buff";
	   //myPrint(nullbuff); 
		oh_no();
            return (-1);
	    }
           char* temp1 = strdup(buf);	    
	   argc = parse_help(temp1, argv);
	   file[0] = strdup(buf2); 
	   argv[argc] = NULL; 
    }
    return argc; 
}


void eval(int argc, char* cmd, char** argv, char** file)
{	
        char* temp = strdup(cmd);
        int redir = redirect(temp);
	//printf("redir = %d\n", redir);
        char* first = argv[0];
        char* second = argv[1];
//	char* filex = file[0];
//	printf("eval is being accessed, argc is %d, redir is %d, file[0] is %s and argv0 is %s and argv1 is %s\n",argc,redir,filex, first, second);  
        if((argc < 0) || first == NULL)
                return;
        if(!strcmp(first, "exit"))
        {
                if(argc>1 || redir)
                {
                        //char* exit1 = "happening in exit";
			//myPrint(exit1);
			oh_no();
                        return;
                }
                else
                        exit(0);
        }

        else if(!strcmp(first, "pwd"))
        {
                if(argc > 1|| redir)
                {
			oh_no();
                        //exit(0);
                       // char* pwd1 = "happening in pwd1";
			//myPrint(pwd1);
                        return;
                }
                char a[1024];
		getcwd(a, 1024);
                myPrint(a);
                myPrint("\n");
	}
        else if(!strcmp(first, "cd"))
        {    
		int x;   
                if(redir || argc == 0)
                {       
                        //char* cd1 = "happening in cd1";
			//myPrint(cd1); 
			oh_no();
			return;
                        //exit(0);
                }
                if(argc==1)
                        chdir(getenv("HOME"));
                if(argc==2)
		{
                        x = chdir(second);
                        if(x)
			{   
                       		// char* cd2 = "happening cd2";
				//myPrint(cd2); 
				oh_no(); 
				return;
				//exit(0); 
			}
		}
		if(argc > 2) 
		{
			//char* stillcd = "stillcd";
			//myPrint(stillcd);
			oh_no(); 
			return; 
			//exit(0);
		}
		
        }
        else
        {
		//FILE* file1;
		//FILE* fd1; 
                int forkret;
                int fd;
                if((forkret = fork()) == 0)
                {
                        switch(redir)
                        {
                                case 0:
                                        if (execvp(argv[0], argv) == -1)
                                        {
						//char* onecase0 = "1 case 0";
						//myPrint(onecase0);
                                                oh_no();
                                                exit(0);
                                        }
                                        break;
                                case 1:
                                        fd = open(file[0], O_WRONLY|O_TRUNC|O_EXCL|O_CREAT, S_IRUSR|S_IRGRP|S_IWGRP|S_IWUSR);
<<<<<<< .mine
                                        if(fd == -1 || argc > 1)
||||||| .r72
                                        if(fd == -1)
=======
                                        if(fd < 0)
>>>>>>> .r77
                                        {
						//char* onecase1 = "1 case 1";
						//myPrint(onecase1);
                                                oh_no();
                                                exit(0);
                                        }
					else
					{
                                        	dup2(fd, 1);
                                        	close(fd);
					}
                                         if (execvp(argv[0], argv) == -1)
					{
                                                oh_no();
						//char* case2 = "1 case 2";
						//myPrint(case2);
                                                exit(0);
                                        }
                                        break;
				case 2:
					fd = open(file[0], O_WRONLY|O_TRUNC|O_EXCL|O_CREAT, S_IRUSR|S_IRGRP|S_IWGRP|S_IWUSR);
					if(fd < 0)
					{	
						oh_no();
						exit(0);
                                        }
						close(fd);
						break;
                         	}              
                   	}
                        
                else
                        wait(NULL);
        }
}



int main(int argc, char *argv[])
{
	//char first[1024]; 
        char cmd_buff[1024];
        char *pinput;
<<<<<<< .mine
	char *file[2];  
||||||| .r72
	char **file = NULL; 
=======
	char *file[1024]; 
>>>>>>> .r77
	int job_count;
	int args;
	int i; 
	char* temp; 
	char *jobs[512]; 
	char *cmdargv[512]; 
	
        if (argc==1)
        {
                while (1) {
                        myPrint("myshell> ");
                        pinput = fgets(cmd_buff, 512, stdin);
                        if (!pinput)
                        exit(0);
		}
		temp = strdup(cmd_buff); 	
		job_count = parseline(temp, jobs);
		for(i=0; i<job_count; i++)
                {
			args = parse_cmd(jobs[i], cmdargv, file);
			eval(args, temp, cmdargv, file); 
                }
        }
	else if(argc==2)
	{
		FILE *batch = fopen(argv[1], "r");
		if(batch == NULL)
		{
			oh_no();
			exit(0);
		}
		while(fgets(cmd_buff,1024, batch) != NULL)
		{
			int length = strlen(cmd_buff);
			char* line = strdup(cmd_buff);
			char* test = strtok(line, " \n\t");
			if(test == NULL)
				continue;
			if(length > 512)
			{
				myPrint(cmd_buff); 
				oh_no();
				continue; 
			}
			job_count = parseline(cmd_buff, jobs);
			//printf("job count is %d\n", job_count);
			myPrint(cmd_buff);
			//myPrint(" \n"); 
			for(i=0; i<job_count; i++)
			{
				char* indiv_cmd = jobs[i];	
				//myPrint(indiv_cmd);
				//myPrint(" \n"); 
				args = parse_cmd(indiv_cmd, cmdargv, file);
                                eval(args, indiv_cmd, cmdargv, file);
			}
			memset(cmd_buff, 0, 513);
		}
		fclose(batch);
	}
	else
	{

		oh_no();
		exit(0);
	}		
       
}

