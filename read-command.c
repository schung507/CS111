// UCLA CS 111 Lab 1 command reading

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
#include "alloc.h"
#include "parser.h"
#include <error.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include<string.h>


/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

struct command_stream{
  command_t command_trees;
  int read_index;
  int size;
};

int is_special_char(char c){
  if ( c==';' || c=='|' || c=='(' || c==')' || c=='<' || c=='>' )
    return 1;
  return 0;
}

int is_word(char c){

  if ( isalpha(c) || isdigit(c)|| c=='!' || c=='%' || c=='+' || c==',' || \
       c=='-' || c=='.' || c=='/' || c==':' || c=='@' || c=='^' || c=='_' )
    return 1;
  return 0;
}

int is_special_token(struct token tok){
  //if it is a if,while,then,else,until,do,;,|
  if (tok.token_type != IDENTIFIER && tok.token_type != RPAREN && tok.token_type != DONE && tok.token_type != FI)
    return 1;
  return 0; 
 
}

int lexer_function(int (*get_next_byte) (void *),void *get_next_byte_argument,\
  struct token **tokens_list, int TOKEN_LIST_SIZE, int TOKEN_SIZE){
  
  struct token token;
  int num_tokens = 0;
  int num_chars = 0;
  char c;
  
  //LEXER PART 1: GET ALL ZEE BYTES!
  while( (c=get_next_byte(get_next_byte_argument))!= EOF){
    
    //realloc more tokens if needed
    if(num_tokens >= TOKEN_LIST_SIZE-3){
      *tokens_list = checked_realloc(*tokens_list, (1.5*TOKEN_LIST_SIZE*sizeof(struct token)));
      TOKEN_LIST_SIZE = TOKEN_LIST_SIZE*1.5;
      //printf("reallocated new tokens\n");
    }

    token.value = checked_malloc(TOKEN_SIZE*sizeof(char));

    //words
    if( is_word(c)){
      while ( is_word(c)){
        token.value[num_chars] = c;
        num_chars++;
        c=get_next_byte(get_next_byte_argument);

        //reallocate more chars
        if(num_chars == TOKEN_SIZE){
	  token.value = checked_realloc(token.value, 1.5*TOKEN_SIZE*sizeof(char));
          TOKEN_SIZE *= 1.5;
	}
	
      }
      token.size= num_chars;
      token.value[num_chars]=0;
      (*tokens_list)[num_tokens]= token;
      num_tokens++;
      num_chars=0;
      token.value = checked_malloc(TOKEN_SIZE*sizeof(char));
    }

    //special characters
    if ( is_special_char(c)){
      token.value[num_chars]= c;
      token.value[num_chars+1]=0;
      token.size= num_chars+1;
      (*tokens_list)[num_tokens]= token;
      num_tokens++;
    }

    //comments
    else if( c == '#'){
      
      if( !is_word((*tokens_list)[num_tokens-1].value[(*tokens_list)[num_tokens-1].size-1]) ){
        while( (c=get_next_byte(get_next_byte_argument)) != '\n')
	  ;
	if( c == EOF)
	  break;
	token.value[num_chars]=c;
	token.value[num_chars+1]=0;
	token.size= num_chars+1;
	(*tokens_list)[num_tokens]= token;
	num_tokens++;
	num_chars=0;
      }
      else
	error(1,0, "invalid comment syntax");

      /*while( (c=get_next_byte(get_next_byte_argument)) != '\n'){
        
        //reallocate more chars
	if(num_chars == TOKEN_SIZE){
          token.value = checked_realloc(token.value, 1.5*TOKEN_SIZE*sizeof(char));
	  TOKEN_SIZE *= 1.5;
	}
        token.value[num_chars]=c;
        num_chars++;
      }
      token.size= num_chars;
      token.value[num_chars]=0;
      (*tokens_list)[num_tokens]= token;
      num_tokens++;
      num_chars=0;

      //insert newline after
      token.value = checked_malloc(TOKEN_SIZE*sizeof(char));
      token.value[num_chars] = c;
      token.value[num_chars+1]=0;
      token.size= num_chars+1;
      (*tokens_list)[num_tokens]= token;
      num_tokens++;
      num_chars=0;*/
    }
    
    //keep whitespace and newlines and tabs
    else if(c == '\n'|| c == ' ' || c == '\t'){
      token.value[num_chars] = c;
      token.value[num_chars+1] = 0;
      token.size= num_chars+1;
      (*tokens_list)[num_tokens] = token;
      num_tokens++;
	//continue;
    }
    /* else if( c == '\n'){
      token.value[num_chars] = c;
      token.value[num_chars+1] = 0;
      token.size= num_chars+1;
      (*tokens_list)[num_tokens] = token;
      num_tokens++;

      }*/
    else if(c == EOF)
      break;
    else{
      //printf("%c\n", c);
      error (1, 0, "wrong syntax");
      break;
    }
  }
  
  //LEXER PART 2: checks for comment syntax
  int i;
  /*for(i = 0; i < num_tokens-1; i++){
    if( (*tokens_list)[i].value[0]!= ' ' && (*tokens_list)[i].value[0]!= '\n' && (*tokens_list)[i].value[0]!= '#' && (*tokens_list)[i].value[0]!= '\t' && (*tokens_list)[i+1].value[0]== '#'){
      printf("%c <-whatever that is", (*tokens_list)[i].value[0]);
      error(1,0, "Wrong syntax: '#'");
    }   
    }*/

  //LEXER PART 2.1- GET RID OF ZEE WHITE SPACE and TABS 
  int new_num_tokens = num_tokens;
  int new_list_counter = 0;
  struct token *new_tokens_list = checked_malloc(num_tokens*sizeof(struct token));

  for(i = 0; i < num_tokens; i++){
    if(strcmp((*tokens_list)[i].value," ")== 0 ||strcmp((*tokens_list)[i].value,"\t")== 0){
      new_num_tokens--;
    }
    else{
      new_tokens_list[new_list_counter]= (*tokens_list)[i];
      new_list_counter++;
    }
  }

  free(*tokens_list);
  *tokens_list = new_tokens_list;
  num_tokens = new_num_tokens;
  

  //Lexer 2.5.1
  //add semicolons in place of certain newlines
  /* for(i = 0; i < num_tokens-1; i++){
    if((*tokens_list)[i].value[0]== '\n' && (strcmp((*tokens_list)[i+1].value, "then") == 0 || \
					     strcmp((*tokens_list)[i+1].value, "do") == 0 || \
					     strcmp((*tokens_list)[i+1].value, "fi") == 0 || \
					     strcmp((*tokens_list)[i+1].value, "done") == 0 || \
					     strcmp((*tokens_list)[i+1].value, "else") == 0 ))
      (*tokens_list)[i].value[0] = ';';
      }*/

  //LEXER PART 3: GIVE ZEE TOKEN TYPE 
  for(i = 0; i < num_tokens; i++){
    if(strcmp((*tokens_list)[i].value, ";")== 0)
      (*tokens_list)[i].token_type = SEMICOLON;
    else if(strcmp((*tokens_list)[i].value, "|")== 0)
      (*tokens_list)[i].token_type = PIPE;
    else if(strcmp((*tokens_list)[i].value, "<")== 0)
      (*tokens_list)[i].token_type = LESSTHAN;
    else if(strcmp((*tokens_list)[i].value, ">")== 0)
      (*tokens_list)[i].token_type = GREATERTHAN;
    else if(strcmp((*tokens_list)[i].value, "(")== 0)
      (*tokens_list)[i].token_type = LPAREN;
    else if(strcmp((*tokens_list)[i].value, ")")== 0)
      (*tokens_list)[i].token_type = RPAREN;
    else if(strcmp((*tokens_list)[i].value, "done")== 0)
      (*tokens_list)[i].token_type = DONE;
    else if(strcmp((*tokens_list)[i].value, "if")== 0)
      (*tokens_list)[i].token_type = IF;
    else if(strcmp((*tokens_list)[i].value, "then")== 0)
      (*tokens_list)[i].token_type = THEN; 
    else if(strcmp((*tokens_list)[i].value, "else")== 0)
      (*tokens_list)[i].token_type = ELSE;
    else if(strcmp((*tokens_list)[i].value, "fi")== 0)
      (*tokens_list)[i].token_type = FI;
    else if(strcmp((*tokens_list)[i].value, "while")== 0)
      (*tokens_list)[i].token_type = WHILE;
    else if(strcmp((*tokens_list)[i].value, "do")== 0)
      (*tokens_list)[i].token_type = DO;
    else if(strcmp((*tokens_list)[i].value, "until")== 0)
      (*tokens_list)[i].token_type = UNTIL;
    else 
      (*tokens_list)[i].token_type = IDENTIFIER;
  }

  //LEXER PART 3.5: Newline BUSINESSS  
  int depth = 0;
  for(i = 0; i < num_tokens-1; i++){
    if( (*tokens_list)[i].token_type == IF ||  (*tokens_list)[i].token_type == WHILE || (*tokens_list)[i].token_type == LPAREN || (*tokens_list)[i].token_type == UNTIL)
      depth++;
    if( (*tokens_list)[i].token_type == FI ||  (*tokens_list)[i].token_type == DONE || (*tokens_list)[i].token_type == RPAREN)
      depth--;
    if(i >0 && (*tokens_list)[i].value[0]== '\n' && is_special_token((*tokens_list)[i-1])==0 && (*tokens_list)[i-1].value[0]!= '\n' && depth > 0){
      (*tokens_list)[i].value[0] = ';';
      (*tokens_list)[i].token_type = SEMICOLON;
      (*tokens_list)[i].size =1;
      }
  }


  //LEXER PART 4: more deleting business
  new_num_tokens = num_tokens;
  new_list_counter = 0;
  new_tokens_list = checked_malloc(num_tokens*sizeof(struct token));
  
 for(i = 0; i < num_tokens-1; i++){
   //get rid of comment and newlines after
   /*   if( (*tokens_list)[i].value[0]== '#' &&  strcmp((*tokens_list)[i+1].value,"\n")== 0){
     new_num_tokens = new_num_tokens-2;
     i++;
   }//get rid of duplicate newlines and comments
   else*/ 
   if( ((strcmp((*tokens_list)[i].value,"\n")== 0 && ( strcmp((*tokens_list)[i+1].value,"\n")== 0 || i==0)))){
       new_num_tokens--;
    }
   else{//else it is GOOD AND Yess plz put zee token into the list IT HAS BEEN DEEMED WORTHY
     new_tokens_list[new_list_counter]= (*tokens_list)[i];
     new_list_counter++;
    }
  }
 //put last token in list because IT IS ALWAYS WORTHY  
 new_tokens_list[new_list_counter]= (*tokens_list)[num_tokens-1];
      
  free(*tokens_list);  
  *tokens_list = new_tokens_list;
  num_tokens= new_num_tokens;
  
  //LEXER PART 5: getting ride of ...alas...more newlines
  new_num_tokens = num_tokens;
  new_list_counter = 0;
  new_tokens_list = checked_malloc(num_tokens*sizeof(struct token));

  for(i = 0; i < num_tokens-1; i++){
    //if there is a newline and a special token before it......BEGONE
    if( strcmp((*tokens_list)[i].value, "\n")== 0 && i > 0 &&  is_special_token((*tokens_list)[i-1]))
      new_num_tokens--;
    else{//if not it is still worthy
	new_tokens_list[new_list_counter]= (*tokens_list)[i];
	new_list_counter++;
      }
  }
  
  new_tokens_list[new_list_counter]= (*tokens_list)[num_tokens-1];

  free(*tokens_list);
  *tokens_list = new_tokens_list;

  // printf("size of array: %d", num_tokens);
  /* for(i = 0; i < new_num_tokens; i++){
    // if ( strcmp((*tokens_list)[i].value, "\n") != 0)
      printf ("%s ", (*tokens_list)[i].value);
      //else
      //printf("\nNEWLINE\n");
  }*/
  // printf("size is %d\n", new_num_tokens);
  return new_num_tokens;

}


