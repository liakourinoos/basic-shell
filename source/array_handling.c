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
int word_has_wildchar(char *word){
    int i,j;
    int count=0,*arr=NULL; //arxika kenos,dhladh den brikame wildchar
    for(i=0;i<strlen(word);i++){
        if(word[i]=='*' || word[i]=='?') return 1; //brethike wildchar, tha paei stin glob
    }
    return 0; //de brethike,synexzoume
}
void check_for_wildchars(char **command,int *string_index){
    int i,j,k,g_result,new_string_index,no_of_matches;
    glob_t names;
    for(i=0;i<*string_index;i++){ //gia kathe leksi tis protasis
        if(word_has_wildchar((command)[i])==1){//brethike
            g_result=glob(command[i],0,NULL,&names);
            if(g_result==3) write(2,"no matches.\n",strlen("no matches.\n"));
            else if(g_result==0){//bethikan matches,prpei na ta balw stin protasi mou
                no_of_matches=names.gl_pathc; //posa matches brikame
                char **temp;
                int remaining=*string_index-(i+1);
                if(remaining>0){ //exei noima na kratisoume tis upoloipes, an ontws uparxoun
                    temp=(char **) malloc(remaining*sizeof(char *)); //kratame sto temp tis upoloipes lekseis meta apo auti me to wildchar
                    for(j=0;j<remaining;j++) temp[j]=(char *) malloc(MAXSIZE*sizeof(char));
                    k=0;
                    for(j=i+1;j<*string_index;j++){ strcpy(temp[k++],command[j]);
                        //antigrafoume tis ypoloipes lekseis.
                        // printf("kanw copy ti leksi %s stin temp.\n",temp[k-1]);
                        } 
                }
                for(j=i,k=0;j<MAXSIZE && k<no_of_matches;){ //oso exoume xwro stin protasi kai exoume lekseis na baloume
                    strcpy(command[j++],names.gl_pathv[k++]);
                }
                *string_index+=no_of_matches-1; //enimerenoyme to poses lekseis exoyme, no_of_matches -1 giati i leksi me to wildchar antikatastathike
                globfree(&names); //free ton allocated xwro kathe fora.
                i=i+no_of_matches; //gia na pame stin epomeni leksi tis arxikis protasis,
                k=0;
                for(j=i;j<MAXSIZE && k<remaining;){
                    strcpy(command[j++],temp[k++]); //bazoume pisw stin arxiki protasi tis ypoloipes lekseis.
                }
                // for(j=0;j<*string_index;j++) printf("|%s| ",command[j]);
                if(remaining>0) {for(k=0;k<remaining;k++) free(temp[k]); free(temp) ;} //free ton xwro pou desmeysame
                --i; 
            }
        }
    }
    return;
}


void destroy_array(char ***command,int cind){
    int i,j;
    for(i=0;i<cind;i++){
        for(j=0;j<MAXSIZE;j++){
             free(command[i][j]);
            
        }
        free(command[i]);
    }
    free(command);
    return;
}

                                                      

int check_correct_input(char *input){  //an exoume dwsei mono mia akolouthia apo kena kai ';'den kanoume parse, pame stin epomeni epanalipsi
    int i,q_m,only_question_marks_and_spaces=1,max=strlen(input),start;
    for(i=0;i<max;i++) if(input[i]!= ' ' && input[i]!= ';') only_question_marks_and_spaces=0; //simainei oti exoume kai allous xaraktires, ftiaxoume to input
    if(only_question_marks_and_spaces==1) return 0; //lathos morfi, den kanoume tipota
    start=0;
    for(i=0;i<max;i++){
        if(input[i]!= ' ' && input[i]!= ';'){
            input[start++]=input[i];
            q_m=1;
        }
        else if(input[i]== ' ' || (input[i]== ';' && q_m==1 )){
            input[start++]=input[i];
            if(input[i]==';') q_m=0; //kleinei mexri na broume xaraktira plin ' ' kai ';'

        }
    }    
    if(start<MAXSIZE-1) input[start]=0; //neo akro.
    return 1;
}

