// UCLA CS 111 Lab 1 command execution

// Copyright 2012-2014 Paul Eggert.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


void r_execute(command_t c, int in, int out);
void execute_if(command_t c, int in, int out);
void execute_while(command_t c, int in, int out);
void execute_until (command_t c, int in, int out);
void execute_sequence(command_t c, int in, int out); 
void execute_simple(command_t c, int in, int out);
void execute_subshell(command_t c, int in, int out);
void execute_pipe(command_t c, int in, int out); 

int
prepare_profiling (char const *name)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  error (0, 0, "warning: profiling not yet implemented");
  return -1;
}

int
command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, int profiling)
{
  r_execute(c, -1, -1); //no file descriptors are yet set
}

/*Will be recursively called in order to execute down the command
trees. The two last arguments will represent the file descriptor 
numbers if any command tree contains pipes/redirections. */ 
void r_execute(command_t c, int in, int out) {
  enum command_type type = c->type;
  
  switch(type) {

  case IF_COMMAND: 
    execute_if(c, in, out);
    break;
  case WHILE_COMMAND:
    execute_while(c, in, out);
    break;
  case UNTIL_COMMAND:
    execute_until(c, in, out);
    break;
  case SEQUENCE_COMMAND:
    execute_sequence(c, in, out);
    break; 
  case SIMPLE_COMMAND:
    execute_simple(c, in, out);
    break;
  case SUBSHELL_COMMAND:
    execute_subshell(c, in, out);
    break;
  case PIPE_COMMAND:
    execute_pipe(c, in, out);
    break;
  }

}

void execute_if(command_t c, int in, int out) {

  //Check if there are any redirections
  //Redirections are handed down the tree
  //Overwritten by redirections farther down the tree
  if (c->input != NULL)
    in = open(c->input, O_RDONLY|O_CREAT);
  if (c->output != NULL)
    out = open(c->output, O_WRONLY|O_CREAT|O_TRUNC);

  r_execute(c->u.command[0], in, out);

  if (c->u.command[0]->status == 0) {
    r_execute(c->u.command[1], in, out);
    c->status = c->u.command[1]->status;
  }
  else if (c->u.command[2] != NULL) {
    r_execute(c->u.command[2], in, out);
    c->status = c->u.command[2]->status;
  }
  c->status = c->u.command[0]->status;

}


void execute_while(command_t c, int in, int out) {

  if (c->input != NULL)
    in = open(c->input, O_RDONLY|O_CREAT);
  if (c->output != NULL)
    out = open(c->output, O_WRONLY|O_CREAT|O_TRUNC);

  r_execute(c->u.command[0], in, out);

  //while the while condition is false (until break out of loop)
  while (c->u.command[0]->status != 0) {
    r_execute(c->u.command[1], in, out);
    c->status = c->u.command[1]->status;
  }
  c->status = c->u.command[0]->status;
}

void execute_until(command_t c, int in, int out) {
  
  if (c->input != NULL)
    in = open(c->input, O_RDONLY|O_CREAT);
  if (c->output != NULL)
    out = open(c->output, O_WRONLY|O_CREAT|O_TRUNC);

  r_execute(c->u.command[0], in, out);

  //while the until condition is true (until it is false)
  while (c->u.command[0]->status == 0) {
    r_execute(c->u.command[1], in, out);
    c->status = c->u.command[1]->status;
  }
  c->status = c->u.command[0]->status;
} 

void execute_sequence(command_t c, int in, int out) {
  
  r_execute(c->u.command[0], in, out);
  r_execute(c->u.command[1], in, out);

  //Bash convention
  c->status = c->u.command[1]->status;
}

void execute_simple(command_t c, int in, int out) {

  if (c->input != NULL)
    in = open(c->input, O_RDONLY|O_CREAT);
  if (c->output != NULL)
    out = open(c->output, O_WRONLY|O_CREAT|O_TRUNC);

  int status;

  pid_t p = fork();
  if (p == 0) {   //We are in the child
    if (in != -1)  //Replace file descriptors if there are redirects/pipes
      dup2(in, STDIN_FILENO);
    if (out != -1)
      dup2(out, STDOUT_FILENO);
    execvp(c->u.word[0], c->u.word);
    error(1, 0, "Execvp returned");    //execvp returned, but if successful should destroy itself
  }
  //Error in forking
  else if (p < 0) {
    error(1, 0, "Failed to fork.");
  }
  //Wait for the child process to finish
  else {
    waitpid(p, &status, 0);
  }

  if (WIFEXITED(status))   //check to see that wait exits normally
    c->status = WEXITSTATUS(status);   //set aassociated status
  else if (WIFSTOPPED(status))   //in case wait is stopped
    c->status = WSTOPSIG(status);
  else if (WIFSIGNALED(status))   //if wait signals an error
    c->status = WTERMSIG(status);
}

void execute_subshell(command_t c, int in, int out) {
  
  int status;

  if (c->input != NULL)
    in = open(c->input, O_RDONLY|O_CREAT);
  if (c->output != NULL)
    out = open(c->output, O_WRONLY|O_CREAT|O_TRUNC);

  pid_t p = fork();
  //Error in forking
  if (p < 0) 
    error(1, 0, "Failed to fork");
  else if (p == 0) {
    r_execute(c->u.command[0], in, out);
    //Save the exit status
    exit(c->u.command[0]->status);
  }
  else {
    waitpid(p, &status, 0);
  }

  if (WIFEXITED(status))
    c->status = WEXITSTATUS(status);
  else if (WIFSTOPPED(status))
    c->status = WSTOPSIG(status);
  else if (WIFSIGNALED(status))
    c->status = WTERMSIG(status);
}

void execute_pipe(command_t c, int in, int out){

  int pid1, pid2, status;
  int fd[2];

  pipe(fd);

  switch (pid1 = fork()){
 
  case 0:

    switch (pid2 = fork()) {

    case 0: /* child */
     
      close(fd[0]);/*does not need this end of the pipe */
      r_execute(c->u.command[0], in, fd[1]);
      exit(command_status(c->u.command[0]));

     
    default: /* parent*/
      close(fd[1]);
      r_execute(c->u.command[1], fd[0], out);
      //  printf("BEFORE\n");
      waitpid(pid2, &status, 0);
      // printf("AFTER");
      exit(command_status(c->u.command[1]));

    case -1:
      perror("Failed to fork second time");
      exit(1);
    }

  default:/*parent*/
    close(fd[0]);
    close(fd[1]);
    waitpid( pid1, &status, 0);
    c->status = WEXITSTATUS(&status);
    break;
  case -1:
    perror("Failed fork first time");
    exit(1);
  }

  exit(0);
  
}

