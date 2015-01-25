#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include "alloc.h"
#include "parser.h"

//#define VLAD_DEBUG

//Implementations
command_t simple_command_parser(struct token* simple_token, int size_simple) {
  int size = size_simple;
  int i = 0;
  int WORD_SIZE=10; 
  command_t simple_statement = (command_t)checked_malloc(sizeof(struct command));
  simple_statement->type = SIMPLE_COMMAND;
  simple_statement->u.word = (char**)checked_malloc(WORD_SIZE*sizeof(char*));
  //*simple_statement->u.word = (char*)checked_malloc(WORD_SIZE*sizeof(char));
  
  //Check for redirection
   input_checker(simple_token, size, simple_statement);

  //store the simple commands into command struct
  for (i = 0; simple_token[i].token_type != SEMICOLON && i != size
	 && simple_token[i].token_type != GREATERTHAN &&
	 simple_token[i].token_type != LESSTHAN; i++)
    {
      if( i == WORD_SIZE){
        simple_statement->u.word = (char **)checked_realloc(simple_statement->u.word, 1.5*WORD_SIZE*sizeof(char*));
	//*simple_statement->u.word = (char *)checked_realloc(*simple_statement->u.word, 1.5*WORD_SIZE*sizeof(char));       
	WORD_SIZE *= 1.5;
	//printf("reallocated more words\n");
      }
      if (simple_token[i].token_type != IDENTIFIER)
	error(1, 0, "Invalid token in command");

     simple_statement->u.word[i] = simple_token[i].value;
     //printf("%s\n",  simple_statement->u.word[i]);
    /* Error checking 
	printf("%s\n", simple_statement->u.word[i]); */     
    }
  simple_statement->u.word[i] = NULL;   //end with nullpointer
  
  return simple_statement;
}

command_t if_command_parser(struct token* if_token, int size_if) {
  int size = size_if;
  int nested = 0;

  int then_position = 0;
  int fi_position = 0;
  int else_position = 0;    //only if there is an 'else' statement

  command_t ifthen;
  command_t thenelse;
  command_t torefi;
  command_t if_statement = checked_malloc(sizeof(struct command));


  int i;
  for (i = 1; i != size; i++) {

    if (if_token[i].token_type == IF &&
	if_token[i-1].token_type != IDENTIFIER)
      nested++;                          //Add when we enter a nested statement
  if (if_token[i].token_type == FI &&
      if_token[i-1].token_type != IDENTIFIER  && nested != 0)
    {
      nested--;                          //count when we leave a nested statement
      continue;
    }
  if (if_token[i].token_type == THEN &&
          if_token[i-1].token_type == SEMICOLON &&
      nested == 0)
   then_position = i;
 if (if_token[i].token_type == ELSE &&
          if_token[i-1].token_type == SEMICOLON &&
     nested == 0)
   else_position = i;
 if (if_token[i].token_type == FI &&
          if_token[i-1].token_type == SEMICOLON &&
     nested == 0)
   fi_position = i;
  }
  
  //Redirection
  if (fi_position <= (size-1))
    input_checker(if_token + fi_position + 1,
		  size - fi_position - 1, if_statement);

  if (then_position == 0)  //no 'then' is found
    error(1, 0, "No 'then' to complete 'if'");
  if (fi_position == 0)   //no 'fi' is found
    error(1, 0, "No 'fi' to complete 'if'");
  if (fi_position < size-1)
    {
      if (if_token[fi_position+1].token_type != PIPE &&
          if_token[fi_position+1].token_type != SEMICOLON &&
          if_token[fi_position+1].token_type != LESSTHAN &&
          if_token[fi_position+1].token_type != GREATERTHAN)
        error(1,0, "Invalid input after end of 'if' conditional");
    }


  //not including 'then'
  ifthen = parser(if_token + 1, then_position - 1);

  //check if there is an 'else' statement
  if (else_position != 0)
    {
      thenelse = parser(if_token + then_position + 1, 
			else_position - then_position - 1);
      torefi = parser(if_token + else_position + 1, 
		      fi_position - else_position - 1);
    }

  //otherwise proceed with thenfi
  if (else_position == 0)
    torefi = parser(if_token + then_position + 1, 
		    fi_position - then_position - 1);

  //Store into command struct
  if_statement->type = IF_COMMAND;
  //if_statement->input = NULL;
  //if_statement->output = NULL;

  if_statement->u.command[0] = ifthen;
  if (else_position != 0)
    {
      if_statement->u.command[1] = thenelse;
      if_statement->u.command[2] = torefi;
    }
  if (else_position == 0)
    {
    if_statement->u.command[1] = torefi;
    if_statement->u.command[2] = NULL;
    }

  return if_statement;

}

