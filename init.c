#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>

void pipe_my (char *args[],int i,int count)
{
    pid_t fd[2];
    pipe(fd);
    char* args2[128];
    args[i+1]=NULL;
    int j;
    for (j=i+2;j<count;j++)
    {
	args2[j-i-2]=args[j];
    }
    args2[j-i-2]=NULL;
    if (fork()==0)
    {
        dup2(fd[1],1);
        close(fd[1]);
        close(fd[0]);
        execvp(args[i],args);
    }
    else {
        wait(NULL);
	/*if (args[i+3]!=NULL && strcmp(args[i+3],"|")==0)
	   pipe_my(args,i+2,count);
	else{
        dup2(fd[0],0);
        close(fd[0]);
        close(fd[1]);
        execvp(args[i+2],args);
	}*/
	int flag=0;
	dup2(fd[0],0);
        close(fd[0]);
        close(fd[1]);
	for (int k=0;k<j-i-2;k++)
	     if(strcmp(args2[k],"|")==0 && k!=0)
		{
			pipe_my(args2,k-1,j-i-2);
			flag=1;
                }
	if(!flag)
	{
        execvp(args2[0],args2);
	}
    }

}

int main() {
    /* 输入的命令行 */
    char cmd[256];
    /* 命令行拆解成的各部分，以空指针结尾 */
    char *args[128];
    while (1) {
        /* 提示符 */
        printf("# ");
        fflush(stdin);
        fgets(cmd, 256, stdin);
        /* 清理结尾的换行符 */
        int i;
        for (i = 0; cmd[i] != '\n'; i++)
            ;
        cmd[i] = '\0';
        /* 拆解命令行 */
        args[0] = cmd;
        for (i = 0; *args[i]; i++)
            for (args[i+1] = args[i] + 1; *args[i+1]; args[i+1]++)
                if (*args[i+1] == ' ') {
                    *args[i+1] = '\0';
                    args[i+1]++;
                    break;
                }
        args[i] = NULL;

        /* 没有输入命令 */
        if (!args[0])
            continue;

        /* 内建命令 */
        if (strcmp(args[0], "cd") == 0) {
            if (args[1])
                chdir(args[1]);
            continue;
        }
        if (strcmp(args[0], "pwd") == 0) {
            char wd[4096];
            puts(getcwd(wd, 4096));
            continue;
        }
	if(strcmp(args[0],"export")==0)
	{
	    int len=strlen(args[1]);
	    char path_name[128];
	    char path_value[128];
	    int k;
	    for (k=0;k<len;k++)
	    {
		if (*(args[1]+k)=='=')
		{   
		    *(args[1]+k)='\0';
		    break;
		}
	    }
	    strcpy(path_name,args[1]);
	    strcpy(path_value,args[1]+k+1);
	    setenv(path_name,path_value,1);
	}
        if (strcmp(args[0], "exit") == 0)
            return 0;
	/* 外部命令 */
	pid_t pid = fork();
        if (pid == 0)
	{
	    int flag=0;
            for (int j=0;j<i;j++)
     	    {
                if (strcmp(args[j],"|")==0)
                {
		    flag=1;
		    pipe_my(args,j-1,i);
                }
            }
       
        
            if(!flag){
            /* 子进程 */
                execvp(args[0], args);
            /* execvp失败 */
                return 255;  
            }
        }
        /* 父进程 */
        wait(NULL);
    }
}
