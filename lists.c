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

struct process_list{//gia process_list
    pid_node_ptr head;
};
struct process_list_node{
    pid_t pid;
    pid_node_ptr next;
};

struct list_node{ //gia aliases
    char *name;
    char **function;
    struct list_node *next;
};

struct list_head{
    node_ptr L;
};

//xeirismos listas pid's
pid_list pid_list_init(){
    pid_list h=malloc(sizeof(struct process_list));
    h->head=NULL;
    return h;
}
int pid_list_is_empty(pid_list pl){
    return (pl->head==NULL);
}
void pid_list_add(pid_list pl,pid_t pid ){
    pid_node_ptr n;
    n=malloc(sizeof( pid_node));
    n->next=NULL; //teleytaios kombos
    n->pid=pid;
    if(pl->head==NULL) pl->head=n; //adeia lista, to bazoume prwto kombo.
    else { //oxi adeia lista, to bazoume sto telos.
        pid_node_ptr nxt=pl->head;
        while(nxt->next!=NULL) nxt=nxt->next;  //ftanoume sto teleytaio kombo.
        nxt->next=n;
    }

    return ;
}
void pid_list_destroy(pid_list pl){
    pid_node_ptr n,p;
    if(pl->head!=NULL){ //an i lista exei kombous
        p=pl->head;
        while(p!=NULL){
            n=p->next; //deixnoyme sto epomeno kombo, an uparxei
            free(p); //free ton twrino
            p=n; //pame sto epomeno
        }
    }
    free(pl);
    return ;
}
void pid_list_remove(pid_list pl ,pid_t pid){  //kseroume oti osa zitisoume na diagraftoyn yparxoyn sti lista, an diagraftei kati, pame sto epomeno kombo tis listas
    pid_node_ptr p=pl->head;
    if(p!=NULL){//an yparxoun stoixeia.
        if(p->next==NULL){ //mono ena stoixeio sti lista, to diagrafw
            free(p);
            pl->head=NULL;
        }
        else{ //panw apo 1 stoixeia.
            pid_node_ptr prev=NULL,next;  //mporei na einai to prwto stoixeio pou theloume na bgaloume,ara to prev den exei timi
            while(p->pid!=pid){  //oso den eimaste ston swsto kombo
                prev=p;
                p=p->next;
                next=p->next;
            }
            //printf("p->pid:%d,pid:%d\n",p->pid,pid);
            if(prev==NULL){ //an den yparxei proigoymeno apo to p, simainei to p einai o prwtos kombos. prwta bazoume to epomeno na einai to 
                        //prwto stoixeio tis listas kai meta kanoyme free to p.
                next=p->next;
                pl->head=next;
            }
            else prev->next=next; //den einai to p to prwto, syndeoume to proigoumeno kai to epomeno tou p
            free(p);
        }
    
    
    }
    return ;
}
void check_for_zombies(pid_list pl){  //edw prin tin ektelesi kathe entolis, elegxoume an exei teleiwsei kapoia apo tis background wste na tis apeleytherwnoyme
    pid_node_ptr p=pl->head,n;
    pid_t * pid_array=NULL,*new_array;
    int i=0;
    while(p!=NULL){
        if(waitpid(p->pid,NULL,WNOHANG)>0) {//dhladh exei teleiwsei kapoia
            pid_array=realloc(pid_array,(i+1)*sizeof(pid_t)) ; //megalwnoume ton pinaka kata 1
            pid_array[i++]=p->pid;
           
        }
        p=p->next; 
    }
    for(int j=0;j<i;j++){//bgazoume ena ena ta pid pou termatisan.
        pid_list_remove(pl,pid_array[j]) ;
    } 
    if(pid_array!=NULL) free(pid_array); //free kathe fora.
    return ;
}

//xeirismos listas alias
                                                                                    //mode==0: remove, mode==1, append/update      