command_t woru_parser(struct token* woru_token, int size_woru)
{
  int size = size_woru;
  int nested = 0;          //keeps track of nested commands of the same type
  int do_position = 0;
  int done_position = 0;
  command_t wudo;
  command_t dodone;
  command_t woru = checked_malloc(sizeof(struct command));

  int i;
  for (i = 1; i != size; i++) {
    if (woru_token[i].token_type == WHILE
	&& woru_token[i-1].token_type != IDENTIFIER)
      nested++;
    if (woru_token[i].token_type == UNTIL
	&& woru_token[i-1].token_type != IDENTIFIER)
      nested++;
    if (woru_token[i].token_type == DONE &&
	woru_token[i-1].token_type != IDENTIFIER  && nested != 0)
      {
	nested--;
	continue;
      }
    if (woru_token[i].token_type == DO
    && woru_token[i-1].token_type == SEMICOLON
    && nested == 0)
      do_position = i;
    if (woru_token[i].token_type == DONE
        && woru_token[i-1].token_type == SEMICOLON
        && nested == 0)
      done_position = i;
  }

  /*Error checking 
  printf("%i\n", do_position);
  printf("%i\n", done_position); */
 
  //Redirection
  if (done_position <= (size-1))
    {
    input_checker(woru_token + done_position + 1, 
		  size - done_position - 1, woru);
    }

  if (do_position == 0)
    error(1, 0, "No 'do' to complete 'while'");
  if (done_position == 0)
    error(1, 0, "No 'done' to complete 'while'");
  if (done_position < size-1)
    {
      if (woru_token[done_position+1].token_type != PIPE &&
         woru_token[done_position+1].token_type != SEMICOLON &&
         woru_token[done_position+1].token_type != LESSTHAN &&
	 woru_token[done_position+1].token_type != GREATERTHAN)
        error(1,0, "Invalid input after matching 'done'");
    }


  /* Error checking 
  int k;
  for (k = 0; k != do_position - 2; k++)
    {
      printf("%s\n", woru_token[k+1].value); 
     } */

  wudo = parser(woru_token + 1, do_position - 1);
  dodone = parser(woru_token + do_position + 1,
		  done_position - do_position - 1);

  if (woru_token[0].token_type == WHILE)
    woru->type = WHILE_COMMAND;
  if (woru_token[0].token_type == UNTIL)
    woru->type = UNTIL_COMMAND;

  woru->u.command[0] = wudo;
  woru->u.command[1] = dodone;

  return woru;
}

command_t subshell_parser(struct token* sub_token, int size_sub) {
  int size = size_sub;
  int nested = 0;
  int position_right = 0;

  command_t subshell = checked_malloc(sizeof(struct command));

  int i;
  for (i = 1; i != size; i++)
    {
      if (sub_token[i].token_type == LPAREN)
        nested++;
      if (sub_token[i].token_type == RPAREN && nested == 0)
	{
	  position_right = i;
	  break;
	}
      if (sub_token[i].token_type == RPAREN && nested != 0)
	  nested--;
    }

  //Redirection
  if (position_right <= size-1)
    input_checker(sub_token + position_right + 1, 
		  size - position_right - 1, subshell);

  if (position_right == 0)
    error(1, 0, "No ')' to match '('");
  if (nested != 0)
    error(1, 0, "Number of ')' does not match '('");
  if (position_right < size-1)
    {
      if (sub_token[position_right+1].token_type != PIPE &&
         sub_token[position_right+1].token_type != SEMICOLON &&
         sub_token[position_right+1].token_type != LESSTHAN &&
	  sub_token[position_right+1].token_type != GREATERTHAN)
        error(1,0, "Invalid input after matching ')'");
    }
  
  command_t subcommand = parser(sub_token + 1, position_right - 1);
  subshell->type = SUBSHELL_COMMAND;
  subshell->u.command[0] = subcommand;

  return subshell;
}

