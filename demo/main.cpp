// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include <iostream>
#include <cstring>

#include "unit_manager.h"
#include "units/timer_unit.h"
#include "util/time.h"

// timer test
int run_timer();

// network
bool run_tcp_client(usrv::IP host, usrv::PORT port, int client_num);
bool run_tcp_server(usrv::PORT port);

// lua
bool run_lua();

int main(int argc, char* argv[])
{
	if(strcmp(argv[1], "timer") == 0)
	{
		run_timer();
	}
	else if (strcmp(argv[1], "tcpclient") == 0)
	{
		run_tcp_client("127.0.0.1", 6666, 1);
	}
	else if (strcmp(argv[1], "tcpserver") == 0)
	{
		run_tcp_server(6666);
	}
	else if (strcmp(argv[1], "lua") == 0)
	{
		run_lua();
	}
	return 0;
}