void alias_management(head h,char *name, char **command, int string_index,int MODE){ //mode: an prokeitai gia append/update i remove
    int s;                              //sto command pername olokliri tin protasi
    if(MODE==0){//destroyalias, kanw remove 
        list_remove(h,name);
    }
    else{  //MODE==1, update h append
        s=list_search(h,name);
                        //den uparxei idi gia na kanw update, kanw to alias append
        if(s==0) list_append(h,name,command,string_index);

            //yparxei idi to alias, to kanw update
        else list_update(h,name,command,string_index);     
    }                           
    return;
} 
void list_remove(head h,char * name){
    node_ptr p=h->L,next,prev;
    int n,s,i;
    n=no_of_entries(h);
    if(list_search(h,name)!=0){ //de sbinoume to stoixeio an dwthike lathos onoma.
        if(n==1){ //mono ena stoixeio i lista
            free(p->name);
            for(i=0;i<MAXSIZE;i++) if(p->function[i]!=NULL) free(p->function[i]);
            free(p->function);
            free(p);
            h->L=NULL;//adeia i lista.
        }
        else { //panw apo 1
            if(list_search(h,name)>1){//dhladh den einai to prwto stoixeio.
                prev=p;
                while(strcmp(p->name,name)!=0){
                    prev=p;
                    p=p->next;
                }
                next=p->next;
                free(p->name);
                for(i=0;i<MAXSIZE;i++) if(p->function[i]!=NULL) free(p->function[i]);
                free(p->function);
                free(p);
                prev->next=next;
            }
            else{ //einai to prwto
                next=p->next;
                free(p->name);
                for(i=0;i<MAXSIZE;i++) if(p->function[i]!=NULL) free(p->function[i]);
                free(p->function);
                free(p);
                h->L=next;
            }

        }
    }
}            

head list_init(){
    head h=malloc(sizeof(struct list_head));
    h->L=NULL;
    return h;
}
void list_destroy(head h){
    node_ptr n=h->L,p;
    int i,j;
    if(n!=NULL){//yparxoun komboi na sbisw
        while(n!=NULL){
            p=n->next;//deixnoume ston epomeno(an yparxei) gia free
            free(n->name);
            for(i=0;i<MAXSIZE;i++){
                if(n->function[i]!=NULL) free(n->function[i]);
            }
            free(n->function);
            free(n);
            n=p;
        }
    }
    free(h);
}

void list_append(head h, char* name,char** command,int string_index){
    node_ptr new,p;
    p=h->L;
    int i,count;
    char *token,*dup;
    new=(node_ptr) malloc(sizeof(node ));
    if(p==NULL) h->L=new; //prwto stoixeio tis listas
    else{
        while(p->next!=NULL){
            p=p->next; //ftanoume sto telos tis listas
        }  
        p->next=new;
    }
    new->name=malloc(MAXSIZE*sizeof(char));
    new->name=strcpy(new->name,name); //mpike to onoma
    new->next=NULL; //tha mpei teleytaio
    new->function=(char **) malloc(MAXSIZE*sizeof(char *));  //maxsize*maxsize diastasi, tha kanoume free oses den xrisimopoihsoume
    for(i=0;i<MAXSIZE;i++) new->function[i]=(char *) malloc(MAXSIZE*sizeof(char));
    dup=(char *) malloc(MAXSIZE*sizeof(char)); //kratame antigrafo tis leksis.
    strcpy(dup,command[string_index-1]);
    token=strtok(dup,"\""); //pairnoume to eswteriko twn quotes,mia leksi.
    char *dup2=malloc(MAXSIZE*sizeof(char));
    strcpy(dup2,token);
    token=strtok(dup2," ");  //xwrizoume ana keno
    i=0;
    while(token!=NULL){
        strcpy(new->function[i++],token);
        token=strtok(NULL," ");
    }
    for(i;i<MAXSIZE;i++) {free(new->function[i]); new->function[i]=NULL;} //kanoume free oses theseis den xrisimopoihsame.
    free(dup);
    free(dup2);
    return;
}


int list_search(head h, char* name){
    node_ptr p=h->L;
    int i=0;
    while(p!=NULL){        
        i++;
        if(strcmp(p->name,name)==0){ 
            return i; //se poia thesi to brikame
        } 
        p=p->next;
    }
    return 0; //den to brikame, psaksame olous tous komboys
}

