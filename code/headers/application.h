#pragma once

#include <string>
#include <iostream>
#include "volumeReader.h"

class Application
{
private:
	std::string volumePath="";

	VolumeReader volReader;
	GLFCameraWindow* w_handle;
public:
	Application(int argc, char** argv);

	bool run ();
};