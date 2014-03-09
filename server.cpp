#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include <stdint.h>
#include <syslog.h>
#include <signal.h>
#include <iostream>
#include <bullet/btBulletDynamicsCommon.h>
#include "world.h"
#include "build/libwebsockets/lib/libwebsockets.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

class User{
public:
  int32_t id;
  std::string name;
  
  User(){
    id = random();
    name = "ready player one";
  }
  
  std::string inspect(){
    return name + std::string(" ") + std::to_string(id);
  }
};


int force_exit = 0;

#define MAX_ECHO_PAYLOAD 1400
#define LOCAL_RESOURCE_PATH INSTALL_DATADIR"/libwebsockets-test-server"

struct per_session_data__echo {
	unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + MAX_ECHO_PAYLOAD + LWS_SEND_BUFFER_POST_PADDING];
	unsigned int len;
	unsigned int index;
};

static UpdatePacket update;

static int callback_echo(
  struct libwebsocket_context *context,
  struct libwebsocket *wsi,
	enum libwebsocket_callback_reasons reason, 
	void *user,
	void *in, 
	size_t len
){
	struct per_session_data__echo *pss = (struct per_session_data__echo *)user;
	int n;

  // UpdatePacket update;
  
  // printf("%u\n", sizeof(UpdatePacket));
  
	switch (reason) {

  case LWS_CALLBACK_CLIENT_ESTABLISHED:{
    void *userContext = libwebsocket_context_user(context);
    User *user = reinterpret_cast<User*>(userContext);
    std::cout << user->inspect() << std::endl;
  }

	case LWS_CALLBACK_SERVER_WRITEABLE:{
	  // LWS_WRITE_TEXT
	  // n = libwebsocket_write(wsi, &pss->buf[LWS_SEND_BUFFER_PRE_PADDING], pss->len, LWS_WRITE_BINARY);
	  n = libwebsocket_write(wsi, (unsigned char *) &update, sizeof(UpdatePacket), LWS_WRITE_BINARY);
		if (n < 0) {
			lwsl_err("ERROR %d writing to socket, hanging up\n", n);
			return 1;
		}
		if (n < (int)pss->len) {
			lwsl_err("Partial write\n");
			return -1;
		}
	}

	case LWS_CALLBACK_RECEIVE:{
		if (len > MAX_ECHO_PAYLOAD) {
			lwsl_err("Server received packet bigger than %u, hanging up\n", MAX_ECHO_PAYLOAD);
			return 1;
		}
		
		memcpy(&pss->buf[LWS_SEND_BUFFER_PRE_PADDING], in, len);
		pss->len = (unsigned int)len;
		
    printf("Message: %s\n", in);
    
		libwebsocket_callback_on_writable(context, wsi);
	}

	default:{
		break;
	}
	}

	return 0;
}



static struct libwebsocket_protocols protocols[] = {
	/* first protocol must always be HTTP handler */

	{
		"default",		/* name */
		callback_echo,		/* callback */
		sizeof(struct per_session_data__echo)	/* per_session_data_size */
	},
	{
		NULL, NULL, 0		/* End of list */
	}
};

void sighandler(int sig){
	force_exit = 1;
}

static struct option options[] = {
	{ "help",	no_argument,		NULL, 'h' },
	{ "debug",	required_argument,	NULL, 'd' },
	{ "port",	required_argument,	NULL, 'p' },
#ifndef LWS_NO_CLIENT
	{ "client",	required_argument,	NULL, 'c' },
	{ "ratems",	required_argument,	NULL, 'r' },
#endif
	{ "ssl",	no_argument,		NULL, 's' },
	{ "interface",  required_argument,	NULL, 'i' },
#ifndef LWS_NO_DAEMONIZE
	{ "daemonize", 	no_argument,		NULL, 'D' },
#endif
	{ NULL, 0, 0, 0 }
};

void report_errors(lua_State *L, int status)
{
  if ( status!=0 ) {
    std::cerr << "-- " << lua_tostring(L, -1) << std::endl;
    lua_pop(L, 1); // remove error message
  }
}

int main(int argc, char **argv)
{
	int n = 0;
	int port = 7681;
	int use_ssl = 0;
	struct libwebsocket_context *context;
	int opts = 0;
	char interface_name[128] = "";
	const char *interface = NULL;
	int syslog_options = LOG_PID | LOG_PERROR;
	int client = 0;
	int listen_port;
	struct lws_context_creation_info info;
	char address[256];
	int rate_us = 250000;
	unsigned int oldus = 0;
	struct libwebsocket *wsi;

	int debug_level = 7;
	int daemonize = 0;

	memset(&info, 0, sizeof info);

	lwsl_notice("Built to support server operations\n");

	/* we will only try to log things according to our debug_level */
	setlogmask(LOG_UPTO (LOG_DEBUG));
	openlog("lwsts", syslog_options, LOG_DAEMON);

	/* tell the library what debug level to emit and to send it to syslog */
	lws_set_log_level(debug_level, lwsl_emit_syslog);

	lwsl_notice("libwebsockets echo test - "
			"(C) Copyright 2010-2013 Andy Green <andy@warmcat.com> - "
						    "licensed under LGPL2.1\n");
		lwsl_notice("Running in server mode\n");
		listen_port = port;

	info.port = listen_port;
	info.iface = interface;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;
	info.options = opts;
  info.user = (void *) new User;
  
	context = libwebsocket_context(&info);

	if (context == NULL) {
		lwsl_err("libwebsocket init failed\n");
		return -1;
	}

	signal(SIGINT, sighandler);

  World *world = new World;
  world->addGround();
  world->addObject();
  
  const char* file = "setup.lua";
  
  lua_State *L = luaL_newstate();

  luaL_openlibs(L);

  // luaopen_io(L); // provides io.*
  // luaopen_base(L);
  // luaopen_table(L);
  // luaopen_string(L);
  // luaopen_math(L);
  // luaopen_loadlib(L);

  std::cerr << "-- Loading file: " << file << std::endl;

  int s = luaL_loadfile(L, file);

  if ( s==0 ) {
    // execute Lua program
    s = lua_pcall(L, 0, LUA_MULTRET, 0);
  }

	n = 0;
	while (force_exit) { // }!force_exit) { // n >= 0 && !force_exit) {
		struct timeval tv;

    world->simulate();
    
    // if (client) {
    //  gettimeofday(&tv, NULL);
    // 
    //  if (((unsigned int)tv.tv_usec - oldus) > (unsigned int)rate_us) {
    //    libwebsocket_callback_on_writable_all_protocol(&protocols[0]);
    //    oldus = tv.tv_usec;
    //  }
    // }

    libwebsocket_callback_on_writable_all_protocol(&protocols[0]);

		n = libwebsocket_service(context, 10);
	}

  report_errors(L, s);
  lua_close(L);
  std::cerr << std::endl;

	libwebsocket_context_destroy(context);

	lwsl_notice("libwebsockets-test-echo exited cleanly\n");
	closelog();

	return 0;
}
