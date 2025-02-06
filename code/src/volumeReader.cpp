#include "volumeReader.h"

bool VolumeReader::readVolume(std::string filename)
{
    FILE* file = fopen(filename.c_str(), "rb");
    if (NULL == file)
    {
        return false;
    }
    fread(volume, sizeof(GLubyte), vol_size, file);
    fclose(file);
    return true;
}

unsigned char* VolumeReader::getVolume()
{
    return volume;
}

float VolumeReader::getVolumeDimensionX() {
    return x_size;
}

float VolumeReader::getVolumeDimensionY() {
    return y_size;
}

float VolumeReader::getVolumeDimensionZ() {
    return z_size;
}