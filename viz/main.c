/**
 * This file is part of groufix.
 * Copyright (c) Stef Velzel. All rights reserved.
 *
 * groufix : graphics engine produced by Stef Velzel.
 * www     : <www.vuzzel.nl>
 */

#include <groufix.h>
#include <stdlib.h>
#include <groufix/gilia/gilia.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>


/****************************
 * grouviz entry point.
 */
int main(int argc, char **argv)
{
	if (!gfx_has_gilia()) {
		fprintf(stderr, "Missing gilia support!\n");
		exit(EXIT_FAILURE);
	}

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <gilia file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	FILE *f = fopen(argv[1], "r");
	if (!f) {
		fprintf(stderr, "Failed to open '%s': %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (!gfx_gilia_execute_file(f)) {
		fprintf(stderr, "Failed to execute!\n");
		exit(EXIT_FAILURE);
	}

	fprintf(stderr, "Executed Gilia script.\n");
	exit(EXIT_SUCCESS);
}
