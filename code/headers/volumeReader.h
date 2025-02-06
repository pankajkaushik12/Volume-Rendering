#pragma once

#include <string>
#include "utils.h"

struct MHDHeader {
	int dims[3]; // Dimensions for the image (NDims x DimSize)
	std::string elementType;
	std::string elementDataFile;
};

class VolumeReader
{
private:
	std::string filePath;

	float x_size = 256;
	float y_size = 256;
	float z_size = 256;
	const int vol_size = x_size * y_size * z_size;
	unsigned char* volume = new unsigned char[vol_size];

public:
	~VolumeReader() {
		delete[] volume;
	}

	bool readVolume(std::string Path);

	unsigned char* getVolume();

	float getVolumeDimensionX();
	float getVolumeDimensionY();
	float getVolumeDimensionZ();
};
