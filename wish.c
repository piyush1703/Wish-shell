#include<stdio.h>
#include<ctype.h>
#include<string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/wait.h>
#include <time.h>

char *path[1024]={"/bin",NULL};
char* arr[1024];
FILE*out=NULL;
char * command;

void print_error(){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message)); 
}

char* strsep(char** stringp, const char* delim)
{
  char* start = *stringp;
  char* p;

  p = (start != NULL) ? strpbrk(start, delim) : NULL;

  if (p == NULL)
  {
    *stringp = NULL;
  }
  else
  {
    *p = '\0';
    *stringp = p + 1;
  }

  return start;
}

int search_path(char*temp,char *arg){
    int j=0;
    while(path[j]!=NULL){
        snprintf(temp, 1024, "%s/%s", path[j], arg);
        int check=access(temp,X_OK);
        if(check==0) return 0;
        j++;
    }
    return -1;
}

char *trim(char *s) {
  // trim leading spaces
  while (isspace(*s))
    s++;

  if (*s == '\0')
    return s; // empty string

  // trim trailing spaces
  char *end = s + strlen(s) - 1;
  while (end > s && isspace(*end))
    end--;

  end[1] = '\0';
  return s;
}

void redirect(FILE *out) {
  int outFileno;
  if ((outFileno = fileno(out)) == -1) {
    print_error(); 
    return;
  }

  if (outFileno != STDOUT_FILENO) {
    // redirect output
    if (dup2(outFileno, STDOUT_FILENO) == -1) {
      print_error();       
      return;
    }
    if (dup2(outFileno, STDERR_FILENO) == -1) {
      print_error();     
      return;
    }
    fclose(out);
  }
}

void *execute(){
    char* line=(char*)malloc(1024 * sizeof(char));
    strcpy(line,command);
    char* ptr;
    int id=0;
    while((ptr=strsep(&line," "))!=NULL){
        char *str=trim(ptr);
        char * rs=strdup(str);
        char *tpr;
        if(strcmp(str,">")==0){
            arr[id]=strdup(str);
            id++;
            continue;
        }
        int fg=0;
        while((tpr=strsep(&str,">"))!=NULL){
            arr[id]=strdup(trim(tpr));
            id++;
            char ap[]=">";
            char *pr=strstr(rs,ap);
            if(pr && fg%2==0){
                arr[id]=strdup(ap);
                id++,fg++;
            }
        }
    }

    int fg=0;
    for(int j=0;j<id;j++){
        if(strcmp(arr[j],">")==0){
            fg++;
            if(j+1==id-1){
                out=fopen(arr[j+1],"w");
            }
            else out=NULL;
        }
    }

    if((fg==1 && out==NULL) || fg>1){
        out=NULL;
        print_error();
        return NULL;
    }

    if(strcmp(arr[0],"exit")==0){
        if(id==1 || (id==2 && strcmp(arr[1],"\0")==0)){ 
            exit(0);
        }
        else{
            print_error(); 
            return NULL;
        }
    }

    else if(strcmp(arr[0],"cd")==0){
        if(id==1 || id>2){
            print_error(); 
            return NULL;
        }
        else{
            int ck=chdir(arr[1]);
            if(ck!=0){
                print_error();
                return NULL;
            }
        }
    }

    else if(strcmp(arr[0],"path")==0){
        path[0]=NULL;
        for(int i=1;i<id;i++){
            path[i-1]=strdup(arr[i]);
        }
        path[id-1]=NULL;
    }

    else{
        char temp[1024];
        if(search_path(temp,arr[0])==0){
            pid_t pid = fork();
            if(pid==-1){
                print_error(); 
            }
            else if(pid==0){
                char *args[id+1];
                args[0]=strdup(temp);
                int idx=1;
                for(int i=1;i<id;i++){
                    args[idx]=strdup(arr[i]);
                    idx++;
                }
                args[idx]=NULL;
                
                if(out!=NULL) redirect(out);

                if(execv(args[0],args)==-1){
                    print_error(); 
                }
                
            }
            else{
                waitpid(pid,NULL,0);
            }
        }
        else{
            print_error(); 
        }
    }
    return NULL;
}

int main(int argc, char*argv[]){

    
    if(argc==1){// Interactive Mode

        printf("wish> ");
        char *line = NULL;
        size_t ln = 0;
        size_t nread;
        while((nread = getline(&line, &ln, stdin)) != -1){
            out=NULL;
            while((command=strsep(&line,"&"))!=NULL){
                char *buf=trim(command);
                command=strdup(buf);
                if(strcmp(command,"\n")==0 || strcmp(command,"\t")==0 || strcmp(command,"")==0 ) continue;
                pthread_t thid;
                if(pthread_create(&thid, NULL, &execute,NULL) != 0){
                    print_error();
                }
                if (pthread_join(thid, NULL) != 0){
                    print_error();
                }
            }
            printf("wish> ");
        }
        exit(0);
    }

    else if(argc==2){// Batch Mode

        FILE *fp = fopen(argv[1], "r");
        if (fp == NULL) {
            print_error();
        }
        if (NULL != fp) {
            fseek (fp, 0, SEEK_END);
            long size = ftell(fp);

            if (size==0) {
                print_error();
            }
        }

        char *line = NULL;
        size_t ln = 0;
        size_t nread;
        while((nread = getline(&line, &ln, fp)) != -1){
            out=NULL;
            while((command=strsep(&line,"&"))!=NULL){
                char *buf=trim(command);
                command=strdup(buf);
                pthread_t thid;
                if(pthread_create(&thid, NULL, &execute,NULL) != 0){
                    print_error();
                }
                if (pthread_join(thid, NULL) != 0){
                    print_error();
                }
            }
        }
        exit(0);
    }

    else{
        print_error();
        exit(1);
    }
    
}