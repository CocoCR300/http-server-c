#include <ctype.h>
#if __has_include("bits/posix1_lim.h")
#include <bits/posix1_lim.h>
#endif

#include <assert.h>
#include <magic.h>
#include <signal.h>
#include <stdarg.h>
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

#include "buffer.h"
#include "definition.h"
#include "error_number.h"
#include "app_string.h"

typedef struct
{
	String status_code_titles[412];

} Context;

typedef struct
{
	Buffer		buffer;

	u64			number;
	s32			client_socket_fd;

	String	*	header_values;
	s64		header_values_capacity;

	String		method;
	String		argument;
	String		version;
} RequestData;

const s8 HTTP_METHOD_NAME_MAX_LENGTH = 7;
const char * standard_method_names[] = { "CONNECT", "DELETE", "GET", "HEAD", "OPTIONS", "PATCH", "POST", "PUT", "TRACE", "UPDATE" };
const char * standard_header_names[] = { "Accept", "Accept-Encoding", "Accept-Language", "Age", "Connection", "Content-Length", "DNT", "Host", "Priority", "Referer", "Sec-Fetch-Dest", "Sec-Fetch-Mode", "Sec-Fetch-Site", "Upgrade-Insecure-Requests", "User-Agent" };
const s8 standard_headers_count = ARRAY_COUNT(standard_header_names);

s32		socket_file_descriptor;
magic_t	magic_obj;

void	print_error(const char * message);
void	print_lines(const u8 * string, s64 length, const char * line_header);
s64 string_find_index(const u8 * string, u8 to_find, s64 length);
s64	string_find_index_in_line(const u8 * string, s64 length, u8 to_find);
s64	string_find_line_end(const u8 * string, s64 length);
s64	string_ignore_space(const u8 ** string, s64 length);
s64	string_multi_find(const u8 * string, u8 * chars, s64 * indices, s64 length, s64 chars_count);

static inline s64 min(s64 value0, s64 value1)
{
	if (value0 < value1) {
		return value0;
	}

	return value1;
}

s64 path_find_extension_index(String * path)
{
	u8 * string = path->start;

	for (s64 i = path->length; i >= 0; --i) {
		if (string[i] == '.') {
			return i + 1;
		}
	}

	return -1;
}

String path_find_extension(String * path)
{
	String extension = STRING_NULL;
	s64 index = path_find_extension_index(path);
	if (index >= 0) {
		extension.start = path->start + index;
		extension.length = path->length - index;
	}

	return extension;
}

void print_error(const char * message)
{
	fprintf(stderr, "[ERROR] %s", message);
}

void print_lines(const u8 * string, s64 length, const char * line_header)
{
	if (string == NULL || length == 0) {
		return;
	}

	s64 line_end_index;
	while ((line_end_index = string_find_line_end(string, length)) >= 0) {

		printf("%s%.*s\n", line_header, line_end_index, string);

		u8 skip_trailing_chars = 1;
		if (string[line_end_index] == '\r') {
			// Assume CRLF
			skip_trailing_chars = 2;

		}

		string += line_end_index + skip_trailing_chars;
		length -= line_end_index + skip_trailing_chars;
	}
}

static inline bool string_equals_c_str(String * string, const char * c_string)
{
	return strncmp(c_string, string->start, string->length) == 0;
}

u8 string_to_s64(String string, s64 * output)
{
	const u8 base = 10;
	bool negative = false;
	s64 length = string.length;
	u8 index = 0;
	u8	* start = string.start;

	*output = 0;
	if (start[0] == '+' || (negative = start[0] == '-')) {
		index += 1;
	}
	for (; index < length; ++index) {
		u8 char_decimal_value = start[index];
		if (!isdigit(char_decimal_value)) {
			break;
		}

		char_decimal_value -= '0';
		u64 value = (*output * base) + char_decimal_value;
		if (value > S64_MAX) {
			// Negate index to signal range error
			index = -index;
			break;
		}

		*output = value;
	}

	if (negative) {
		*output = -(*output);
	}

	return index;
}

s64 string_find_line_end(const u8 * string, s64 length)
{
	for (s64 i = 0; i < length; ++i) {
		u8 character = string[i];
		if (character == '\r' || character == '\n') {
			return i;
		}
	}

	return -1;
}

s64 string_ignore_space(const u8 ** string, s64 length)
{
	const u8	* start = *string,
	   			* end = *string + length;
	for (; start != end; ++start) {
		if (*start != ' ') {
			break;
		}
	}

	s64 ignored = start - *string;
	*string = start;
	return ignored;
}

