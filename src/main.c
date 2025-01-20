#if __has_include("bits/posix1_lim.h")
#include <bits/posix1_lim.h>
#endif

#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include "definition.h"
#include "buffer.h"
#include "app_string.h"

typedef struct
{
	Buffer		buffer;
	//u8		*	buffer;
	//size_t		buffer_capacity;
	//size_t		buffer_used;

	String	*	header_values;
	size_t		header_values_capacity;

	String		method;
	String		argument;
	String		version;
} RequestData;

const size_t HTTP_METHOD_NAME_MAX_LENGTH = 7;
const char * standard_method_names[] = { "CONNECT", "DELETE", "GET", "HEAD", "OPTIONS", "PATCH", "POST", "PUT", "TRACE", "UPDATE" };
const char * standard_header_names[] = { "Accept", "Accept-Encoding", "Accept-Language", "Age", "Connection", "Content-Length", "DNT", "Host", "Priority", "Referer", "Sec-Fetch-Dest", "Sec-Fetch-Mode", "Sec-Fetch-Site", "Upgrade-Insecure-Requests", "User-Agent" };
const s8 standard_headers_count = ARRAY_COUNT(standard_header_names);

s32 socket_file_descriptor;

void print_error(const char * message)
{
	fprintf(stderr, "%s", message);
}

void print_lines(const u8 * string, size_t length, const char * line_header)
{
	if (string == NULL || length == 0) {
		return;
	}

	const u8	* end = string + length,
				* current;

	for (u32 length_to_print = 0; (current = string + length_to_print) <= end;) {
		length_to_print += 1;
		if (*current != '\n') {
			continue;
		}

		printf("%s%.*s", line_header, length_to_print, string);

		string += length_to_print;
		length_to_print = 0;
	}
}

ssize_t string_find_line_end(const u8 * string, size_t length)
{
	for (ssize_t i = 0; i < length; ++i) {
		u8 character = string[i];
		if (character == '\r' || character == '\n') {
			return i;
		}
	}

	return -1;
}

size_t string_ignore_space(const u8 ** string, size_t length)
{
	const u8	* start = *string,
	   			* end = *string + length;
	for (; start != end; ++start) {
		if (*start != ' ') {
			break;
		}
	}

	size_t ignored = start - *string;
	*string = start;
	return ignored;
}

ssize_t string_find_index_in_line(u8 * string, size_t length, u8 to_find)
{
	ssize_t index = 0;
	for (; index < length; ++index) {
		u8 character = string[index];
		if (character == to_find) {
			return index;
		}

		if (character == '\r' || character == '\n') {
			break;
		}
	}

	// Return the index of the closest line end character, but negated
	return -index;
}

ssize_t string_find_index(u8 * string, u8 to_find, size_t length)
{
	for (ssize_t index = 0; index < length; ++index) {
		if (string[index] == to_find) {
			return index;
		}
	}

	return -1;
}

size_t string_multi_find(const u8 * string, u8 * chars, ssize_t * indices, size_t length, size_t chars_count)
{
	memset(indices, -1, chars_count);

	size_t found = 0;
	for (ssize_t index = 0; index < length; ++index) {
		for (size_t chars_index = 0; chars_index < chars_count; ++chars_index) {
			if (string[index] == chars[chars_index] && indices[chars_index] < 0) {
				found += 1;
				indices[chars_index] = index;
			}
		}

		if (found == chars_count) {
			break;
		}
	}

	return found;
}

void signal_handler(int signal_number)
{
	printf("Process interrupted.\n");
	
	if (socket_file_descriptor != -1) {
		printf("Closing socket.\n");
		s32 result = shutdown(socket_file_descriptor, SHUT_RDWR);
		if (result != 0) {
			print_error("Failed to shutdown socket.\n");
			perror("shutdown");
		}
		result = close(socket_file_descriptor);
		if (result != 0) {
			print_error("Failed to close socket.\n");
			perror("close");
		}

		socket_file_descriptor = -1;
	}

	exit(0);
}

s8 index_by_header_name(const u8 * header_name, size_t length, const char * header_names[], s8 header_count)
{
	for (s8 i = 0; i < header_count; ++i) {
		if (strncmp(header_names[i], header_name, length) == 0) {
			return i;
		}
	}

	return -1;
}

String get_header_string_value(RequestData * data, const u8 * header_name, size_t header_name_length)
{
	String header_value = { 0 };

	s8 index = index_by_header_name(header_name, header_name_length, standard_header_names, standard_headers_count);
	if (index < 0) {
		return header_value;
	}

	header_value = data->header_values[index];
	return header_value;
}

