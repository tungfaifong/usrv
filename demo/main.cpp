// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include <iostream>
#include <cstring>

bool run_client(uint32_t client_num, uint32_t req_num, size_t thread_num = 0, uint32_t info_level = 0);
bool run_server(uint64_t interval, size_t thread_num = 0);

int main(int argc, char* argv[])
{
	if (strcmp(argv[1], "client") == 0)
	{
		run_client(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
	}
	else if (strcmp(argv[1], "benchmark") == 0)
	{
		run_client(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), 2);
	}
	else if (strcmp(argv[1], "server") == 0)
	{
		run_server(atoi(argv[2]), atoi(argv[3]));
	}
	return 0;
}