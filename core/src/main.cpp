#include <vector>
#include <string>

#include "controller.h"

int main(int argc, char** argv)
{
	std::vector<std::string> vargv;
	for (int i = 0; i < argc; i++)
		vargv.push_back(argv[i]);

	CController ctrl;

	int res = 0;
	res = ctrl.Initialize(vargv);
	if (res != 0) {
		return res;
	}

	return ctrl.Run();
}
