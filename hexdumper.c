#include "args.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


char* helptext = "Usage: hexdump [FILE] [OPTIONS]\n"
                 "\n"
                 "Arguments:\n"
                 "  [FILE]                 File to read (default: STDIN)\n"
                 "\n"
                 "Options:\n"
                 "  -l, --line <int>       Bytes per line in output (default: 16)\n"
                 "  -b, --bytes <int>      Number of bytes to read (default: all)\n"
                 "  -o, --offset <int>     Byte offset at which to begin reading\n"
                 "  -f, --file <filename>  Output file to save the hexdump\n"
                 "\n"
                 "Flags:\n"
                 "  -h, --help             Display this help text and exit\n"
                 "  -v, --version          Display the version number and exit\n";


void print_line(FILE* output, uint8_t* buffer, int num_bytes, int offset, int line_length) {
    fprintf(output, "%6X ", offset);

    for (int i = 0; i < line_length; i++) {
        if (i > 0 && i % 8 == 0) {
            fprintf(output, " ");
        }
        if (i < num_bytes) {
            fprintf(output, " %02X", buffer[i]);
        } else {
            fprintf(output, "   ");
        }
    }

    fprintf(output, "   ");

    for (int i = 0; i < num_bytes; i++) {
        if (buffer[i] > 31 && buffer[i] < 127) {
            fprintf(output, "%c", buffer[i]);
        } else {
            fprintf(output, ".");
        }
    }

    fprintf(output, "\n");
}


void dump_file(FILE* input, FILE* output, int offset, int bytes_to_read, int line_length) {
    uint8_t* buffer = (uint8_t*)malloc(line_length);
    if (buffer == NULL) {
        fprintf(stderr, "Error: insufficient memory.\n");
        exit(1);
    }

    while (true) {
        int max_bytes;

        if (bytes_to_read < 0) {
            max_bytes = line_length;
        } else if (line_length < bytes_to_read) {
            max_bytes = line_length;
        } else {
            max_bytes = bytes_to_read;
        }

        int num_bytes = fread(buffer, sizeof(uint8_t), max_bytes, input);
        if (num_bytes > 0) {
            print_line(output, buffer, num_bytes, offset, line_length);
            offset += num_bytes;
            bytes_to_read -= num_bytes;
        } else {
            break;
        }
    }

    free(buffer);
}


int main(int argc, char** argv) {
    // Instantiate a new ArgParser instance.
    ArgParser* parser = ap_new();
    ap_helptext(parser, helptext);
    ap_version(parser, "0.1.0");

    // Register our options with their default values.
    ap_int_opt(parser, "line l", 16);
    ap_int_opt(parser, "bytes b", -1);
    ap_int_opt(parser, "offset o", 0);
    ap_string_opt(parser, "file f", NULL);

    // Parse the command line arguments.
    ap_parse(parser, argc, argv);

    // Default to reading from stdin.
    FILE* input = stdin;
    if (ap_has_args(parser)) {
        char* filename = ap_arg(parser, 0);
        input = fopen(filename, "rb");
        if (input == NULL) {
            fprintf(stderr, "Error: cannot open the file '%s'.\n", filename);
            exit(1);
        }
    }

    // Try seeking to the specified offset.
    int offset = ap_int_value(parser, "offset");
    
    if (offset < 0) {
        // Get the file size.
        fseek(input, 0, SEEK_END);
        long file_size = ftell(input);
        if (file_size == -1) {
            fprintf(stderr, "Error: cannot determine file size.\n");
            exit(1);
        }

        // Adjust the offset to a positive value.
        offset = file_size + offset;
    }
	
    if (offset != 0) {
        if (fseek(input, offset, SEEK_SET) != 0) {
            fprintf(stderr, "Error: cannot seek to the specified offset.\n");
            exit(1);
        }
    }

    int bytes_to_read = ap_int_value(parser, "bytes");
    int line_length = ap_int_value(parser, "line");

    // Determine the output file.
    FILE* output = stdout;
    char* output_filename = ap_string_value(parser, "file");
    if (output_filename != NULL) {
        output = fopen(output_filename, "w");
        if (output == NULL) {
            fprintf(stderr, "Error: cannot open the file '%s' for writing.\n", output_filename);
            fclose(input);
            exit(1);
        }
    }

    dump_file(input, output, offset, bytes_to_read, line_length);

    fclose(input);
    fclose(output);
    ap_free(parser);
}