s8 set_header_string_value(RequestData * data, const u8 * header_name, size_t header_name_length, u8 * header_value, size_t header_value_length)
{
	s8 index = index_by_header_name(header_name, header_name_length, standard_header_names, standard_headers_count);
	if (index < 0) {
		return -1;
	}

	String * item = &data->header_values[index];
	item->start = header_value;
	item->length = header_value_length;

	return 0;
}

int main(void)
{
	u8 _request_data_buffer[1024] = { 0 };
	String request_headers[50] = { 0 };
	RequestData request_data;
	request_data.buffer.start = _request_data_buffer;
	request_data.buffer.capacity = sizeof(_request_data_buffer);
	request_data.buffer.used = 0;
	request_data.header_values = request_headers;
	request_data.header_values_capacity = sizeof(request_headers) / sizeof(request_headers[0]);

	u8 * status_code_titles[501];
	status_code_titles[200] = "OK";
	status_code_titles[400] = "Bad Request";
	status_code_titles[404] = "Not Found";
	status_code_titles[500] = "Internal Server Error";

	u64 connection_number = 0;

	socket_file_descriptor = -1;
	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		print_error("An error occurred while setting a signal handler.\n");
		return 1;
    }

	struct in_addr ip_address = { .s_addr = htonl(INADDR_ANY) };
	sockaddr_in address_info = {
	   .sin_family = AF_INET,
	   .sin_port = htons(8080),
	   .sin_addr = ip_address
	};

	// Choose a protocol automatically using 0 as the third argument
	socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_file_descriptor == -1) {
		print_error("Couldn't create socket.\n");
		return -1;
	}

	if (bind(socket_file_descriptor, (sockaddr *) &address_info, sizeof(address_info)) == -1) {
		print_error("Couldn't bind socket.\n");
		perror("bind");
		return 1;
	}

	if (listen(socket_file_descriptor, SOMAXCONN) == -1) {
		print_error("Couldn't listen on socket.\n");
		return 1;
	}

	u8 input[16384] = { 0 };
	u8 output[16384] = { 0 };
	sockaddr_in client_address_info;
	socklen_t client_address_info_len = sizeof(client_address_info);
	while (true) {
		printf("Waiting for incoming connections.\n");
		s32 client_socket_file_descriptor = accept(socket_file_descriptor, (sockaddr *) &client_address_info, &client_address_info_len);
		if (client_socket_file_descriptor == -1) {
			print_error("Couldn't accept incoming connection.\n");
			continue;
		}

		printf("Incoming connection #%llu accepted.\n", connection_number);

		while (true) {
			ssize_t input_length = read(client_socket_file_descriptor, input, sizeof(input));
			printf("Read %ld bytes from client.\n", input_length);

			if (input_length < 0) {
				print_error("Couldn't read data from client.\n");
				break;
			}


			String	* method = &request_data.method,
					* argument = &request_data.argument,
					* version = &request_data.version;
			
			u8 * request_data_buffer = request_data.buffer.start;
			buffer_write(&request_data.buffer, input, 1024);
			ssize_t line_end_index = string_find_index(request_data_buffer, '\n', request_data.buffer.used);

			ssize_t index = string_find_index(request_data_buffer, ' ', HTTP_METHOD_NAME_MAX_LENGTH + 1);
			if (index < 0) {
				// Bad request (?)
				break;
			}
			*method = string_create(request_data_buffer, index);

			index = string_find_index(request_data_buffer + method->length + 1, ' ', line_end_index);
			if (index < 0) {
				// Bad request (?)
				break;
			}
			*argument = string_create(request_data_buffer + method->length + 1, index);

			index = string_find_index(argument->start + argument->length + 1, '\n', line_end_index);
			if (index < 0) {
				// Bad request (?)
				break;
			}
			*version = string_create(argument->start + argument->length + 1, index);

			printf("Verb: ");
			fwrite(method->start, SIZE_U8, method->length, stdout);
			printf("\nArgument: ");
			fwrite(argument->start, SIZE_U8, argument->length, stdout);
			printf("\nVersion: ");
			fwrite(version->start, SIZE_U8, version->length, stdout);
			printf("\n");
			
			//print_lines(input, input_length, "CLIENT > ");
			const u8	* input_string = input,
						* end = input + input_length;
			size_t input_left = input_length;
			bool header_section = false;
			for (; input_string != end; ++input_string, --input_left) {
				u8 skip_trailing_chars = 0;
				if (header_section) {
					ssize_t separator_index = string_find_index_in_line(input_string, input_left, ':');
					if (separator_index < 0) {
						line_end_index = separator_index;
					}
					else {
						line_end_index = string_find_line_end(input_string, input_left);
						if (input_string[line_end_index] == '\r') {
							skip_trailing_chars = 1;
						}

						const u8 * value_start = input_string + separator_index + 1;
						u8 value_length = line_end_index - separator_index - skip_trailing_chars;
						value_length -= string_ignore_space(&value_start, value_length);

						//printf("%lu %hhu %.*s:%.*s\n", separator_index, value_length, separator_index, input_string, value_length, value_start);
						set_header_string_value(&request_data, input_string, separator_index, value_start, value_length);
					}
				}
				else {
					line_end_index = string_find_line_end(input_string, input_left);
					if (input_string[line_end_index] == '\r') {
						skip_trailing_chars = 1;
					}
				}

				if (line_end_index < 0) {
					// Bad request (?)
					break;
				}

				printf("CLIENT > %.*s\n", line_end_index, input_string);
				header_section = true;
				input_string += line_end_index + skip_trailing_chars;
				input_left -= line_end_index + skip_trailing_chars;
			} 

			printf("Recognized headers: \n");
			printf("| Header%24s | Value\n", "");
			for (s8 i = 0; i < standard_headers_count; ++i) {
				const char * header_name = standard_header_names[i];
				String header_value = get_header_string_value(&request_data, header_name, strlen(header_name));

				if (header_value.start != NULL) {
					printf("| %30s | %.*s\n", header_name, header_value.length, header_value.start);
				}
			}

			size_t content_length = 0;
			const char content_length_name[] = "Content-Length";
			String header_value = get_header_string_value(&request_data, content_length_name, strlen(content_length_name));
			if (string_is_null(header_value)) {
				printf("Content-Length header is not present. Assumming no request body.\n");
			}
			else {
				// TODO: Convert string to number considering length and not null byte
				//content_length = strtoul();
			}

			printf("Considering argument a path.\n");
			const char * path = NULL;
			if (argument->length == 1 && argument->start[0] == '/') {
				printf("Client is requesting root resource.\n");
				path = "index.html";
			}
			else {
				// Ignore first character: "/"
				memcpy(input, argument->start + 1, argument->length - 1);
				input[argument->length - 1] = 0;
				path = input;
			}

			printf("Requested resource: %s\n", path);

			struct stat file_info;
			u8 * resource_contents = NULL;
			size_t resource_length = 0;
			u16 status_code = 200;
			if (stat(path, &file_info) != 0) {
				print_error("Couldn't open requested resource.\n");
				perror("stat");

				if (errno == ENOENT) {
					status_code = 404;
				}
				else {
					status_code = 500;
				}
			}
			else {
				size_t file_size = file_info.st_size;

				resource_contents = malloc(file_size);
				if (resource_contents == NULL) {
					print_error("Couldn't allocate enough memory to hold the requested resource.\n");
				}
				else {
					FILE * requested_resource = fopen(path, "r");
					resource_length = fread(resource_contents, SIZE_U8, file_size, requested_resource);
					if (resource_length < file_size) {
						printf("An error occured while reading the resource.\n");
						perror("read");
					}
				}
			}

			printf("Sending response message to client.\n");

			const u8 response_status_data[] = "HTTP/1.1 %hu %s\n"
				"Server: Mis huevos\n"
				"Connection: close\n";
			size_t output_length = snprintf(output, sizeof(output) - sizeof(response_status_data), response_status_data, status_code, status_code_titles[status_code]);

			if (resource_length > 0) {
				const u8 content_length_template[] = "Content-Length: %lu\n";
				output_length += snprintf(output + output_length, sizeof(output) - output_length - sizeof(content_length_template), content_length_template, resource_length);

				if (strstr(path, "css") != NULL) {
					const char content_type[] = "Content-Type: text/css\n";
					memcpy(output + output_length, content_type, sizeof(content_type));
					output_length += strlen(content_type);
				}
				else {
					const char content_type[] = "Content-Type: text/html\n";
					memcpy(output + output_length, content_type, sizeof(content_type));
					output_length += strlen(content_type);
				}
			}


			printf( "[INFO] Header section length is: %lu.\n"
					"[INFO] Content length is: %lu.\n"
					"[INFO] Total is: %lu.\n",
					output_length, resource_length, output_length + resource_length);

			memcpy(output + output_length, "\n", 1);
			output_length += 1;

			print_lines(output, output_length, "SERVER > ");
			print_lines(resource_contents, resource_length, "SERVER > ");

			write(client_socket_file_descriptor, output, output_length);
			write(client_socket_file_descriptor, resource_contents, resource_length);

			memset(input, 0, input_length);
			memset(output, 0, output_length);

			break;

		}

		request_data.buffer.used = 0;
		memset(request_data.header_values, 0, sizeof(request_data.header_values[0]) * request_data.header_values_capacity);
		close(client_socket_file_descriptor);
		printf("Connection #%llu closed.\n\n", connection_number);
		connection_number += 1;
	}

	return 0;
}