void input_parse(char *input,char ****command,int *cind,int **indexes_array){ //tou pername 0, i arithmisi ksekinaei apo 1
    char a[MAXSIZE][MAXSIZE];  //se kathe ena a[i][] bazw mia entoli kai meta tis spaw meta se epimerous gia na mpei sto command
    int i,nl,k,j,counter,**space_replaced,max;
    char *token=NULL;
    nl=0;
    while(input[nl]!='\n' && input[nl]!=0) nl++;
    for(i=nl;i<MAXSIZE;i++) input[i]=0; //kleinoume swsta to input
    if(check_correct_input(input)==0) return ;
    counter=0;
    i=0;
    while(input[i]){
        if( input[i]=='\"') counter++; //metrame posa strings mesa se " " h ' ' exoyme
        i++;
    }
    char **strings;
    counter=counter/2; //an broume 2 " , kseroume oti einai 1 string
    space_replaced=(int **) malloc(counter*sizeof(int *));
    strings=(char **) malloc(counter*sizeof( char *));
    for(i=0;i<counter;i++) {
        space_replaced[i]=(int *) malloc(MAXSIZE*sizeof(int));
        for(int j=0;j<MAXSIZE;j++) space_replaced[i][j]=0; //edw tha kanoume 1 opou broume keno mesa sta strings gia na kseroume meta to parsing na to ksanaftiaksoume
        strings[i]=(char *) malloc(MAXSIZE*sizeof(char));
    }
    int index;
    index=0;
    for(i=0;i<strlen(input);i++){
        if(input[i]!=0){
            if(input[i]=='\"'){
                k=0; //gia na mpainoun stous pinakes strings kai space_replaced
                strings[index][k++]=input[i];//kanoume aplo copy edw to quote.  
                j=i+1; //apo mia thesi meta,afou to prwto eisagwgiko mpike
                for(j;j<MAXSIZE;j++){
                    if(input[j]!=0){
                        if(input[j]!=input[i]){ //dhladh de ftasame sto telos tou string entos eisagwgikwn
                            if(input[j]==' ')       {space_replaced[index][k]=1; input[j]='/'; strings[index][k]='/'; } //analoga tin timi tou sumbolou, kratame kai enan arithmo gia na kseroume pws tha ta epanaferoume meta to parsing ana keno/;/|
                            else if (input[j]==';') {space_replaced[index][k]=2; input[j]='/'; strings[index][k]='/'; }     
                            else if (input[j]=='|') {space_replaced[index][k]=3; input[j]='/'; strings[index][k]='/'; }                                                                                                  
                            else strings[index][k]=input[j];//kanoume aplo copy edw                                
                        }
                        else {
                            strings[index][k++]=input[j];
                            strings[index][k]=0; //kleinoume kai to string.
                            break ; 
                        }//teleiwnoume edw to copy, parakatw den exei noima.
                    }
                    
                    k++;
                }
                i=j;
                index ++;
            }
        }
    }
        //exoume loipon ston pinaka space_replaced theseis pou exoun allaxtei ta kena kai ston strings ta strings me alalgmeno to keno se '/'
    i=0;  //to i metraei poses entoles exoume
    token=strtok(input,";");
    while(token!=NULL){ //xwrismos ana ;

        strcpy(a[i],token);
        token=strtok(NULL,";");
        for(j=strlen(a[i]);j<MAXSIZE;j++) a[i][j]=0; //kleinouyme kai ta upoloipa kelia
        i++;        
    }
    max=i; //max: poses protaseis exoume
    (*command)=(char ***) malloc(max*sizeof(char**));  //i, oses entoles diabasame apo input.
    for(int j=0;j<max;j++) (*command)[j]=(char **) malloc(MAXSIZE*sizeof(char *)); //max 50 lekseis
    for( k=0;k<i;k++) for(int j=0;j<MAXSIZE;j++)  { (*command)[k][j]=(char *) malloc(MAXSIZE* sizeof(char)); strcpy((*command)[k][j],""); }//max MAXSIZE characters kathe leksi
    (*indexes_array)=(int *) malloc(max*sizeof(int)); //pinakas pou deixnei mexri pou exoun apothikeytei lekseis gia kathe antikeimeno
    (*cind)=0; //ksekiname na pername ena ena spasmena ana leksi ta commands apo ton a ston pinaka command
    for(int j=0;j<max;j++){ //edw kanw parse kathe mia protasi ksexwrisa.
        k=0;
        token=strtok(a[j]," ");
        while(token!=NULL){
            strcpy((*command)[*(cind)][k++],token);
            token=strtok(NULL," ");
        }
        (*indexes_array)[*(cind)]=k; //an exei ftasei mexri to 5 logw tou k++, einai mexri tin 4i thesi stoixeia.
        (*cind)++;
    }
        //twra prepei na kanw restore ta kena entos tou command
   //ksekiname to restoration.
    int l,w;
    for(i=0;i<max;i++){
        for(j=0;j<(*indexes_array)[i];j++){
            for(w=0;w<counter;w++){
                if(strcmp((*command)[i][j],strings[w])==0){
                    for(l=0;l<strlen(strings[w]);l++){
                        if(space_replaced[w][l]==1) (*command)[i][j][l]=' ';
                        else if(space_replaced[w][l]==2) (*command)[i][j][l]=';';
                        else if(space_replaced[w][l]==3) (*command)[i][j][l]='|';
                    }
                }
            }
        }
    }

    for(i=0;i<counter;i++) { free(space_replaced[i]); free(strings[i]);}
    free(space_replaced);
    free(strings);
}





int search_in_array(char **command,int cind, char* c){
    int i,j;
    for(i=0;i<cind;i++) if(strcmp(command[i],c)==0) return 1;
    return 0;
}


