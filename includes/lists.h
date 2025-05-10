typedef struct list_head* head;//gia ta aliases
typedef struct list_node node; 
typedef struct list_node* node_ptr;
typedef struct process_list* pid_list; //gia ti lista me ta bg processes.
typedef struct process_list_node * pid_node_ptr;
typedef struct process_list_node pid_node;
extern pid_list pl; //global giati tha elegxoume sti shell prin tin ektelesi kathe entolis an exei teleiwsei kapoia bg entoli na tin kanoume wait.


pid_list pid_list_init();
int pid_list_is_empty(pid_list );
void pid_list_add(pid_list ,pid_t  );
void pid_list_destroy(pid_list );
void pid_list_remove(pid_list  ,pid_t );
void check_for_zombies(pid_list );
void alias_management(head ,char *, char **, int ,int );
void list_remove(head ,char * );
head list_init();
void list_destroy(head );
void list_append(head , char* ,char** ,int );
int list_search(head , char* );
void get_alias(head , char * , char **,int *,int **,char ****,int *,int );
void list_print(head);
void list_update(head , char * , char **, int );
int no_of_entries(head );
void process_print(pid_list);













