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
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#define BILLION 1000000000
#define MILLION 1000000

static int profile_descriptor;


/*Write timing information to a file*/
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
  chmod(name, S_IRWXU|S_IRWXG|S_IROTH|S_IWOTH);
  return open(name, O_WRONLY|O_CREAT|O_APPEND, 0666);
}

int
command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, int profiling)
{
  profile_descriptor = profiling;

  r_execute(c, -1, -1); //no file descriptors are yet set

}

void write_log(struct profiling_time* profile_times){
 
 
  char time_string[1024];
  //casting? 
  double absolute_time= profile_times->absolute_time.tv_sec +  profile_times->absolute_time.tv_nsec/(double)BILLION;
  double real_time_start =  profile_times->real_time_start.tv_sec +  profile_times->real_time_start.tv_nsec/(double)BILLION;
  double real_time_end = profile_times->real_time_end.tv_sec +  profile_times->real_time_end.tv_nsec/(double)BILLION;
  double user_time_start = profile_times->cpu_time_start.ru_utime.tv_sec +  profile_times->cpu_time_start.ru_utime.tv_usec/(double)MILLION;
  double system_time_start = profile_times->cpu_time_start.ru_stime.tv_sec +  profile_times->cpu_time_start.ru_stime.tv_usec/(double)MILLION;  
  double user_time_end = profile_times->cpu_time_end.ru_utime.tv_sec +  profile_times->cpu_time_end.ru_utime.tv_usec/(double)MILLION;
  double system_time_end = profile_times->cpu_time_end.ru_stime.tv_sec +  profile_times->cpu_time_end.ru_stime.tv_usec/(double)MILLION;
  
  int string_counter = snprintf(time_string, 1023, "%.*f %.*f %f %f",\
				(int)(9-log10(profile_times->abs_res.tv_nsec)),\
				absolute_time,\
				(int)(9-log10(profile_times->real_res.tv_nsec)),\
				real_time_end - real_time_start,\
				user_time_end - user_time_start,\
				system_time_end - system_time_start); 
  // printf("string counter: %d\n", string_counter);
  //printf("%f %f %f %f\n", absolute_time, real_time_end-real_time_start, user_time_end-user_time_start, system_time_end-system_time_start);
  int command_counter = 0;

  if (profile_times->command == NULL){
    string_counter += snprintf(time_string+string_counter, 1023-string_counter, " [%d]", profile_times->process_id);
  }
  else {
    while(profile_times->command->u.word[command_counter] != NULL){
    //printf("command: %s\n", profile_times->command->u.word[command_counter]);
      string_counter += snprintf(time_string+string_counter, 1023-string_counter, " %s", profile_times->command->u.word[command_counter]);   
 // printf("%d", string_counter);
      command_counter++;
    }
  }

  time_string[string_counter] = '\n';
  // printf("%s", time_string);
  /*  struct flock lock;
  lock.l_type = F_WRLCK;
  lock.l_start = 0;
  lock.l_whence = SEEK_SET;
  lock.l_len = 0;
  lock.l_pid = getpid();
  fcntl(profile_descriptor, F_SETLKW, &lock); 
  */
  write(profile_descriptor, time_string, string_counter+1);

  //lock.l_type = F_UNLCK;
  //fcntl(profile_descriptor, F_SETLK, &lock);
}

/*Will be recursively called in order to execute down the command
trees. The two last arguments will represent the file descriptor 
numbers if any command tree contains pipes/redirections. */ 
void r_execute(command_t c, int in, int out) {
  enum command_type type = c->type;

  //Check if there are any redirections
  //Redirections are handed down the tree
  //Overwritten by redirections farther down the tree
  if (c->input != NULL)
    in = open(c->input, O_RDONLY|O_CREAT);
  if (c->output != NULL)
    out = open(c->output, O_WRONLY|O_CREAT|O_TRUNC);

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

  //Close the files
  if (c->input != NULL)
    close(in);
  if (c->output != NULL)
    close(out);

}

void execute_if(command_t c, int in, int out) {

  r_execute(c->u.command[0], in, out);  
  c->status = c->u.command[0]->status;

  if (c->u.command[0]->status == 0) {
    r_execute(c->u.command[1], in, out);
    c->status = c->u.command[1]->status;
  }
  else if (c->u.command[2] != NULL) {
    r_execute(c->u.command[2], in, out);
    c->status = c->u.command[2]->status;
  }
  
}


void execute_while(command_t c, int in, int out) {

  r_execute(c->u.command[0], in, out);

  //while the while condition is false (until break out of loop)
  while (c->u.command[0]->status != 0) {
    r_execute(c->u.command[1], in, out);
    c->status = c->u.command[1]->status;
  }
  c->status = c->u.command[0]->status;
}