command_t pipe_parser(struct token* pipe_token, int size_pipe, int pipe_loc) {
  int size = size_pipe;
  int pipe_position = pipe_loc;
  command_t beforepipe;
  command_t afterpipe;
  command_t pipe_command = checked_malloc(sizeof(struct command));

  if (pipe_loc == size - 1)
    error(1, 0, "Invalid '|' at end of statement");

  beforepipe = parser(pipe_token, pipe_position);
  afterpipe = parser(pipe_token + (pipe_position + 1),
                     (size - 1) - pipe_position);

  pipe_command->type = PIPE_COMMAND;
  pipe_command->input = NULL;
  pipe_command->output = NULL;
  pipe_command->u.command[0] = beforepipe;
  pipe_command->u.command[1] = afterpipe;

  return pipe_command;
}

int pipe_checker(struct token* pipe_token, int size_pipe) {

  enum token_types first = pipe_token[0].token_type;
  int size = size_pipe;
  int nested = 0;
  int left = 1;
  int right = 0;

  int i;
  for (i = 1; i != size; i++)
    {
      //An IF_STATEMENT with pipe
      if (first == IF)
	{
	  if (pipe_token[i].token_type == IF &&
	      pipe_token[i-1].token_type != IDENTIFIER)
	    nested++;
	  if (pipe_token[i].token_type == FI &&
	      pipe_token[i-1].token_type != IDENTIFIER  && nested != 0)
	      nested--;
	  if (pipe_token[i].token_type == PIPE &&
             pipe_token[i-1].token_type == FI &&
             nested == 0 &&
	      (i-2) >= 0)
	    {
	      if (pipe_token[i-2].token_type == SEMICOLON)
		{
		  /*printf("%i", i); */
		  return i;
		}
	    }
	}
      //A WHILE_STATEMENT or UNTIL_STATEMENT with pipe
      if (first == WHILE || first == UNTIL)
	{
          if (pipe_token[i].token_type == WHILE &&
              pipe_token[i-1].token_type != IDENTIFIER)
            nested++;
	  if (pipe_token[i].token_type == UNTIL &&
              pipe_token[i-1].token_type != IDENTIFIER)
            nested++;
          if (pipe_token[i].token_type == DONE &&
              pipe_token[i-1].token_type != IDENTIFIER  && nested != 0)
	    nested--;
	  if (pipe_token[i].token_type == PIPE &&
             pipe_token[i-1].token_type == DONE &&
             nested == 0 && (i-2) >= 0)
            {
              if (pipe_token[i-2].token_type == SEMICOLON)
                {
                  /*printf("%i\n", i); */ 
                  return i;
                }
            }
        }
      //A SUBSHELL_STATEMENT with pipe
    if (first == LPAREN) {
      if (pipe_token[i].token_type == LPAREN)
	left++;
      if (pipe_token[i].token_type == PIPE 
	  && pipe_token[i-1].token_type == RPAREN
	  && left == right)
	return i;
      if (pipe_token[i].token_type == RPAREN)
	right++;
    }

      //A SIMPLE_COMMAND with pipe
      if (first == IDENTIFIER)
	{
	  if (pipe_token[i].token_type == PIPE)
	    return i;
	}
    }
  return 0;
}

command_t sequence_parser(struct token* sequence_token, int size_sequence, int sequence_loc) {
  int size = size_sequence;
  int sequence_position = sequence_loc;
  command_t beforesequence;
  command_t aftersequence;
  command_t sequence_command = checked_malloc(sizeof(struct command));

  if (sequence_position != size-1 &&
      sequence_token[sequence_position+1].token_type == SEMICOLON)
    error(1, 0, "Invalid ';' following ';'");

  beforesequence = parser(sequence_token, sequence_position);
  aftersequence = parser(sequence_token + (sequence_position + 1),
			 (size - 1) - sequence_position);

  sequence_command->type = SEQUENCE_COMMAND;
  sequence_command->input = NULL;
  sequence_command->output = NULL;
  sequence_command->u.command[0] = beforesequence;
  sequence_command->u.command[1] = aftersequence;

  return sequence_command; 
  return 0;
}

