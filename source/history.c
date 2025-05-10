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
char ***history_init(){ 
    char ***history; //tridiastatos pinakas na apothikeyw H_MAXSIZE entoles,spasmenes ana leksi na tis exw etoimes.
    int i,j,k;
    history=(char ***) malloc(H_MAXSIZE*sizeof(char **));
    for(i=0;i<H_MAXSIZE;i++) {
        history[i]=(char **) malloc(MAXSIZE*sizeof(char*));
        for(j=0;j<MAXSIZE;j++) {
            history[i][j]=NULL; //arxika de desmeyoume xwro gia to kathe string, bazoume null gia na kseroume poia einai kena
        }
    }
    return history;
}
void destroy(char ***history){  
    int i,j;
     for(i=0;i<H_MAXSIZE;i++) {
        for(j=0;j<MAXSIZE;j++) if(history[i][j]!=NULL) free(history[i][j]);  //for free errors, an den exw balei 20 entoles.
        free(history[i]);
    }
    free(history);
}
void print_history(char ***h){
    int i,j;

    for(i=0;i<H_MAXSIZE;i++){
        printf("%d: ",i+1);   
        for(j=0;j<MAXSIZE ;j++){ 
            if(h[i][j]!=NULL) printf("%s  ",h[i][j]);
        }
        printf("\n");
    }
    return ;
}

int history_update(char ****h,char **command,int string_index){ //string_index: mexri poia thesi exw lekseis
    int i,j,identical,counter,same;
    i=0;
    while(i<H_MAXSIZE){
        if((*h)[i][0]==NULL) break;  //elegxoume to prwto keli tis entolis, an einai NULL, den exei kati mesa
        i++;
    }
    if(i>0){  //dhladi exei mesa stoixeia
        same=1;
        counter=0;
        for(j=0;j<MAXSIZE;j++) if((*h)[i-1][j]!=NULL) counter++; //poses theseis exoun periexomeno
        for(j=0; j<string_index;j++){ //psaxnoume eite mexri to telos tou pinaka eite mexri to telos tou command an to teleytaio entry tou history einai idio me to trexon command.an einai idia, den to bazoume
            if((*h)[i-1][j]==NULL) { same=0; break;} //teleiwnei to periexomeno tou history prin tou command, ara den einai idia
            else if(strcmp((*h)[i-1][j],command[j])!=0){ same=0; break;} //kai stis duo periptwseis teleiwnoume to psaksimo edw
        }
        identical=same && (counter==string_index);
        if(identical==1){ return 0;} //an einai idio to command me tin teleytaia entoli pou einai apothikeymeni sto history, den tin pername
        if(i==H_MAXSIZE){ //exei gemisei o pinakas, metakinw mia thesi ola ta stoixeia pisw gia na balw to kainourgio command sti teleytaia thesi tou pinaka
            for(i=1;i<H_MAXSIZE;i++){
                for(j=0;j<MAXSIZE;j++){
                    if((*h)[i-1][j]==NULL &&  (*h)[i][j]!=NULL){ //an einai adeio to keli tou proigoumenou stoixeioy kai to twrino keli den einai, desmeyoyme mnimi gia na kanoume copy 
                        (*h)[i-1][j]=(char *)malloc(MAXSIZE*sizeof(char)); //desmeyoyme xwro gia to proigoumeno gia na to kanoume copy
                        strcpy((*h)[i-1][j],(*h)[i][j]);
                    }
                    else if((*h)[i][j]==NULL){// an einai adeio to keli tou twrinou stoixeiou, kanoume to idio kai gia to apo panw tou
                        free((*h)[i-1][j]);
                        (*h)[i-1][j]=NULL; //to kanoume NULL 
                    } 
                    else if((*h)[i-1][j]!=NULL &&  (*h)[i][j]!=NULL){                        
                        strcpy((*h)[i-1][j],(*h)[i][j]);
                    } //an kanena apo ta dyo den einai adeia, kanoume strcpy kateytheian
                }
            }//enimerwnoume to teleytaio stoixeio tou pinaka
            for(j=0;j<MAXSIZE;j++) if((*h)[H_MAXSIZE-1][j]!=NULL){ //kanoume oli ti grammi reset se null gia na baloume me ena for ta kainourgia strings.
                free((*h)[H_MAXSIZE-1][j]); 
                (*h)[H_MAXSIZE-1][j]=NULL; 
            } 
            for(j=0;j<string_index;j++){ //edw malloc osa pedia xreiazontai gia na ginei to copy.
                (*h)[H_MAXSIZE-1][j]=(char *) malloc(MAXSIZE*sizeof(char)); 
                strcpy((*h)[H_MAXSIZE-1][j],command[j]);
                }
        }
        else{ //den einai gematos, bazoume stin i+1-osti thesi to neo command
           // i++; //pame stin epomeni tehsi, pou kseroume einai adeia.
            for(j=0;j<string_index;j++){(*h)[i][j]=(char *) malloc(MAXSIZE*sizeof(char)); strcpy((*h)[i][j],command[j]);} //desmeyoume xwro na mpei to kathe string
        }

    }  //terma adeios o pinakas, mono mia fora tha mpei edw.
    else{       
        for(j=0;j<string_index;j++){
            (*h)[i][j]=(char *) malloc(MAXSIZE*sizeof(char)); //desmeyoume xwro na mpei to kathe string
            strcpy((*h)[i][j],command[j]);
        } 
    }
    return 0;
}

//history_get(h,[1,20],command[i],index_array[i]); , opou command[i] : kathe thesi kai ena string
void history_get(char ***h,int index,char ** command,int *command_index){   //history 3 == history[2][i][]
    if(index>=1 && index<=H_MAXSIZE){  //[1,20] == [0,19]
        if(h[index-1][0]==NULL) { write(2,"not enough commands have been entered.\n",strlen("not enough commands have been entered.\n")); return; }
        int j;
        for(j=0;j<MAXSIZE;j++){
            if(h[index-1][j]==NULL) break; //den exei allo na perasoume, stamatame edw.
            strcpy(command[j],h[index-1][j]);
        }
        *command_index=j; //an  history[i][3]=NULL, exoume stoixeia apo to 0 ews to 2, ara 3
    }
    else write(2,"invalid range of argument for history.\n",strlen("invalid range of argument for history.\n"));
    return ;
}  