s64 string_find_index_in_line(const u8 * string, s64 length, u8 to_find)
{
	s64 index = 0;
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

s64 string_find_index(const u8 * string, u8 to_find, s64 length)
{
	for (s64 index = 0; index < length; ++index) {
		if (string[index] == to_find) {
			return index;
		}
	}

	return -1;
}

s64 string_multi_find(const u8 * string, u8 * chars, s64 * indices, s64 length, s64 chars_count)
{
	memset(indices, -1, chars_count);

	s64 found = 0;
	for (s64 index = 0; index < length; ++index) {
		for (s64 chars_index = 0; chars_index < chars_count; ++chars_index) {
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
	
	magic_close(magic_obj);

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

s8 index_by_header_name(const u8 * header_name, s64 length, const char * header_names[], s8 header_count)
{
	for (s8 i = 0; i < header_count; ++i) {
		if (strncmp(header_names[i], header_name, length) == 0) {
			return i;
		}
	}

	return -1;
}

String get_status_code_title(Context * context, u16 status_code)
{
	assert(status_code >= 100 && status_code <= 511);

	return context->status_code_titles[status_code - 100];
}
String get_header_string_value(RequestData * data, const u8 * header_name, s64 header_name_length)
{
	String header_value = STRING_NULL;

	s8 index = index_by_header_name(header_name, header_name_length, standard_header_names, standard_headers_count);
	if (index < 0) {
		return header_value;
	}

	header_value = data->header_values[index];
	return header_value;
}

s8 set_header_string_value(RequestData * data, const u8 * header_name, s64 header_name_length, u8 * header_value, s64 header_value_length)
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

u8 read_requested_resource(const char * path, Buffer * output_buffer)
{
	struct stat file_info;
	FILE * requested_resource_file = fopen(path, "r");

	if (requested_resource_file == NULL) {
		if (errno == ENOMEM) {
			return MEMORY_INSUFFICIENT;
		}
		if (errno == ENOENT) {
			return FILE_NOT_FOUND;
		}
		if (errno == EACCES){
			return FILE_NO_PERMISSION;
		}

		return FILE_UNKNOWN_ERROR;
	}

	if (stat(path, &file_info) != 0) {
		if (errno == ENOMEM) {
			return MEMORY_INSUFFICIENT;
		}

		return FILE_UNKNOWN_ERROR;
	}

	buffer_allocate(output_buffer, file_info.st_size);
	if (buffer_full(output_buffer)) {
		return MEMORY_INSUFFICIENT;
	}

	s64 buffer_size = output_buffer->capacity;
	u8 result = buffer_read_file(output_buffer, requested_resource_file, buffer_size);
	fclose(requested_resource_file);

	return result;
}

// No need to free the given memory, at least that's what I can understand from the libmagic's
// documentation, since nothing is mentioned about it.
const char * determine_resource_mime_type(String * path, const Buffer * resource_buffer)
{
	const char * resource_mime_type = magic_buffer(magic_obj, resource_buffer->start, resource_buffer->used);

	if (resource_mime_type == NULL || strcmp(resource_mime_type, "text/plain") == 0) {
		String extension = path_find_extension(path);

		if (string_equals_c_str(&extension, "css")) {
			resource_mime_type = "text/css";
		}
		else {
			resource_mime_type = "text/plain";
		}
	}

	return resource_mime_type;
}

void process_request(Context * context, RequestData * request_data)
{
	s32 client_socket_fd = request_data->client_socket_fd;
	u8 input[16384] = { 0 };
	u8 output[16384] = { 0 };

	while (true) {
		s64 input_length = read(client_socket_fd, input, sizeof(input));
		printf("Read %ld bytes from client.\n", input_length);

		if (input_length < 0) {
			print_error("Couldn't read data from client.\n");
			break;
		}

		String	* method = &request_data->method,
				* argument = &request_data->argument,
				* version = &request_data->version;
		
		u8 * request_data_buffer = request_data->buffer.start;
		buffer_write(&request_data->buffer, input, 1024);
		s64 line_end_index = string_find_index(request_data_buffer, '\n', request_data->buffer.used);

		s64 index = string_find_index(request_data_buffer, ' ', HTTP_METHOD_NAME_MAX_LENGTH + 1);
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
		
		const u8	* input_string = input,
					* end = input + input_length;
		s64 input_left = input_length;
		bool header_section = false;
		for (; input_string != end; ++input_string, --input_left) {
			u8 skip_trailing_chars = 0;
			if (header_section) {
				s64 separator_index = string_find_index_in_line(input_string, input_left, ':');
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
					set_header_string_value(request_data, input_string, separator_index, value_start, value_length);
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
			String header_value = get_header_string_value(request_data, header_name, strlen(header_name));

			if (header_value.start != NULL) {
				printf("| %30s | %.*s\n", header_name, header_value.length, header_value.start);
			}
		}

		s64 content_length = 0;
		const char content_length_name[] = "Content-Length";
		String header_value = get_header_string_value(request_data, content_length_name, strlen(content_length_name));
		if (string_is_null(header_value)) {
			printf("Content-Length header is not present. Assumming no request body.\n");
		}
		else {
			u8 digits_read = string_to_s64(header_value, &content_length);
			if (digits_read < 0) {
				printf("Content-Length value is too large. Resulting value: %ld\n", content_length);
			}
		}

		printf("Considering argument a path.\n");
		String path;
		char * path_c_str = NULL;
		if (string_equals_c_str(argument, "/")) {
			printf("Client is requesting root resource.\n");
			path_c_str = "index.html";
			path = string_create_from_static(path_c_str);
		}
		else {
			// Ignore first character: "/"
			path = string_ignore_leading_chars(argument, 1);
			path_c_str = input;

			string_to_c_string(&path, path_c_str);
		}

		printf("Requested resource: %s\n", path_c_str);

		u16 status_code;
		Buffer requested_resource;
		u8 result = read_requested_resource(path_c_str, &requested_resource);
		if (result == 0) {
			status_code = 200;
		}
		else if (result == FILE_NOT_FOUND) {
			status_code = 404;
			print_error("Couldn't open requested resource.\n");
		}
		else if (result == FILE_READ_ERROR) {
			status_code = 500;
			print_error("An error occured while reading the resource.\n");
		}
		else if (result == FILE_NO_PERMISSION) {
			status_code = 500;
			print_error("Insufficient permissions to access requested resource.\n");
		}
		else if (result == MEMORY_INSUFFICIENT) {
			status_code = 500;
			print_error("Couldn't allocate enough memory to store the requested resource.\n");
		}
		else {
			status_code = 500;
			print_error("An unknown error occurred while reading the requested resource.\n");
		}

		printf("Sending response message to client.\n");

		const char response_status_data[] = "HTTP/1.1 %hu %.*s\n"
			"Server: Still thinking on a name...\n"
			"Connection: close\n";
		String status_code_title = get_status_code_title(context, status_code);
		s64 output_length = snprintf(output, sizeof(output) - sizeof(response_status_data), response_status_data, status_code, status_code_title.length, status_code_title.start);

		s64 resource_length = requested_resource.used;
		if (status_code == 200 && resource_length > 0) {
			const char content_length_template[] = "Content-Length: %ld\n";
			output_length += snprintf(output + output_length, sizeof(output) - output_length - sizeof(content_length_template), content_length_template, resource_length);

			const char * resource_mime_type = determine_resource_mime_type(&path, &requested_resource);
			const char content_type_template[] = "Content-Type: %s\n";
			output_length += snprintf(output + output_length, sizeof(output) - output_length - strlen(resource_mime_type), content_type_template, resource_mime_type);
		}

		printf(	"[INFO] Header section length is: %ld.\n"
				"[INFO] Content length is: %ld.\n"
				"[INFO] Total is: %ld.\n",
				output_length, resource_length, output_length + resource_length);

		memcpy(output + output_length, "\n", 1);
		output_length += 1;

		print_lines(output, output_length, "SERVER > ");
		print_lines(requested_resource.start, resource_length, "SERVER > ");

		write(client_socket_fd, output, output_length);
		write(client_socket_fd, requested_resource.start, resource_length);

		buffer_free(&requested_resource);

		memset(input, 0, input_length);
		memset(output, 0, output_length);

		break;
	}
}

static inline void fill_status_code(Context * context, u16 status_code, const char * title)
{
	assert(status_code >= 100 && status_code <= 511);
	context->status_code_titles[status_code - 100] = string_create_from_static(title);
}

int main(void)
{
	Context server_context;

	fill_status_code(&server_context, 100, "Continue");
	fill_status_code(&server_context, 101, "Switching Protocols");
	fill_status_code(&server_context, 102, "Processing");
	fill_status_code(&server_context, 103, "Early Hints");
	fill_status_code(&server_context, 200, "OK");
	fill_status_code(&server_context, 400, "Bad Request");
	fill_status_code(&server_context, 404, "Not Found");
	fill_status_code(&server_context, 500, "Internal Server Error");

	u64 connection_number = 0;

	magic_obj = magic_open(MAGIC_MIME_TYPE);
	magic_load(magic_obj, NULL);
	//magic_compile(magic_obj, NULL);

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

	sockaddr_in client_address_info;
	socklen_t client_address_info_len = sizeof(client_address_info);
	while (true) {
		printf("Waiting for incoming connections.\n");
		s32 client_socket_file_descriptor = accept(socket_file_descriptor, (sockaddr *) &client_address_info, &client_address_info_len);
		if (client_socket_file_descriptor == -1) {
			print_error("Couldn't accept incoming connection.\n");
			continue;
		}

		printf("Incoming connection #%lu accepted.\n", connection_number);

		u8 _request_data_buffer[1024] = { 0 };
		String _request_headers[50] = { 0 };

		RequestData request_data;
		request_data.number = connection_number;
		request_data.buffer.start = _request_data_buffer;
		request_data.buffer.capacity = sizeof(_request_data_buffer);
		request_data.buffer.used = 0;
		request_data.header_values = _request_headers;
		request_data.header_values_capacity = sizeof(_request_headers) / sizeof(_request_headers[0]);
		request_data.client_socket_fd = client_socket_file_descriptor;

		process_request(&server_context, &request_data);

		buffer_clear(&request_data.buffer);
		memset(request_data.header_values, 0, sizeof(request_data.header_values[0]) * request_data.header_values_capacity);
		close(client_socket_file_descriptor);
		printf("Connection #%lu closed.\n\n", connection_number);
		connection_number += 1;
	}

	magic_close(magic_obj);

	return 0;
}