int end_of_statement(struct token* count_token, int size_count) {
  enum token_types first = count_token[0].token_type;
  int size = size_count;
  int nested = 0;
  int left = 1;
  int right = 0;

  int i;
  for (i = 1; i != size; i++) {

    //IF                                                                        
    if (first == IF)
      {
	if (count_token[i].token_type == IF &&
	    count_token[i-1].token_type != IDENTIFIER)
	  nested++;                          //Add when we enter a nested statement                                               
	if (count_token[i].token_type == FI &&
	    count_token[i-1].token_type != IDENTIFIER  && nested != 0)
	  {
	    nested--;                          //count when we leave a nested statement   
	    continue;
	  }
	if (count_token[i].token_type == FI &&
          count_token[i-1].token_type == SEMICOLON &&
	    nested == 0)
	  return i;      //position of outermost fi 
      }

    //WHILE or UNTIL                               
    if (first == WHILE || first == UNTIL) {
      if (count_token[i].token_type == WHILE
          && count_token[i-1].token_type != IDENTIFIER)
        nested++;
      if (count_token[i].token_type == UNTIL
          && count_token[i-1].token_type != IDENTIFIER)
        nested++;
      if (count_token[i].token_type == DONE &&
          count_token[i-1].token_type != IDENTIFIER  && nested != 0)
        {
          nested--;
          continue;
        }
      if (count_token[i].token_type == DONE
        && count_token[i-1].token_type == SEMICOLON
          && nested == 0)
        return i;   //position of outermost done   
    }

    //SUBSHELL
    if (first == LPAREN) {
    if (count_token[i].token_type == LPAREN)
      left++;
    if (count_token[i].token_type == SEMICOLON 
	&&count_token[i-1].token_type == RPAREN
	&& left == right)
      return i;
    if (count_token[i].token_type == RPAREN)
      right++;
    }

    //SIMPLE                                         
    if (first == IDENTIFIER)
      { 
	if (count_token[i].token_type == IF || count_token[i].token_type == WHILE ||
	    count_token[i].token_type == UNTIL || count_token[i].token_type == LPAREN) {
	  if (count_token[i-1].token_type != IDENTIFIER)
	    nested++;
	}	
	if (count_token[i].token_type == SEMICOLON && nested == 0)
          return i;
	if (count_token[i].token_type == FI || count_token[i].token_type == DONE ||
	    count_token[i].token_type == RPAREN) {
	  if (count_token[i-1].token_type != IDENTIFIER  && nested != 0)
	    nested--;
	}
      }
    
  }
  return 0;
}

int sequence_checker(struct token* sequence_token, int size_sequence) {
  enum token_types first = sequence_token[0].token_type;
  int size = size_sequence;
  int end_statement = end_of_statement(sequence_token, size);
  if (end_statement == (size - 1))     //there is no sequence                                                           
    return 0;
  if (end_statement == (size - 2) && sequence_token[size - 1].token_type == SEMICOLON)
    return 0;   //there is no sequence, just semicolon deliminating the end
  if (first == LPAREN && end_statement != (size-1))
    return end_statement;   //returns position of valid sequence
  if (first == IDENTIFIER && end_statement != (size-1))
    return end_statement;
  if (sequence_token[end_statement+1].token_type == SEMICOLON)
    return (end_statement+1);
  return 0;   //no sequence to be handled                                                                                          
}

