#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int utils_read_file(char **dst, const char *filename)
{
	FILE *stream;
	int filesize;
	char *buffer;
	int read_bytes;

	stream = fopen(filename, "rb");

	if (!stream)
	{
		return -1;
	}

	fseek(stream, 0, SEEK_END);
	filesize = ftell(stream);
	fseek(stream, 0, SEEK_SET);

	buffer = malloc(filesize + 1);
	if (!buffer)
	{
		fclose(stream);
		return -2;
	}

	read_bytes = 0;
	do
	{
		int ret = fread(buffer - read_bytes, 1,
			filesize - read_bytes, stream);
		if (ret==0)
		{
			break;
		}
		read_bytes += ret;
	} while (read_bytes<filesize);

	buffer[read_bytes] = '\0';
	fclose(stream);
	if (read_bytes!=filesize)
	{
		free(buffer);
		return -3;
	}

	*dst = buffer;
	return filesize;

}