void get_alias(head h , char * name, char **command,int *string_index,int **indexes_array,char ****comm,int *sizeofarray,int position){
    int i,j,s,counter,pos,c;
    node_ptr p=h->L;
    char *dup,*dup2,*token;
    if(list_search(h,name)==0) return ;
    s=list_search(h,name);
    if(s>0){ //brethike to alias.
        counter=0; //metrame poses lekseis einai to command tou alias.
        for(i=1;i<s;i++) p=p->next; //briskoume ton kombo ston opoio brisketai to alias.
        for(i=0;i<MAXSIZE;i++){
            if(p->function[i]==NULL) break;
            else counter++;
        }   
        dup=(char *)malloc(MAXSIZE*sizeof(char));
        dup[0]=0;
        for(i=0;i<counter-1;i++){ 
            strcat(dup,p->function[i]);
            if(strcmp(p->function[i],";")!=0) strcat(dup," ");  

        }
        strcat(dup,p->function[i]);//de bazoume keno stin teleytaia leksi.
        dup2=(char *)malloc(MAXSIZE*sizeof(char));
        strcpy(dup2,dup);
        c=0;
        token=strtok(dup,";");
        while(token!=NULL){c++; token=strtok(NULL,";"); }
        if(c==1){//mia entoli sto alias, apla tin kanw copy sti thesi tou command me to opoio perase
            counter=0; //metrame poses lekseis mesa stin entoli
            strcpy(dup,dup2);
            token=strtok(dup,";"); //bgazoume to ; apo to telos.
            strcpy(dup2,dup);
            token=strtok(dup2," ");
            while(token!=NULL){counter++; token=strtok(NULL," "); }
            char **temp;
            int temp_ind=*string_index;
            temp=(char **)malloc(*string_index*sizeof(char *));
            for(i=0;i<*string_index;i++) temp[i]=(char *) malloc(MAXSIZE*sizeof(char));
            for(i=1;i<*string_index;i++) strcpy(temp[i-1],command[i]); //copy tis lekseis,ektos apo tin prwti leksi pou einai to alias
            for(i=0;i<counter;i++) strcpy(command[i],p->function[i]);
            *string_index+=counter-1;  //-1 giati to antikatastisame mia leksi apo tin arxiki entoli. px an alias ll=ls -l kai kalesame ll, einai 1+2 -1=2
            for(i=counter;i<*string_index;i++) strcpy(command[i],temp[i-counter]); //antigrafoume oles tis epomenes apo to alias
            for(i=0;i<temp_ind;i++) free(temp[i]);
            free(temp);
        }
        else{ 
            char ***comm_dup;
            int *index_array_dup,new_size=*sizeofarray+c-1;  //c-1 efoson mia entoli tha mpei se xwro pou idi yparxei, tin entoli stin opoia yparxei to alias
            comm_dup=(char ***) malloc(new_size*sizeof(char **));  //desmeyoyme pinakes gia na kanoume antigrafi twn prin kai meta apo to alias kai an ta ksanaperasoume ston neo pinaka command
            index_array_dup=(int *) malloc(new_size*sizeof(int));
            for(i=0;i<new_size;i++) {
                comm_dup[i]=(char **) malloc(MAXSIZE*sizeof(char *));
                for(j=0;j<MAXSIZE;j++) comm_dup[i][j]=(char *) malloc(MAXSIZE*sizeof(char));
            }
            for(i=0;i<position;i++){//antigrafoume oles tis palies entoles
                for(j=0;j<(*indexes_array)[i];j++){
                    strcpy(comm_dup[i][j],(*comm)[i][j]); 
                }
                index_array_dup[i]=(*indexes_array)[i];
            }
            int next=position+c; //pou ksekinaei i epomeni.
            int old_pos=position+1; //krataw sysxetisi me ton palio pinaka
            for(i=next;i<new_size;i++){
                for(j=0;j<(*indexes_array)[old_pos];j++){
                    strcpy(comm_dup[i][j],(*comm)[old_pos][j]);
                }
                index_array_dup[i]=(*indexes_array)[old_pos];
                old_pos++;
            } 
            //pame na kanoume tis allages sta twrina, ksanabazw stin dup olokliri tin entoli
            strcpy(dup,""); //adeiazoume to string.
            for(i=0;i<counter-1;i++){ 
                strcat(dup,p->function[i]);
                if(strcmp(p->function[i],";")!=0) strcat(dup," ");  
            }
            strcat(dup,p->function[i]);//de bazoume keno stin teleytaia leksi.
            char **c_dup;
            c_dup=(char **) malloc(c*sizeof(char **)); //pinaka toswn stoixeiwn oswn protasewn
            for(i=0;i<c;i++) c_dup[i]=(char *) malloc(MAXSIZE*sizeof(char));
            i=0;           
            token=strtok(dup,";"); //spame ana entoli
            while(token!=NULL){
                strcpy(c_dup[i++],token);
                token=strtok(NULL,";");
            }
            //twra kathe mia protasi pou tin spaw ana leksi tin bazw stin katallili thesi
            pos=position;
            counter=i; //poses protaseis exoume
            for(i=0;i<counter;i++){
                token=strtok(c_dup[i]," "); //ana keno
                j=0;
                while(token!=NULL){
                    strcpy(comm_dup[pos][j++],token);
                    token=strtok(NULL," ");
                }
                index_array_dup[pos]=j;
                pos++;
            }
            for(i=0;i<c;i++) free(c_dup[i]); //daigrafoume xwro pou desmeysame gia na ta kanoume copy
            free(c_dup);
            destroy_array(*comm,*sizeofarray);
            free(*indexes_array);
            *sizeofarray+=c-1;  //enimerwnoume ton pinaka me tis entoles
            (*comm)=comm_dup;
            (*indexes_array)=index_array_dup; 
        }
    }
    free(dup);
    free(dup2);
    return ;  
}

