/*
 * Copyright (c) 2004-2009 Sergey Lyubka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * $Id: embed.c 70 2008-12-12 10:18:27Z valenok $
 * Unit test for the mongoose web server. Tests embedded API.
 */


#include <stdlib.h>
#include <stdio.h>
#include "mongoose.h"

#if !defined(LISTENING_PORT)
#define LISTENING_PORT	23456
#endif /* !LISTENING_PORT */

static struct mg_context *ctx;
static const char *standard_reply =	"HTTP/1.1 200 OK\r\n"
					"Content-Type: text/plain\r\n"
					"Connection: close\r\n\r\n";

static void
test_get_var(struct mg_connection *conn, const struct mg_request_info *ri,
		void *user_data)
{
	char *value;

	mg_printf(conn, "%s", standard_reply);

	value = mg_get_var(conn, "my_var");
	if (value != NULL) {
		mg_printf(conn, "Value: [%s]", value);
		free(value);
	}
}

static void
test_get_header(struct mg_connection *conn, const struct mg_request_info *ri,
		void *user_data)
{
	const char *value;

	mg_printf(conn, "%s", standard_reply);

	value = mg_get_header(conn, "Host");
	if (value != NULL)
		mg_printf(conn, "Value: [%s]", value);
}

static void
test_get_ri(struct mg_connection *conn, const struct mg_request_info *ri,
		void *user_data)
{
	int	i;

	mg_printf(conn, "%s", standard_reply);

	mg_printf(conn, "Method: [%s]\n", ri->request_method);
	mg_printf(conn, "URI: [%s]\n", ri->uri);
	mg_printf(conn, "HTTP version: [%d/%d]\n",
			ri->http_version_major,
			ri->http_version_minor);

	for (i = 0; i < ri->num_headers; i++)
		mg_printf(conn, "HTTP header [%s]: [%s]\n",
			 ri->http_headers[i].name,
			 ri->http_headers[i].value);


	mg_printf(conn, "Query string: [%s]\n",
			ri->query_string ? ri->query_string: "");
	mg_printf(conn, "POST data: [%.*s]\n",
			ri->post_data_len, ri->post_data);
	mg_printf(conn, "Remote IP: [%lu]\n", ri->remote_ip);
	mg_printf(conn, "Remote port: [%d]\n", ri->remote_port);
	mg_printf(conn, "Remote user: [%s]\n",
			ri->remote_user ? ri->remote_user : "");
}

static void
test_error(struct mg_connection *conn, const struct mg_request_info *ri,
		void *user_data)
{
	const char *value;

	mg_printf(conn, "%s", standard_reply);
	mg_printf(conn, "Error: [%d]", ri->status_code);
}

static void
test_user_data(struct mg_connection *conn, const struct mg_request_info *ri,
		void *user_data)
{
	const char *value;

	mg_printf(conn, "%s", standard_reply);
	mg_printf(conn, "User data: [%d]", * (int *) user_data);
}

int main(void)
{
	int	user_data = 1234;

	ctx = mg_start();
	mg_set_option(ctx, "ports", LISTENING_PORT);

	mg_bind_to_uri(ctx, "/test_get_header", &test_get_header, NULL);
	mg_bind_to_uri(ctx, "/test_get_var", &test_get_var, NULL);
	mg_bind_to_uri(ctx, "/test_get_request_info", &test_get_ri, NULL);
	mg_bind_to_uri(ctx, "/foo/*", &test_get_ri, NULL);
	mg_bind_to_uri(ctx, "/test_user_data", &test_user_data, &user_data);

	mg_bind_to_error_code(ctx, 404, &test_error, NULL);
	mg_bind_to_error_code(ctx, 0, &test_error, NULL);

	for (;;)
		(void) getchar();
}
