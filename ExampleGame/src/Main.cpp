#include <iostream>
#include "Application.h"

class App : public LizardEngine::Application {
};

int main() {

	auto app = App{};

	LizardEngine::TestFunc();
	
	std::cout << "hello world" << std::endl;
	return 0;
}