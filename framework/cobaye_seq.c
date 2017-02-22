#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cobaye_framework.h"

int cobaye_run_seq(struct cobaye_entry *entry)
{
	int i;

	for (i = 1; i < entry->value + 1; i++) {
		cobaye_conf[item_repeat].value = entry[i].value;
		if (entry[i].test)
			cobaye_run_tst(&entry[i]);
	}

	return 0;
}

static FILE *cobaye_open_seq(char *name, struct cobaye_entry **seqptr)
{
	int seq_count = 0;
	char seq_name[NAME_LEN];
	FILE *seq = fopen(name, "r");
	struct cobaye_entry *sequence = NULL;

	if (seq) {
		char data[NAME_LEN];

		fgets(seq_name, NAME_LEN, seq);
		seq_name[strlen(seq_name) - 1] = '\0';
		fscanf(seq, "%d", &seq_count);

		sequence = calloc(1 + seq_count, sizeof(struct cobaye_entry));

		sequence[0].type = cobaye_int;
		strcpy(sequence[0].name, seq_name);
		sequence[0].value = seq_count;
	}

	*seqptr = sequence;
	return seq;
}

static int cobaye_seq_add_entry(struct cobaye_entry *sequence, char *data, int index)
{
	char *cobaye_name;
	int cobaye_iter = 0;
	struct cobaye_test *newtest = NULL;

	cobaye_iter = strtol(data, &cobaye_name, 0);

	if (cobaye_name[0] == ':') {
		struct cobaye_entry *entry;
		cobaye_name++;
		entry = cobaye_test_exist(cobaye_name);
		if (entry) {
			strcpy(sequence[index].name, entry->test->name);
			sequence[index].test = entry->test;
			sequence[index].value = cobaye_iter;
			sequence[index].seq = sequence;
			index++;
		}
	}
	return index;
}

void cobaye_destroy_seq(struct cobaye_entry *seq)
{
	free(seq);
}

struct cobaye_entry *cobaye_build_seq(char *name)
{
	int i = 1;
	struct cobaye_entry *sequence = NULL;
	FILE *seq = cobaye_open_seq(name, &sequence);
	
	if (sequence) {
		char data[NAME_LEN+8] = {0};

		while (i <= sequence[0].value && !feof(seq)) {
			fgets(data, sizeof(data) - 1, seq);
			data[strlen(data) - 1] = '\0';

			i = cobaye_seq_add_entry(sequence, data, i);
		}
		fclose(seq);
	}

	return sequence;
}
