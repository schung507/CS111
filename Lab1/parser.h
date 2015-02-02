#include "command.h"
#include "command-internals.h"

command_t simple_command_parser(struct token* simple_token, int size_simple);
command_t if_command_parser(struct token* if_token, int size_if);
command_t woru_parser(struct token* woru_token, int size_woru);
command_t subshell_parser(struct token* sub_token, int size_sub);
command_t pipe_parser(struct token* pipe_token, int size_pipe, int pipe_loc);
int pipe_checker(struct token* pipe_token, int size_pipe);
command_t sequence_parser(struct token* sequence_token, int size_sequence, int sequence_loc);
int end_of_statement(struct token* count_token, int size_count);
int sequence_checker(struct token* sequence_token, int size_sequence);
void input_checker(struct token* input_token, int size_input, command_t current_command);
command_t parser(struct token* token_data, int size_of_token_array);