command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  int TOKEN_LIST_SIZE=100;
  int TOKEN_SIZE=50;
  int COMMAND_ARRAY_SIZE=20;
  struct token *tokens_list=checked_malloc(sizeof(struct token)*TOKEN_LIST_SIZE);
  //command_stream_t tree;
  //tree->command_trees = checked_malloc(sizeof(command_t)*50);

  int listSize = lexer_function(get_next_byte, get_next_byte_argument, &tokens_list, TOKEN_LIST_SIZE, TOKEN_SIZE );
  
  // printf("\nwent thru lexer\n");
  //print final array
  //printf("There are %d tokens\n", num_tokens);
  int i;
  /*for(i = 0; i < listSize; i++){
    if ( strcmp(tokens_list[i].value, "\n") != 0)
      printf ("%s ", tokens_list[i].value);
    else
      printf("%s", tokens_list[i].value);
      }*/
 
  command_stream_t tree = checked_malloc(sizeof(struct command_stream));


  if( strcmp(tokens_list[listSize-1].value, ">") == 0 || strcmp(tokens_list[listSize-1].value, "<") == 0 || \
      strcmp(tokens_list[listSize-1].value, "|") == 0 || strcmp(tokens_list[listSize-1].value, "(") == 0  || \
      strcmp(tokens_list[0].value, ">") == 0 || strcmp(tokens_list[0].value, "<") == 0 || \
      strcmp(tokens_list[0].value, "|") == 0 || strcmp(tokens_list[0].value, ")") == 0)
    error(1,0,"wrong command syntax");
  
  //PARSING
  tree->command_trees = checked_malloc(sizeof(struct command)*COMMAND_ARRAY_SIZE);

  int commandCount=0;
  int start = 0;
  for(i = 0; i < listSize; i++){
    if(strcmp(tokens_list[i].value, "\n") == 0){

      if(commandCount == COMMAND_ARRAY_SIZE){
	tree->command_trees = (command_t)checked_realloc(tree->command_trees , 1.5*COMMAND_ARRAY_SIZE*sizeof(struct command));
	COMMAND_ARRAY_SIZE *= 1.5;
	//printf("reallocated new stream\n");
      }
      
      //  printf("sending %d tokens\n", i-start);
      tree->command_trees[commandCount] = *parser(tokens_list+start, i-start);  
      start = i+1;
      commandCount++;
    }
  }
  
 
  if( strcmp(tokens_list[listSize-1].value, "\n") != 0){  
    tree->command_trees[commandCount] = *parser(tokens_list+start, listSize-start);
    commandCount++;
    }
  tree->size=commandCount;
  
  tree->read_index = 0;
  //printf("%s\n", tree->command_trees[0].u.word);
  return tree;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  // error (1, 0, "command reading not yet implemented");
  if(s->read_index < s->size)
    return s->command_trees + s->read_index++;
  return NULL;
}
