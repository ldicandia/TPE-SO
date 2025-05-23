// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "../headers/arg_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

void parse_arguments(int argc, char *argv[], int *width, int *height,
					 int *delay, int *timeout, unsigned int *seed,
					 char **view_path, char *player_paths[], int *num_players) {
	int opt;
	while ((opt = getopt(argc, argv, "w:h:d:t:s:v:p:")) != -1) {
		switch (opt) {
			case 'w':
				*width = atoi(optarg);
				if (*width < 10) {
					fprintf(stderr, "Error: minimum width must be 10.\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 'h':
				*height = atoi(optarg);
				if (*height < 10) {
					fprintf(stderr, "Error: minimum height must be 10.\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 'd':
				*delay = atoi(optarg);
				if (*delay < 0) {
					fprintf(stderr, "Error: delay must be non-negative.\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 't':
				*timeout = atoi(optarg);
				if (*timeout < 0) {
					fprintf(stderr, "Error: timeout must be non-negative.\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 's':
				*seed = atoi(optarg);
				break;
			case 'v':
				*view_path = optarg;
				break;
			case 'p':
				optind--;
				while (optind < argc && argv[optind][0] != '-') {
					if (*num_players >= MAX_PLAYERS) {
						fprintf(stderr,
								"Error: maximum number of players is %d.\n",
								MAX_PLAYERS);
						exit(EXIT_FAILURE);
					}
					if (argv[optind] == NULL || strlen(argv[optind]) == 0) {
						fprintf(stderr,
								"Error: invalid player path provided.\n");
						exit(EXIT_FAILURE);
					}
					player_paths[(*num_players)++] = argv[optind++];
				}
				break;
			default:
				fprintf(stderr,
						"Usage: %s [-w width] [-h height] [-d delay] "
						"[-t timeout] [-s seed] [-v view_path] "
						"[-p player_paths...]\n",
						argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	if (*num_players == 0) {
		fprintf(stderr, "Error: at least one player path must be provided.\n");
		exit(EXIT_FAILURE);
	}
}

void print_parameters(int width, int height, int delay, int timeout,
					  unsigned int seed, const char *view_path,
					  const char *player_paths[], int num_players) {
	printf("Parameters:\n");
	printf("Width: %d\n", width);
	printf("Height: %d\n", height);
	printf("Delay: %d ms\n", delay);
	printf("Timeout: %d s\n", timeout);
	printf("Seed: %u\n", seed);
	printf("View path: %s\n", view_path ? view_path : "None");
	printf("Player paths:\n");
	for (int i = 0; i < num_players; i++) {
		printf("  %s\n", player_paths[i]);
	}
}