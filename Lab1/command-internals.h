// UCLA CS 111 Lab 1 command internals

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
#ifndef COMMAND_INT_H
#define COMMAND_INT_H
#include <time.h>
#include <sys/resource.h>
enum command_type
  {
    IF_COMMAND,		 // if A then B else C fi
    PIPE_COMMAND,        // A | B
    SEQUENCE_COMMAND,    // A ; B
    SIMPLE_COMMAND,      // a simple command
    SUBSHELL_COMMAND,    // ( A )
    UNTIL_COMMAND,	 // until A do B done
    WHILE_COMMAND,	 // while A do B done
  };

enum token_types {
  IF,
  THEN,
  ELSE,
  FI,
  WHILE,
  UNTIL,
  DO,
  DONE,
  LPAREN,
  RPAREN,
  PIPE,
  SEMICOLON,
  IDENTIFIER,
  LESSTHAN,
  GREATERTHAN,
  NEWLINE,
};

// Data associated with a command.
struct command
{
  enum command_type type;

  // Exit status, or -1 if not known (e.g., because it has not exited yet).
  int status;

  // I/O redirections, or null if none.
  char *input;
  char *output;

  union
  {
    // For SIMPLE_COMMAND.
    char **word;

    // For all other kinds of commands.  Trailing entries are unused.
    // Only IF_COMMAND uses all three entries.
    struct command *command[3];
  } u;
};

struct token {
  char *value;
  int size;
  enum token_types token_type;
};

struct profiling_time {
  struct timespec absolute_time;
  struct timespec real_time_start;
  struct timespec real_time_end;
  struct timespec abs_res;
  struct timespec real_res;
  struct rusage cpu_time_start;
  struct rusage cpu_time_end;
  command_t command;
  pid_t process_id;
};

void write_log(struct profiling_time* profile_times);

#endif
