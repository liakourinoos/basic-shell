#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h> // to have access to flags def
#include <glob.h>
#include <signal.h>
#include "defines.h"
#include "lists.h"
#include "history.h"
#include "array_handling.h"
#include "basic_shell.h"
int execute(char ****command,int i,int *sizeofarray,int **indexes_array,char ***history,head h){  //pointer ston pinaka se periptwsi pou kanw realloc, poia entoli ektelw,megethos pinaka, megethos kathe entolis, domi gia istoriko kai domi listas gia aliases
                                //PREPEI H EXECVP NA PAREI 2 FORES TO ONOMA TOU PROGRAMMATOS STIN ARXI KAI NULL teleytaio orisma
                                    //execvp(argv[0],argv), opoy argv[n]=NULL.  
    pid_t pid;  

    int in_fd,out_fd,pos,cd_res, retval=0; //ti tha epistrafei                     
    if((strcmp((*command)[i][0],"myHistory")==0) && (*indexes_array[i])==2 ) { //dhladh dwthike kati typoy: "history ...",kanw antikatastasi kai meta paw gia exec
        history_get(history,atoi((*command)[i][1]),(*command)[i],&(*indexes_array)[i]);   //an epistrafei allagmeno to command, to ekteloume, an den epistrafike, apla fainetai oti pigame na kanoume ektelesi mi apodektis timis istorikou.
    }   //dhladh de theloume na to kanoume redirection kapou
    else if((strcmp((*command)[i][0],"myHistory")==0) && (*indexes_array[i])>2 && search_in_array((*command)[i],(*indexes_array)[i],"|")!=0  && search_in_array((*command)[i],(*indexes_array)[i],">")!=0  
            && search_in_array((*command)[i],(*indexes_array)[i],"<")!=0  && search_in_array((*command)[i],(*indexes_array)[i],">>")!=0 ) write(2,"Usage: myHistory <no>\n",strlen("Usage: myHistory <no>\n")) ;
    
    char **copy_of_command;  //edw antigrafoume tin entoli pou mas dwthike. meta tin history get omws se periptwsi pou to myHistory <arithmos> einai valid entoli kai allaksei stin isodynami tis, kathws sto istoriko den apothikeyoyme ta "myHistory ..."
    int copy_ind=(*indexes_array)[i];
    copy_of_command=(char **) malloc((*indexes_array)[i]*sizeof(char *)) ; //kanoume copy tin entoli gia na mpei sto history sto telos.
    
    for(int j=0;j<(*indexes_array)[i];j++){
        copy_of_command[j]=(char *) malloc(MAXSIZE*sizeof(char));
        strcpy(copy_of_command[j],(*command)[i][j]);
    }

    //elegxos an prokeitai gia alias. an einai, kanw antikatastasi to alias me to command kai paw gia exec h kapoia apo tis entoles tis diergasias gonea
    get_alias(h,(*command)[i][0],(*command)[i],&((*indexes_array)[i]),indexes_array,command,sizeofarray,i); 
    if( strcmp((*command)[i][0],"createalias")!=0 ) check_for_wildchars((*command)[i],&((*indexes_array)[i]));  //den prepei na elegxw gia wildchars otan ftiaxnw aliases kai prepei i entoli na mpainei opws einai sto history, oxi expanded
    if( strcmp((*command)[i][0],"quit")==0) retval= -1; 
     //gia na min paei kai kanei fork.
    else if(strcmp((*command)[i][0],"cd")==0 ){    
        char *home=getenv("HOME"); 
        if((*indexes_array)[i]==1) cd_res=chdir(home);     //dhladi i entoli einai sketo cd, se ayti tin periptwsi pame sto home.
        else if((*indexes_array)[i]==2) cd_res=chdir((*command)[i][1]);
        else if((*indexes_array)[i]>2){ write(2,"cd:too many arguments.\n",strlen("cd:too many arguments.\n")); cd_res=0;} //dinoume timi sto cd_res edw gia na einai orismeno se kathe periptwsi. edw de tha ektelestei kan to cd,ara den to metraw sa lathos
        if(cd_res==-1)  perror("cd.");  //de doulepse, alla den teleiwse to programma to cd gia kapoio logo,alla den teleiwnei to programma.
    }                                                                                   
    else if((strcmp((*command)[i][0],"myHistory")==0 && (*indexes_array)[i]==1)){  //to bazoume sto istoriko kai epeita to ektypwnoume kai epistrefoume, gia na min exoume themata me history update pio katw.
        history_update(&history,copy_of_command,(*indexes_array)[i]);  //eidiki periptwsi an exw dwsei san alias to myHistory
        print_history(history); 
        for(int j=0;j<(*indexes_array)[i];j++) free(copy_of_command[j]);
        free(copy_of_command); //kanoyme kai free ton pinaka gia n amin exoume leaks.
        return 0; //gyrizoyme pali sti shell gia na na paroume tin epomeni entoli/ektelesoume tin epomeni entoli 
    }
    else if(strcmp((*command)[i][0],"createalias")==0 ){ //to bazoume sti lista
        if( (*indexes_array)[i]==3) alias_management(h,(*command)[i][1],(*command)[i],(*indexes_array)[i],1);
        else write(2,"Usage: createalias <name_of_alias> \"<command>\"\n",strlen("Usage: createalias <name_of_alias> \"<command>\"\n"));
    }
    else if(strcmp((*command)[i][0],"destroyalias")==0){ //to bgazoume apo ti lista
        if( (*indexes_array)[i]==2) alias_management(h,(*command)[i][1],(*command)[i],(*indexes_array)[i],0);
        else write(2,"Usage: destroyalias <name_of_alias> \n",strlen("Usage: destroyalias <name_of_alias> \n"));
    }
    else if((strcmp((*command)[i][0],"aliases")==0) && (*indexes_array[i])==1){
        list_print(h);                
    }    
    else{  //oxi kapoia apo tis entoles pou xeirizetai i diergasia-goneas, pame gia fork kai exec.
        int bg=0;
        if(search_in_array((*command)[i],(*indexes_array)[i],"&")==1){
            bg=1; //tha prepei na min perimenei o pateras na teleiwsei to paidi tin ektelesi, synexizei me tin epomeni entoli.
            strcpy((*command)[i][(*indexes_array)[i]-1],""); //bgazoume apo tin protasi to &
            (*indexes_array)[i]--; //-1 poses lekseis exoume
        }
        pid=fork();
        if(pid==-1) perror("Cannot create new process.");
        else if(pid==0){//paidi
            signal(SIGTSTP,SIG_DFL); //default simperifora.
            signal(SIGINT,SIG_DFL); 
            if(bg==1) setpgid(getpid(),getpid()); //ftiaxoume process group,efoson einai bg entoli.
             //gia to pg tou shell mas  }
            for(int j=0;j<copy_ind;j++) free(copy_of_command[j]); //kanw free ton pinaka ayto, afoy apla klwnopoihthike me tin fork, xreiazetai mono ston patera.
            free(copy_of_command); 
            if(search_in_array((*command)[i],(*indexes_array)[i],"|")==1){  //an gyrisei 1, den kanw kati peraiterw apo to pipelining, epistrefw sti shell gia tin epomeni entoli
                int pipe_fd[2];//pipe poy tha xrisimopoihthei gia epikoinwnia
                int *length; //pinakas pou tha kratame poses lekseis exei kathe protasi.
                int arraysize; //poses entoles tha exei mesa o pinakas pou tha ftiaksoume
                char ***temp=seperate_commands((*command)[i],(*indexes_array)[i],&arraysize,&length); //epistrefei ton pinaka temp, pou einai oi entoles tou pipeline xwrismenes
                //tha baloume NULL ston pinaka afou ftiaksoume to redirection.
                int input=0,output=1,fd[2],redir_res;  //input, bazoyme default to stdin ektos an broume redirection, opws kai output to arxikopoioume me stdout.
                pid_t pid2;
                int *fd_array; //gia na min exoume leaks, kratame ola ta fd's twn pipes
                int exec_res;
                fd_array=(int *) malloc(arraysize*sizeof(int)); //oses entoles, tosa pipes tha ftiaksw
                pid_list pid_2_l; //kratame ola ta pid's gia na ta kanoume meta wait. theloume na treksoun mazi wste na min gemisei pote pipe kai meinoume se read block.
                pid_2_l=pid_list_init();
                for(int j=0;j<arraysize;j++){
                    exec_res=0;
                    pipe(fd);
                    fd_array[j]=fd[READ]; //to kratame
                    pid2=fork();
                    if(pid2==0){//paidi-paidi
                        dup2(input,0);//kanoume to input to stdin.
                        if(j==arraysize-1) dup2(output,1); //an einai i teleytaia entoli, kanoume to stdout eksodo
                        else dup2(fd[WRITE],1); //an oxi, kanoume eksodo to write end tou pipe.
                        close(fd[WRITE]);
                        close(fd[READ]);
                        open_redirections(temp[j],&(length[j]));  //oi open_redirections anoigei inputs/outputs opws tis briskei                                                                  
                        free(temp[j][length[j]]); //kanoume NULL teleytaia thesi gia na mpei sto exevp swsta o pinakas
                        temp[j][length[j]]=NULL;  
                        if(strcmp(temp[j][0],"aliases")==0) list_print(h);  //mono ayta ta duo commands exw balei na mporoun na ektelountai se pipes.
                        else if(strcmp(temp[j][0],"myHistory")==0) print_history(history);
                        else exec_res=execvp(temp[j][0],temp[j]);
                        if(exec_res==-1) write(2,"failed to execute given command.\n",strlen("failed to execute given command.\n")); //an den petyxei i exec, ektypwnoume lathos kai apodesmeyoume o,ti kaname malloc.
                        destroy_array(temp,arraysize);
                        free(fd_array); 
                        free(length); 
                        pid_list_destroy(pid_2_l); //prin ti fork, ara klironomeitai kai sto paidi, den to xreiazomaste edw.
                        return 2; //an den petyxei, return 2 gia na gyrisei sti shell kai na kanei kai ekei free osa exoun ginei cloned apo tin arxiki fork.
                    } //paidi-pateras diergasia
                    close(fd[WRITE]); //gia na min blockarei to read end.
                    pid_list_add(pid_2_l,pid2); //to kratame sti lista. tha tis treksoume taytoxrona, wste otan grafei kati i mia, na pernaei stin epomeni kai na min riskaroume na gemisei to pipe kai meinoume se write block
                    process_print(pid_2_l);
                    input=fd[READ]; //kratame to read end gia na ginei eisodos stin epomeni epanalipsi.
                } 
                //paidi-pateras diergasia
                while(pid_list_is_empty(pid_2_l)==0) check_for_zombies(pid_2_l); //oso trexoun akoma diergasies, elegxe an teleiwse kamia na tin kaneis wait, mi meinei zombie.
                pid_list_destroy(pid_2_l);
                close(fd[READ]);
                free(fd_array);
                destroy_array(temp,arraysize);
                free(length); //kanoume free tous pinakes pou kaname allocate.
                return 2; //den einai na ektelesw kati parakatw, epistrefw kai edw pisw sti shell gia na kanw free ta ypoloipa.
            }  
            open_redirections((*command)[i],&(*indexes_array)[i]); //anoigoume opoia arxeia prepei kai ftiaxnoume tin protasi na exei mono orismata.
            if(strcmp((*command)[i][0],"pwd")==0){ 
                char cwd[1024];
                if(getcwd(cwd,1024) != NULL){ 
                    if(execlp("echo","echo",cwd,NULL)==-1){
                        perror("Cannot run command");  
                        retval=2; 
                    }
                }
            }
            free((*command)[i][(*indexes_array)[i]]); //kanoume NULL teleytaia thesi gia na mpei sto exevp swsta o pinakas
            (*command)[i][(*indexes_array)[i]]=NULL;                
                                                    //den trexei tin entoli history h thn aliases i exec.    
             if(strcmp((*command)[i][0],"myHistory")==0) print_history(history);
             else if(strcmp((*command)[i][0],"aliases")==0) list_print(h); 
             else if((execvp((*command)[i][0],(*command)[i]))==-1) perror("Cannot run command");              
            //se periptwsi pou den treksei h exec, apeleutherwnoume ti mnimi pou katalambanei to paidi.
            return 2; //gia na kserw oti to paidi tha paei pali stin shell etsi wste na kanw ta panta free ekei.
        }
        else{//pateras
           if (bg==0){
                waitpid(pid,NULL,WUNTRACED);//perimenoume na teleiwsei i diergasia pou den einai sto bg i synexizoyme an labei to paidi sigtstp/sigint                               
            } 
            else{
                setpgid(pid,pid);//dilwnoume kai ston parent to pg 
                pid_list_add(pl,pid) ;//to bazoume se domi gia na tin kanoume wait meta, mi meinei zombie mexri to telos.
            } 
        } 
    }//epistrefoume sti diergasia-patera
    history_update(&history,copy_of_command,copy_ind); //kratame sto history tin arxiki entoli pou diabasame apo ton xristi, oxi ayti meta apo oles tis tyxon enimerwseis/allages
    for(int j=0;j<copy_ind;j++) free(copy_of_command[j]);
    free(copy_of_command); //kanoume free ton xwro pou piasame.       
    return retval; 
}
void shell(){
    char ***history=history_init(); 
    char ***command=NULL; //mia entoli, spasmeni se polles theseis
    char a[MAXSIZE][MAXSIZE];  //se kathe ena a[i][] bazw mia entoli kai meta tis spaw meta se epimerous gia na mpei sto command
    char input[MAXSIZE];
    int cind,nl,i,ret=0,k;
    char *token;
    int* indexes_array=NULL;
    signal(SIGINT,SIG_IGN);
    signal(SIGTSTP,SIG_IGN);
    pl=pid_list_init(); //tin arxikopoioume
    head h=list_init();
    do {
        cind=-1; //to bazoume -1 etsi wste na doume an uparxei ontws input kai na ksekinisei apo 0
        printf("in-mysh-now:>");
        fgets(input, MAXSIZE, stdin); //h fgets exei san teleytaio char to \n,ara sti thesi tou tha baloume to \0
        input_parse(input,&command,&cind,&indexes_array);
        if(cind!=-1){
            //edw pairnw kathe entoli kai tin ektelw
            for(i=0;i<cind;i++){       
                check_for_zombies(pl); //elegxoume an teleiwse kapoia background entoli.    
                ret=execute(&command,i,&cind,&indexes_array,history,h);   //pernaw pointer stous pinakes gia tin periptwsi pou to alias einai panw apo mia entoles kai xreiastei na allaksw megethos pinaka
                char buf[1024]; 
                int ret_len=sprintf(buf, "%d", ret);  //gia na ektypwsoume akeraio me write, to brika online.
              //  write(2,"retval:",strlen("retval:"));
              //  write(2,buf,ret_len);
               // write(2,"\n",1);
                if(ret==-1) break; //feygoume apo to for loop, pame sto while, opou kai ayto tha stamatisei
                else if(ret==2){//einai to paidi pou gyrise, kanw ta panta free gia na min exw leaks kai exit epeita
                    destroy_array(command,cind);
                    free(indexes_array);
                    destroy(history); 
                    list_destroy(h);
                    pid_list_destroy(pl);
                    printf("meta to return, prin to exit.\n");
                    exit(2); 
                }
            } 
            //edw pame otan gyrisei o pateras
            destroy_array(command,cind); //kathe fora to kanoume free gia na mi kratame askopa alloc'd xwro
            free(indexes_array); //to idio kai me ton pinaka gia to kathe string.
        }
    } while(ret!=-1);
    destroy(history); //telos tou shell, kanoume free osa exoun apomeinei pou den eginan free entos tou do...while loop.
    list_destroy(h);
    pid_list_destroy(pl);
}