void open_redirections(char **command,int *string_index){
    int i,in_fd,out_fd,counter=0; //counter: poses anakateythinseis metrisame gia na bgaloume apo tin entoli ta "<" "file"
    int *pos;
    pos=(int *) malloc((*string_index)*sizeof(int)); //an string_index=5, simainei oti exoume 
    for(i=0;i<*string_index;i++) pos[i]=1; //ston pos kratame poies lekseis theloume na kratisoume stin teliki protasi
    for(i=0;i<*string_index;i++){
        if(strcmp(command[i],"<")==0){ //tha anoiksoume to arxeio auto
            counter++;
            pos[i]=0; //den to theloyme to symbolo <
            i++; //pame sto onoma tou arxeiou
            pos[i]=0; //episis den theloume to onoma arxeioy
            in_fd=open(command[i],O_RDONLY ); //apla to anoigoume gia diabasma
            if(in_fd==-1) perror("redirection.");
            dup2(in_fd,0);
            close(in_fd); //kanoume to in_fd na einai i eisodos.
        }
        else if(strcmp(command[i],">")==0){
            pos[i]=0;
            i++;
            pos[i]=0;
            counter++;
            out_fd=open(command[i],O_WRONLY | O_TRUNC |O_CREAT,0644); //an den yparxei to ftiaxnoume, an yparxei idi diagrafoume ta periexomena,apla gia grapsimo ekei
            if(out_fd==-1) perror("redirection.");
            dup2(out_fd,1);
            close(out_fd); //kanoume to out_fd na einai i eksodos.
        }
        else if(strcmp(command[i],">>")==0){
            counter++;
            pos[i]=0;
            i++;
            pos[i]=0;
            out_fd=open(command[i], O_WRONLY | O_APPEND |O_CREAT,0644); //an den yparxei to ftiaxnoume, an yparxei idi kanouome append ekei,apla gia grapsimo ekei
            if(out_fd==-1) perror("redirection.");
            dup2(out_fd,1);
            close(out_fd); //kanoume to stdout na einai i eksodos.
        }
    }
    if(counter>0){ //an broume estw kai mia anakateythinsi
        char **temp; //poy tha kratisoume ta orismata tis protasis gia na bgaloume tis anakateythinseis kai na feroume ta orismata stin arxi tou pinaka command
        int arg_counter,k=0;
        arg_counter=*string_index-2*counter; //2*counter epeidh leksi fylassetai se allo keli, ara to < file einai mia anakateythinsi, alla metraei gia 2 lekseis.
        temp=(char **) malloc(arg_counter*sizeof(char* ));
        for(i=0;i<arg_counter;i++) temp[i]=(char *) malloc (MAXSIZE*sizeof(char));
        k=0;
        for(i=0;i<*string_index;i++){
            if(pos[i]==1) strcpy(temp[k++],command[i]);
        }
        for(i=0;i<arg_counter;i++){
            strcpy(command[i],temp[i]); //ta ksanabazoume ta arguments ston pinaka.
        }
        *string_index=arg_counter;
        for(i=0;i<arg_counter;i++) free(temp[i]);
        free(temp);        
    }
    free(pos);
}
char ***seperate_commands(char **command,int string_index,int *size,int **length){
    int i,j,k,counter;
    counter=0;
    char ***temp;
    for(i=0;i<string_index;i++) if(strcmp(command[i],"|")==0) counter ++; //metrame posa | einai stin entoli
    temp=(char ***) malloc((counter+1)*sizeof(char **));
    (*length)=(int *) malloc((counter+1)*sizeof(int ));
    for(i=0;i<counter+1;i++){ 
        temp[i]=(char **) malloc(MAXSIZE*sizeof(char *));
        for(j=0;j<MAXSIZE;j++) temp[i][j]=(char *) malloc(MAXSIZE*sizeof(char));
    }
    *size=counter+1; //kratame plithos protasewn se mia metabliti gia na kseroume megethos pinaka
    counter=0; //bazoume ta stoixeia sti swsti thesi.
    j=0;
    k=0;
    for(i=0;i<string_index;i++){
        if(strcmp(command[i],"|")==0){//an broume |, ftasame sto telos tis protasis kai pame na gemisoume tin epomeni.
            (*length)[j]=counter; //apothikeyoume metriti leksewn gia tin twrini protasi.
            j++; //pame stin epomeni grammi.
            k=0; //reset kai deikti leksewn
            counter=0; //reset to counter gia tin epomeni entoli
        }
        else{
            strcpy(temp[j][k++],command[i]); //antigrafoume ti leksi stin katallili thesi.
            counter++; //enimerwnoume metriti leksewn kathe protasis.
            if(i==string_index-1) (*length)[j]=counter; //epeidh den exw sto telos |, prepei na apothikeysw etsi ta stoixeia tis teleytaias protasis.
        }
    }
     //poses protaseis exoume
    return temp; 
    
}   