void process_print(pid_list pl){
    pid_node_ptr p=pl->head;
    printf("-----\n");
    while(p!=NULL){
        printf("pid:%d\n",p->pid);
        p=p->next;
    }
    printf("----\n\n");
    return ;
}

void list_print(head h){
    node_ptr p=h->L;
    printf("aliases:\n");
    while(p!=NULL){
        printf("name:%s,function:",p->name);
        for(int i=0;i<MAXSIZE;i++) if(p->function[i]!=NULL) printf("%s ",p->function[i]);
        printf("\n");
        p=p->next;
    }
    return;
}

void list_update(head h, char * name, char **command, int string_index){
    int i,j,s,count;
    char *token;
    node_ptr p=h->L;
    s=list_search(h,name); //s=1, dhladh 1 kombos yparxei.
    for(i=1;i<s;i++){  //gia na pame ston kombo me arithmo 1,exoume idi paei ston kombo #1
        p=p->next;
    }
    //printf("eimai ston kombo me onoma:%s\n",p->name);
    for(i=0;i<MAXSIZE;i++) if(p->function[i]!=NULL) free(p->function[i]);
    for(i=0;i<MAXSIZE;i++) p->function[i]=(char *) malloc(MAXSIZE* sizeof(char ));
    char *dup;
    token=strtok(command[string_index-1],"\""); //pairnoume to eswteriko twn quotes,mia leksi.
    dup=(char *) malloc(MAXSIZE*sizeof(char));
    strcpy(dup,token);
    token=strtok(dup," ");  //xwrizoume ana keno
    i=0;
    while(token!=NULL){
        strcpy(p->function[i++],token);
        token=strtok(NULL," ");
    }
    for(i;i<MAXSIZE;i++) {free(p->function[i]); p->function[i]=NULL;} //kanoume free oses theseis den xrisimopoihsame.
    free(dup);
    return;

}
int no_of_entries(head h){
    node_ptr p=h->L;
    int n=0;
    if(p==NULL) return 0;
    else{
        while(p!=NULL){
            n++;
            p=p->next;
        }
    }
    return n;
}