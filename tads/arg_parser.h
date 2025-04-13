#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#define MAX_PLAYERS 9

void parse_arguments(int argc, char *argv[], int *width, int *height,
					 int *delay, int *timeout, unsigned int *seed,
					 char **view_path, char *player_paths[], int *num_players);

void print_parameters(int width, int height, int delay, int timeout,
					  unsigned int seed, const char *view_path,
					  const char *player_paths[], int num_players);

#endif // ARG_PARSER_H