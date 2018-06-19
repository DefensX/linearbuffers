
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "05-encoder.h"
#include "05-decoder.h"
#include "05-jsonify.h"

int main (int argc, char *argv[])
{
	int rc;

	size_t i;
	uint8_t data[10];

	uint64_t linearized_length;
	const char *linearized_buffer;

	struct linearbuffers_encoder *encoder;
	struct linearbuffers_decoder decoder;

	(void) argc;
	(void) argv;

	srand(time(NULL));
	for (i = 0; i < sizeof(data) / sizeof(data[0]); i++) {
		data[i] = rand();
	}

	encoder = linearbuffers_encoder_create(NULL);
	if (encoder == NULL) {
		fprintf(stderr, "can not create linearbuffers encoder\n");
		goto bail;
	}

	rc  = linearbuffers_output_start(encoder);
	rc |= linearbuffers_output_type_set(encoder, linearbuffers_type_type_3);
	rc |= linearbuffers_output_length_set(encoder, 1);
	rc |= linearbuffers_output_timeval_start(encoder);
	rc |= linearbuffers_output_timeval_seconds_set(encoder, 2);
	rc |= linearbuffers_output_timeval_useconds_set(encoder, 3);
	rc |= linearbuffers_output_timeval_end(encoder);
	rc |= linearbuffers_output_data_set(encoder, data, sizeof(data) / sizeof(data[0]));
	rc |= linearbuffers_output_end(encoder);
	if (rc != 0) {
		fprintf(stderr, "can not encode output\n");
		goto bail;
	}

	linearized_buffer = linearbuffers_encoder_linearized(encoder, &linearized_length);
	if (linearized_buffer == NULL) {
		fprintf(stderr, "can not get linearized buffer\n");
		goto bail;
	}
	fprintf(stderr, "linearized: %p, length: %ld\n", linearized_buffer, linearized_length);

	linearbuffers_output_jsonify(linearized_buffer, linearized_length, printf);

	rc = linearbuffers_output_decode(&decoder, linearized_buffer, linearized_length);
	if (rc != 0) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (linearbuffers_output_type_get(&decoder) != linearbuffers_type_type_3) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (linearbuffers_output_length_get(&decoder) != 1) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (linearbuffers_output_timeval_seconds_get(&decoder) != 2) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (linearbuffers_output_data_get_count(&decoder) != sizeof(data) / sizeof(data[0])) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (linearbuffers_output_data_get_length(&decoder) != sizeof(data)) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	if (memcmp(linearbuffers_output_data_get(&decoder), data, sizeof(data))) {
		fprintf(stderr, "decoder failed\n");
		goto bail;
	}
	for (i = 0; i < linearbuffers_output_data_get_count(&decoder); i++) {
		if (data[i] != linearbuffers_output_data_get(&decoder)[i]) {
			fprintf(stderr, "decoder failed\n");
			goto bail;
		}
	}

	linearbuffers_encoder_destroy(encoder);

	return 0;
bail:	if (encoder != NULL) {
		linearbuffers_encoder_destroy(encoder);
	}
	return -1;
}
