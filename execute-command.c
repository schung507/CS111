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

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  

void r_execute(command_t c, int in, int out);
void execute_if(command_t c, int in, int out);
void execute_while(command_t c, int in, int out);
void execute_until (command_t c, int in, int out);
void execute_sequence(command_t c, int in, int out);
void execute_simple(command_t c, int in, int out);
*/

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
  /* FIXME: Replace this with your implementation, like 'prepare_profiling'.  */
  error (1, 0, "command execution not yet implemented");
}
/*
void r_execute(command_t c, int in, int out) {
  enum command_type type = c->type;
  
  switch(type) {

  case IF_COMMAND: 
    execute_if(c, in, out);
  case WHILE_COMMAND:
  case UNTIL_COMMAND:
    execute_woru(c, in, out);
  case SEQUENCE_COMMAND:
   execute_sequence(c, in, out);
  }
}

void execute_if(command_t c, int in, int out) {

  if (r_execute(c->u.command[0]->status) == 0) {
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

  while (r_execute(c->u.command[0]->status) != 0) {
    r_execute(c->u.command[1], in, out);
    c->status = c->u.command[1]->status;
  }
  c->status = c->u.command[0]->status;
}

void execute_until(command_t c, int in, int out) {
  
  while (r_execute(c->u.command[0]->status) == 0) {
    r_execute(c->u.command[1], in, out);
    c->status = c->u.command[1]->status;
  }
  c->status = c->u.command[0]->status;
} 

void execute_sequence(command_t c, int in, int out) {
  
  r_execute(c->u.command[0], in, out);
  r_execute(c->u.command[1], in, out);
  c->status = c->u.command[1]->status;
}

void execute_simple(command_t c, int in, int out) {

}
*/
