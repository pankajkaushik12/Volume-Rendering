#include "application.h"

Application::Application(int argc, char** argv)
{
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-volumePath") == 0) {
			volumePath = argv[i + 1];
		}
	}

	if (!volReader.readVolume(volumePath))                                      // Reading the Volume
	{
		std::cout << "Volume does not exist" << std::endl;
		exit(EXIT_FAILURE);
	}

	w_handle = new GLFCameraWindow(WIDTH, HEIGHT, WINDOWNAME);

	w_handle->Create3DVolumeTexture(volReader.getVolume(), volReader.getVolumeDimensionX(), volReader.getVolumeDimensionY(), volReader.getVolumeDimensionZ());

	w_handle->Create1DTransferFunction();
}

bool Application::run()
{
	return w_handle->Run();
}