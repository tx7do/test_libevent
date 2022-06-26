#include <iostream>
#include "server_application.h"

int main()
{
	ServerApplication app(7080);
	return app.run();
}
