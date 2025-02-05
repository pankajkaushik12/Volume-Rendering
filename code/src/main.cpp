#include "utils.h"

#define  GLM_FORCE_RADIANS
#define  GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include<cmath>

#include<iostream>
//#include <unistd.h>
#include <direct.h>
#include <stdio.h>

bool load_volume(const char* filename);

GLfloat* createTransferfun(int width, int height);
float x_size = 256;
float y_size = 256;
float z_size = 256;
const int vol_size = x_size*y_size*z_size;
GLubyte* volume = new GLubyte[vol_size];

const int MAX_PATH = 256;

std::string get_current_dir() {
    char buffer[MAX_PATH];
    if (getcwd(buffer, MAX_PATH)) {
        return std::string(buffer);
    }
    else {
        return "";
    }
}

int main(int, char**)
{
    const char *filepath = "../../data/bonzai_volume.raw";
    if(!load_volume(filepath))                                      // Reading the Volume
    {
        std::cout<<"Volume not loaded succesfully"<<std::endl;
        printf("Current working dir: %s\n", get_current_dir().c_str());
        return 0;
    }

    GLFCameraWindow* w_handle = new GLFCameraWindow(WIDTH, HEIGHT, WINDOWNAME);

    w_handle->Create3DVolumeTexture(volume, x_size, y_size, z_size);
    delete [] volume;

    w_handle->Create1DTransferFunction();

    while (true) {   
        if (!w_handle->Run())
        {
            break;
        }
    }

    return 0;
}

bool load_volume(const char* filename)
{
    FILE *file = fopen(filename,"rb");
    if(NULL == file)
    {
        return false;
    }
    fread(volume,sizeof(GLubyte),vol_size,file);
    fclose(file);
    return true;
}
