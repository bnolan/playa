#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <syslog.h>
#endif
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <signal.h>
#include <iostream>
#include <assert.h>
#include <bullet/btBulletDynamicsCommon.h>
// #include "build/libwebsockets/lib/private-libwebsockets.h"

#include "world.h"
#include "user.h"

extern "C" {
#include "libwebsockets/lib/libwebsockets.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}


World *world;

#define MAX_ECHO_PAYLOAD 1400
#define LOCAL_RESOURCE_PATH INSTALL_DATADIR"/libwebsockets-test-server"

int force_exit = 0;
int user_id = 1;
lua_State *L;

// class Object{
//   std::string modelUrl;
//   btRigidBody body;
//   LuaState *L;
//   
//   void onTouch(User sender){
//     if LuaHasFunction("touch"){
//       lua.call("touch", sender)
//     }
//   }
// };

struct per_session_data {
  User *user;
};

void sighandler(int sig){
	force_exit = 1;
}

static int callback_echo(struct libwebsocket_context *context,struct libwebsocket *wsi,enum libwebsocket_callback_reasons reason, void *sessionPointer, void *in, size_t len){
	//struct per_session_data__echo *pss = (struct per_session_data__echo *)user;
	int n;
  struct per_session_data *session = (struct per_session_data *) sessionPointer;

  // UpdatePacket update;
  
  // printf("%u\n", sizeof(UpdatePacket));
  
	switch (reason) {

  case LWS_CALLBACK_ESTABLISHED:{
    assert(session->user == NULL);
    session->user = new User(user_id++);
    std::cout << "User #" << session->user->id_ << " connected..." << std::endl;
    break;
  }

  case LWS_CALLBACK_CLOSED:{
    assert(session->user != NULL);
    std::cout << "User #" << session->user->id_ << " disconnected..." << std::endl;
    delete session->user;
    break;
  }
  
	case LWS_CALLBACK_SERVER_WRITEABLE:{
	  // LWS_WRITE_TEXT
	  // n = libwebsocket_write(wsi, &pss->buf[LWS_SEND_BUFFER_PRE_PADDING], pss->len, LWS_WRITE_BINARY);
	  // n = libwebsocket_write(wsi, (unsigned char *) &update, sizeof(UpdatePacket), LWS_WRITE_BINARY);

    // if (n < 0) {
    //  lwsl_err("ERROR %d writing to socket, hanging up\n", n);
    //  return 1;
    // }
    // if (n < (int)pss->len) {
    //  lwsl_err("Partial write\n");
    //  return -1;
    // }
    break;
	}

	case LWS_CALLBACK_RECEIVE:{
    // if (len > MAX_ECHO_PAYLOAD) {
    //  lwsl_err("Server received packet bigger than %u, hanging up\n", MAX_ECHO_PAYLOAD);
    //  return 1;
    // }
		
    // memcpy(&pss->buf[LWS_SEND_BUFFER_PRE_PADDING], in, len);
    // pss->len = (unsigned int)len;

    std::cout << "Message recieved from user#" << session->user->id_ << std::endl;

    char *message = (char *)in;
    std::cout << message << std::endl;

    luaL_dostring(L, message);
    
    // printf("Message: %s\n", in);
    
		libwebsocket_callback_on_writable(context, wsi);
    break;
	}

	default:{
		break;
	}
	}

	return 0;
}



static struct libwebsocket_protocols protocols[] = {
  { "default", callback_echo, sizeof(struct per_session_data) },
  { NULL, NULL, 0 }
};

void report_errors(lua_State *L, int status)
{
  if ( status!=0 ) {
    std::cerr << "-- " << lua_tostring(L, -1) << std::endl;
    lua_pop(L, 1); // remove error message
  }
}

int lua_world_reset(lua_State *L)
{
  // int argc = lua_gettop(L);
  // 
  // std::cerr << "-- my_function() called with " << argc
  //   << " arguments:" << std::endl;
  // 
  // for ( int n=1; n<=argc; ++n ) {
  //   std::cerr << "-- argument " << n << ": "
  //     << lua_tostring(L, n) << std::endl;
  // }
  //
  // lua_pushnumber(L, 123); // return value
  world->reset();

  return 0; // number of return values
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

  // websockets stuff...
	setlogmask(LOG_UPTO (LOG_DEBUG));
	openlog("lwsts", syslog_options, LOG_DAEMON);
	lws_set_log_level(debug_level, lwsl_emit_syslog);

	listen_port = port;

	info.port = listen_port;
	info.iface = interface;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;
  info.user = NULL; // (void *) new User;
  
	// context = libwebsocket_context(&info);
	context = libwebsocket_create_context(&info);

	if (context == NULL) {
		lwsl_err("libwebsocket init failed\n");
		return -1;
	}

	signal(SIGINT, sighandler);

  world = new World;
  world->addGround();
  world->addObject();
  
  const char* file = "setup.lua";
  
  L = luaL_newstate();

  luaL_openlibs(L);

  lua_register(L, "lua_world_reset", lua_world_reset);

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
  force_exit = false;
	while (!force_exit) { // }!force_exit) { // n >= 0 && !force_exit) {
		struct timeval tv;

    world->simulate();
    
    
    world->update();
    
    // if (client) {
    //  gettimeofday(&tv, NULL);
    // 
    //  if (((unsigned int)tv.tv_usec - oldus) > (unsigned int)rate_us) {
    //    libwebsocket_callback_on_writable_all_protocol(&protocols[0]);
    //    oldus = tv.tv_usec;
    //  }
    // }

    libwebsocket_callback_on_writable_all_protocol(&protocols[0]);
		libwebsocket_service(context, 10);

    // 10fps
    #ifdef _WIN32
      Sleep(100);
    #else
      usleep(100000);
    #endif
	}

  report_errors(L, s);
  lua_close(L);
  std::cerr << std::endl;

	libwebsocket_context_destroy(context);

	lwsl_notice("libwebsockets-test-echo exited cleanly\n");
	closelog();

	return 0;
}
