#include "stdio.h"
#include "KBEMain.h"


int main(int argc, char* argv[])
{
	printf("hello world! \n");

	ClientApp& app = ClientApp::getSingleton();
	app.BeginPlay();

	while (true)
	{
		app.TickComponent(0.1f);
		Sleep(100);
	}

	app.EndPlay();

	int a = getchar();
}