void execute_until(command_t c, int in, int out) {
  
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

  int status;
  struct profiling_time profile_times;
  
  clock_getres(CLOCK_REALTIME, &profile_times.abs_res);
  clock_getres(CLOCK_MONOTONIC, &profile_times.real_res);

  if (strcmp(c->u.word[0], "exec") == 0) {
    if (in != -1)
      dup2(in, STDIN_FILENO);
    if (out != -1)
      dup2(out, STDOUT_FILENO);
    execvp(c->u.word[1], c->u.word + 1);
    error(1, 0, "Bad command"); 
  }
  clock_gettime(CLOCK_MONOTONIC, &profile_times.real_time_start);
  getrusage(RUSAGE_CHILDREN, &profile_times.cpu_time_start);
  pid_t p = fork();
  if (p == 0) {   //We are in the child
    if (in != -1)  //Replace file descriptors if there are redirects/pipes
      dup2(in, STDIN_FILENO);
    if (out != -1)
      dup2(out, STDOUT_FILENO);
    if (strcmp(c->u.word[0], ":") == 0)
      exit(0);
    execvp(c->u.word[0], c->u.word);
    error(1, 0, "Bad command");    //execvp returned, but if successful should destroy itself
  }
  //Error in forking
  else if (p < 0) {
    error(1, 0, "Failed to fork.");
  }
  //Wait for the child process to finish
  else {
    waitpid(p, &status, 0);
    clock_gettime(CLOCK_MONOTONIC, &profile_times.real_time_end);
    clock_gettime(CLOCK_REALTIME, &profile_times.absolute_time);
  
  getrusage(RUSAGE_CHILDREN, &profile_times.cpu_time_end);
    profile_times.command = c;
  }
  write_log(&profile_times);

  if (WIFEXITED(status))   //check to see that wait exits normally
    c->status = WEXITSTATUS(status);   //set aassociated status
  else if (WIFSTOPPED(status))   //in case wait is stopped
    c->status = WSTOPSIG(status);
  else if (WIFSIGNALED(status))   //if wait signals an error
    c->status = WTERMSIG(status);
}

void execute_subshell(command_t c, int in, int out) {
  
  int status;
  struct profiling_time profile_times;
  
  clock_getres(CLOCK_REALTIME, &profile_times.abs_res);
  clock_getres(CLOCK_MONOTONIC, &profile_times.real_res);

  clock_gettime(CLOCK_MONOTONIC, &profile_times.real_time_start);
  getrusage(RUSAGE_CHILDREN, &profile_times.cpu_time_start);
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
    clock_gettime(CLOCK_MONOTONIC, &profile_times.real_time_end);
    clock_gettime(CLOCK_REALTIME, &profile_times.absolute_time);
    getrusage(RUSAGE_CHILDREN, &profile_times.cpu_time_end);
    profile_times.process_id = p;
  }
  profile_times.command = NULL;
  write_log(&profile_times);

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

  struct profiling_time profile_times_left;
  struct profiling_time profile_times_right;

  clock_getres(CLOCK_REALTIME, &profile_times_left.abs_res);
  clock_getres(CLOCK_MONOTONIC, &profile_times_left.real_res);
  clock_getres(CLOCK_REALTIME, &profile_times_right.abs_res);
  clock_getres(CLOCK_MONOTONIC, &profile_times_right.real_res);
  clock_gettime(CLOCK_MONOTONIC, &profile_times_right.real_time_start);
  clock_gettime(CLOCK_MONOTONIC, &profile_times_left.real_time_start);
 
  pipe(fd);

  
  switch (pid1 = fork()){
 
  case 0:

       //getrusage(RUSAGE_CHILDREN, &profile_times_left.cpu_time_start);
     
    close(fd[0]);/*does not need this end of the pipe */
    r_execute(c->u.command[0], in, fd[1]);
    close(fd[1]);
    exit(command_status(c->u.command[0]));
  
  case -1:
    perror("Failed fork first time");
    exit(1);
  
  default:/*parent*/

    switch(pid2 = fork()){
    
    case 0:
  
      //getrusage(RUSAGE_CHILDREN, &profile_times_right.cpu_time_start);
  
      close(fd[1]);
      r_execute(c->u.command[1], fd[0], out);
      /*printf("BEFORE\n");*/
      close(fd[0]);
      //waitpid(pid2, &status, 0);
      
      //write_log(&profile_times_right);

      exit(command_status(c->u.command[1]));

    case -1:
      perror("Failed to fork second time");
      exit(1);

    default:
      close(fd[0]);
      close(fd[1]);

      memset(&profile_times_left.cpu_time_start, 0, sizeof(struct rusage));
      wait4(pid1, &status,0, &profile_times_left.cpu_time_end);

      clock_gettime(CLOCK_MONOTONIC, &profile_times_left.real_time_end);
      clock_gettime(CLOCK_REALTIME, &profile_times_left.absolute_time);
      getrusage(RUSAGE_CHILDREN, &profile_times_left.cpu_time_end);
      profile_times_left.process_id = pid1;
      profile_times_left.command = NULL;

      write_log(&profile_times_left);
      
      memset(&profile_times_right.cpu_time_start, 0, sizeof(struct rusage));
      wait4(pid2, &status,0, &profile_times_right.cpu_time_end); 
      
      clock_gettime(CLOCK_MONOTONIC, &profile_times_right.real_time_end);
      clock_gettime(CLOCK_REALTIME, &profile_times_right.absolute_time);
      getrusage(RUSAGE_CHILDREN, &profile_times_right.cpu_time_end);
      profile_times_right.process_id = pid2;
      profile_times_right.command = NULL;
      
      write_log(&profile_times_right);
     
      c->status = WEXITSTATUS(status);
 
      break;
    }

  }

}