void input_checker(struct token* input_token, int size_input, command_t current_command) {
  int size = size_input;
  int i;
  int k;
  int input_pos = -1;
  int output_pos = -1;


  if (input_token[size-1].token_type == LESSTHAN)
    error(1, 0, "Invalid '<' position");
  if (input_token[size-1].token_type == GREATERTHAN)
  error(1, 0, "Invalid '>' position"); 
  /* if (size < 1)
    error(1, 0, "Invalid input at end of statement"); 
  */
  for (k = size - 1; k != -1; k--)
    {  
      if (input_token[k].token_type == GREATERTHAN)
        {
	  if (k != 0) {
          if (input_token[k-1].token_type == LESSTHAN ||
             input_token[k-1].token_type == GREATERTHAN ||
             input_token[k-1].token_type == LPAREN ||
             input_token[k-1].token_type == RPAREN ||
             input_token[k-1].token_type == SEMICOLON ||
             input_token[k-1].token_type == PIPE)
            error(1, 0, "Invalid output for redirection.");
	  }
	  output_pos = k;
	  break;
        }
    }

  for (i = size - 1; i != -1; i--) {
    if (input_token[i].token_type == LESSTHAN && i != 0)
      {
	if (i != 0) {
        if (input_token[i-1].token_type == LESSTHAN ||
            input_token[i-1].token_type == GREATERTHAN ||
            input_token[i-1].token_type == LPAREN ||
            input_token[i-1].token_type == RPAREN ||
            input_token[i-1].token_type == SEMICOLON ||
            input_token[i-1].token_type == PIPE)
          error(1, 0, "Invalid redirect");
	}
	input_pos = i;
	break;
      }
  }
  
    if (input_pos == -1 && output_pos == -1)
      {
	current_command->input = NULL;
	current_command->output = NULL;
      }
    else if (input_pos == -1 && output_pos != -1)
      {
	current_command->input = NULL;
	current_command->output = input_token[output_pos + 1].value;
	/*printf("%s", current_command->output); */ 
      }
    else if (input_pos != -1 && output_pos == -1)
      {
	current_command->input = input_token[input_pos + 1].value;
	/*printf("%s", current_command->input);  */
	current_command->output = NULL;
      }
    else if (input_pos > output_pos)
      {
	current_command->input = input_token[input_pos + 1].value;
	current_command->output = input_token[output_pos + 1].value;
      }
    else if (output_pos > input_pos)
      {
	current_command->input = input_token[input_pos + 1].value;
	/*printf("%s", current_command->input);  */
	current_command->output = input_token[output_pos + 1].value;
	/*printf("%s", current_command->output);  */
      }

}


command_t parser(struct token* token_data, int size_of_token_array) {

  int size = size_of_token_array;
  if (size == 0)
    error(1, 0, "Invalid array size");

#ifdef VLAD_DEBUG
  int i = 0;
  for (; i < size_of_token_array; i++)
  {
    printf ("%s ", token_data[i].value);
  }
  printf("\n");
#endif


  //Because input does not take in entire size of the array
  //printf("invalid token is %s\n",token_data[size-1].value);
  if (token_data[size-1].token_type == LESSTHAN ||
      token_data[size-1].token_type == GREATERTHAN)
    error(1, 0, "Invalid redirect at end of statement");

  int sequence_position = sequence_checker(token_data, size_of_token_array);
  int pipe_position = pipe_checker(token_data, size_of_token_array); 
 
  enum token_types first = token_data[0].token_type;

  switch(first) { 
  case IF:
    if (sequence_position != 0)
      return sequence_parser(token_data, size_of_token_array, sequence_position);
    if (pipe_position != 0)
      return pipe_parser(token_data, size_of_token_array, pipe_position);
    return if_command_parser(token_data, size_of_token_array);
   
  case WHILE:  //WHILE_COMMAND
    if (sequence_position != 0)
      return sequence_parser(token_data, size_of_token_array, sequence_position);
    if (pipe_position != 0)
      return pipe_parser(token_data, size_of_token_array, pipe_position);
    return woru_parser(token_data, size_of_token_array);

  case UNTIL:  //UNTIL_COMMAND
    if (sequence_position != 0)
      return sequence_parser(token_data, size_of_token_array, sequence_position);
    if (pipe_position != 0)
      return pipe_parser(token_data, size_of_token_array, pipe_position);
    return woru_parser(token_data, size_of_token_array);

  case LPAREN:
    if (sequence_position != 0)
      return sequence_parser(token_data, size_of_token_array, sequence_position);
    if (pipe_position != 0)
      return pipe_parser(token_data, size_of_token_array, pipe_position);
    return subshell_parser(token_data, size_of_token_array);

  case IDENTIFIER: //SIMPLE COMMAND
    if (sequence_position != 0)
      return sequence_parser(token_data, size_of_token_array, sequence_position);
    if (pipe_position != 0)
      return pipe_parser(token_data, size_of_token_array, pipe_position);
    return simple_command_parser(token_data, size_of_token_array); 
    
  default:  //erroneous innput
    error(1, 0, "Invalid input at beginning of command");
  }   //end of switch

  return 0;   //should never reach here
  }

