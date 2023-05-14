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

int main(){ 
    shell();      
  return 0;
}


