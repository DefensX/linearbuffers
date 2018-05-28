
#include <stdio.h>
#include <stdlib.h>

#include <getopt.h>

#include "queue.h"
#include "schema.h"

#include "schema.lex.h"

#define OPTION_HELP		'h'
#define OPTION_SCHEMA		's'
#define OPTION_ENCODER		'e'
#define OPTION_DECODER		'd'

#define DEFAULT_SCHEMA		NULL
#define DEFAULT_ENCODER		NULL
#define DEFAULT_DECODER		NULL

int yyparse (void);

static struct option options[] = {
	{ "help"	, no_argument	   , 0, OPTION_HELP	},
	{ "schema"	, required_argument, 0, OPTION_SCHEMA	},
	{ "encoder"	, required_argument, 0, OPTION_ENCODER	},
	{ "decoder"	, required_argument, 0, OPTION_DECODER	},
	{ 0		, 0                , 0, 0		}
};

static void print_help (const char *name)
{
	fprintf(stdout, "%s:\n", name);
	fprintf(stdout, "\n");
	fprintf(stdout, "options:\n");
	fprintf(stdout, "  -s, --schema : schema file (default: %s)\n", (DEFAULT_SCHEMA == NULL) ? "(null)" : DEFAULT_SCHEMA);
	fprintf(stdout, "  -e, --encoder: generated encoder file (default: %s)\n", (DEFAULT_ENCODER == NULL) ? "(null)" : DEFAULT_ENCODER);
	fprintf(stdout, "  -d, --decoder: generated decoder file (default: %s)\n", (DEFAULT_DECODER == NULL) ? "(null)" : DEFAULT_DECODER);
	fprintf(stdout, "  -h, --help   : this text\n");
}

static char * read_file (const char *filename, size_t max_size, size_t *size_out)
{
	FILE *fp;
	size_t size, pos, n, _out;
	char *buf;

	size = 0;
	buf = NULL;
	size_out = size_out ? size_out : &_out;

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "can not open file: %s\n", filename);
		goto bail;
	}

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	*size_out = size;
	if (max_size > 0 && size > max_size) {
		fprintf(stderr, "size is too big\n");
		goto bail;
	}
	rewind(fp);

	buf = malloc(size ? size : 1);
	if (buf == NULL) {
		fprintf(stderr, "can not allocate memory\n");
		goto bail;
	}

	pos = 0;
	while ((n = fread(buf + pos, 1, size - pos, fp))) {
		pos += n;
	}
	if (pos != size) {
		goto bail;
	}
	fclose(fp);
	*size_out = size;
	return buf;

bail:	if (fp) {
		fclose(fp);
	}
	if (buf) {
		free(buf);
	}
	*size_out = size;
	return 0;
}

static int schema_parse_file (const char *filename)
{
	char *buffer;
	size_t buffer_length;

	buffer = NULL;

	buffer = read_file(filename, -1, &buffer_length);
	if (buffer == NULL) {
		fprintf(stderr, "can not read file: %s\n", filename);
		goto bail;
	}

	YY_BUFFER_STATE bs = yy_scan_bytes(buffer, buffer_length);
	yy_switch_to_buffer(bs);
	yyparse();
	yy_delete_buffer(bs);

	free(buffer);
	return 0;
bail:	if (buffer != NULL) {
		free(buffer);
	}
	return -1;
}

int main (int argc, char *argv[])
{
	int c;
	int option_index;

	int rc;
	const char *option_schema;
	const char *option_encoder;
	const char *option_decoder;

	option_schema  = DEFAULT_SCHEMA;
	option_encoder = DEFAULT_ENCODER;
	option_decoder = DEFAULT_DECODER;

	while (1) {
		c = getopt_long(argc, argv, "s:e:d:h", options, &option_index);
		if (c == -1) {
			break;
		}
		switch (c) {
			case OPTION_HELP:
				print_help(argv[0]);
				goto out;
			case OPTION_SCHEMA:
				option_schema = optarg;
				break;
			case OPTION_ENCODER:
				option_encoder = optarg;
				break;
			case OPTION_DECODER:
				option_decoder = optarg;
				break;
		}
	}

	if (option_schema == NULL) {
		fprintf(stderr, "schema file is invalid\n");
		goto bail;
	}
	if (option_encoder == NULL &&
	    option_decoder == NULL) {
		fprintf(stderr, "nothing to generate\n");
		goto bail;
	}

	rc = schema_parse_file(option_schema);
	if (rc != 0) {
		fprintf(stderr, "can not read schema file: %s\n", option_schema);
		goto bail;
	}

out:	return 0;
bail:	return -1;
}
