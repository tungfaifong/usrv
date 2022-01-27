// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include <iostream>
#include <cstring>

bool run_client();
bool run_server(uint64_t interval);

int main(int argc, char* argv[])
{
	if (strcmp(argv[1], "client") == 0)
	{
		run_client();
	}
	else if (strcmp(argv[1], "server") == 0)
	{

		run_server(atoi(argv[2]));
	}
	return 0;
}