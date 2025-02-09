#include "utils.h"
#include <filesystem>

namespace fs = std::filesystem;

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static void glfwindow_reshape_cb(GLFWwindow* window, int width, int height)
{
    GLFWindow* gw = static_cast<GLFWindow*>(glfwGetWindowUserPointer(window));
    assert(gw);
    gw->ResizeWindow(width, height);
}

static void glfwindow_key_cb(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    GLFWindow* gw = static_cast<GLFWindow*>(glfwGetWindowUserPointer(window));
    assert(gw);

    gw->key(key, action, mods);
}

static void glfwindow_mouseMotion_cb(GLFWwindow* window, double xPos, double yPos)
{
    GLFWindow* gw = static_cast<GLFWindow*>(glfwGetWindowUserPointer(window));
    assert(gw);

    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
        gw->mouseMotion(xPos, yPos);
    }
}

static void glfwindow_mouseScroll_cb(GLFWwindow* window, double xoffset, double yoffset) {
    GLFWindow* gw = static_cast<GLFWindow*>(glfwGetWindowUserPointer(window));
    assert(gw);

    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
        gw->mouseScroll(xoffset, yoffset);
    }
}

static void glfwindow_mouseButton_cb(GLFWwindow* window, int button, int action, int mods)
{
    GLFWindow* gw = static_cast<GLFWindow*>(glfwGetWindowUserPointer(window));
    assert(gw);

    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
        gw->mouseButton(button, action, mods);
    }
}

GLFWindow::GLFWindow(int width, int height, std::string windowName): Width(width), Height(height), WindowName(windowName)
{
    setupWindow();
}

const char* GLFWindow::setGLSLVersion() {
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    return glsl_version;
}

int GLFWindow::openGLInit() {
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
    bool err = gladLoadGL(glfwGetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        exit(1);
    }

    //Enable depth buffer (for correct rendering of cube sides)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //Enable multisampling
    glEnable(GL_MULTISAMPLE);

    // Enable Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set point and line size
    glPointSize(10.0f);
    glLineWidth(2.0f);

    // Enable smooth point rendering
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

GLFWwindow* GLFWindow::setupWindow()
{
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);    

    // Create window with graphics context
    Window = glfwCreateWindow(Width, Height, WindowName.c_str(), NULL, NULL);
    if (Window == NULL) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetWindowUserPointer(Window, this);
    glfwMakeContextCurrent(Window);
    glfwSwapInterval(1);

    // Initialize OpenGL loader
    int status = openGLInit();
    if(!status){
        std::cout << "Initialized OpenGL succesfully " << std::endl;
    }
    //std::cout<< "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    glfwSetFramebufferSizeCallback(Window, glfwindow_reshape_cb);
    glfwSetMouseButtonCallback(Window, glfwindow_mouseButton_cb);
    glfwSetKeyCallback(Window, glfwindow_key_cb);
    glfwSetCursorPosCallback(Window, glfwindow_mouseMotion_cb);
    glfwSetScrollCallback(Window, glfwindow_mouseScroll_cb);

    ShaderProgram = CreateShaderProgram(vShaderFile, fShaderFile);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(Window, true);
    ImGui_ImplOpenGL3_Init(setGLSLVersion());

    glEnable(GL_DEPTH_TEST);

    return Window;
}

unsigned int GLFWindow::CreateShaderProgram(const char* vshader_filename, const char* fshader_filename)
{
    //Create shader objects
    GLuint vs, fs;
    if ((vs = createShader(vshader_filename, GL_VERTEX_SHADER)) == 0) return 0;
    if ((fs = createShader(fshader_filename, GL_FRAGMENT_SHADER)) == 0) return 0;

    //Creare program object and link shader objects
    ShaderProgram = glCreateProgram();
    glAttachShader(ShaderProgram, vs);
    glAttachShader(ShaderProgram, fs);
    glLinkProgram(ShaderProgram);
    GLint link_ok;
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        // fprintf(stderr, "glLinkProgram error:");
        // printLog(program);
        std::cout << "Linking error " << std::endl;
        glDeleteShader(vs);
        glDeleteShader(fs);
        glDeleteProgram(ShaderProgram);
        return 0;
    }
    return ShaderProgram;
}

void GLFWindow::printLog(GLuint object)
{
    GLint log_length = 0;
    if (glIsShader(object))
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else if (glIsProgram(object))
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else {
        fprintf(stderr, "printlog: Not a shader or a program\n");
        return;
    }

    char* log = (char*)malloc(log_length);

    if (glIsShader(object))
        glGetShaderInfoLog(object, log_length, NULL, log);
    else if (glIsProgram(object))
        glGetProgramInfoLog(object, log_length, NULL, log);

    fprintf(stderr, "%s", log);
    free(log);
}

GLuint GLFWindow::createShader(const char* filename, GLenum type)
{
    const GLchar* source = getShaderCode(filename);
    if (source == NULL) {
        fprintf(stderr, "Error opening %s: ", filename); perror("");
        return 0;
    }
    GLuint res = glCreateShader(type);
    glShaderSource(res, 1, &source, NULL);
    free((void*)source);

    glCompileShader(res);
    GLint compile_ok = GL_FALSE;
    char infoLog[512];
    glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
    if (compile_ok == GL_FALSE) {
        fprintf(stderr, "%s:", filename);
        printLog(res);
        glDeleteShader(res);
        return 0;
    }

    return res;
}

char* GLFWindow::getShaderCode(const char* filename)
{
    FILE* input = fopen(filename, "rb");
    if (input == NULL) return NULL;

    if (fseek(input, 0, SEEK_END) == -1) return NULL;
    long size = ftell(input);
    if (size == -1) return NULL;
    if (fseek(input, 0, SEEK_SET) == -1) return NULL;

    /*if using c-compiler: dont cast malloc's return value*/
    char* content = (char*)malloc((size_t)size + 1);
    if (content == NULL) return NULL;

    fread(content, 1, (size_t)size, input);
    if (ferror(input)) {
        free(content);
        return NULL;
    }

    fclose(input);
    content[size] = '\0';
    return content;
}

void GLFWindow::RenderGUI()
{
    ImGui::Begin("Information");
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}

void GLFWindow::DrawTransferFunctionEditor() {
    ImGui::Begin("Transfer Function Editor");

    bool HasTransferFunctionModified = false;

    float width = ImGui::GetContentRegionAvail().x;
    float height = 120.0f;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p0 = ImGui::GetCursorScreenPos();

    ImGui::InvisibleButton("PaletteButton", ImVec2(width, height));

    for (size_t i = 0; i < controlPoints.size() - 1; i++) {
        ImVec2 p1 = ImVec2(p0.x + width * controlPoints[i].position, p0.y);
        ImVec2 p2 = ImVec2(p0.x + width * controlPoints[i + 1].position, p0.y + height);
        draw_list->AddRectFilledMultiColor(
            p1, p2,
            ImGui::ColorConvertFloat4ToU32(controlPoints[i].color),
            ImGui::ColorConvertFloat4ToU32(controlPoints[i + 1].color),
            ImGui::ColorConvertFloat4ToU32(controlPoints[i + 1].color),
            ImGui::ColorConvertFloat4ToU32(controlPoints[i].color)
        );
    }

    for (size_t i = 0; i < controlPoints.size() - 1; i++) {
        ImVec2 p1 = ImVec2(p0.x + width * controlPoints[i].position, p0.y + height - controlPoints[i].color.w * height);
        ImVec2 p2 = ImVec2(p0.x + width * controlPoints[i + 1].position, p0.y + height - controlPoints[i + 1].color.w * height);

        draw_list->AddLine(p1, p2, IM_COL32(255, 255, 255, 255), 2.0f);
    }

    for (size_t i = 0; i < controlPoints.size(); i++) {
        ImVec2 pos = ImVec2(p0.x + width * controlPoints[i].position, p0.y + height - controlPoints[i].color.w * height);

        ImU32 pointColor = (selectedControlPoint == i) ? IM_COL32(255, 255, 0, 255) : IM_COL32(255, 255, 255, 255);
        draw_list->AddCircleFilled(pos, 5.0f, pointColor);

        // Drag to move points
        ImGui::SetCursorScreenPos(ImVec2(pos.x - 5, pos.y - 5));
        ImGui::InvisibleButton(("point" + std::to_string(i)).c_str(), ImVec2(10, 10));

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            selectedControlPoint = i;
        }
    }

    bool clickedOnPalette = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    TransferFunctionPoint newPoint;
    bool addControlPoint = false;
    if (clickedOnPalette) {
        ImVec2 mousePos = ImGui::GetMousePos();
        if (mousePos.y - p0.y < height && mousePos.y - p0.y > 0)
        {
            float newPosition = (mousePos.x - p0.x) / width;
            float newAlpha = 1.0f - (mousePos.y - p0.y) / height;
            newPosition = glm::clamp(newPosition, 0.0f, 1.0f);
            newAlpha = glm::clamp(newAlpha, 0.0f, 1.0f);

            newPoint = { newPosition, ImVec4(1, 1, 1, newAlpha) };
            addControlPoint = true;
        }
    }

    if (addControlPoint) {
        for (int i = 0; i < controlPoints.size(); i++)
        {
            if (newPoint.position < controlPoints[i].position) {
                controlPoints.insert(controlPoints.begin() + i, newPoint);
                HasTransferFunctionModified = true;
                break;
            }
        }
        addControlPoint = false;
    }

    ImGui::Dummy(ImVec2(0, height));
    if (selectedControlPoint >= 0 && selectedControlPoint < controlPoints.size()) {
        ImGui::Separator();
        ImGui::Text("Edit Selected Point:");
        if (ImGui::ColorEdit4("RGBA", (float*)&controlPoints[selectedControlPoint].color)) {
            HasTransferFunctionModified = true;
        }
        ImGui::SameLine();
        if (selectedControlPoint != 0 && selectedControlPoint != controlPoints.size() - 1 && ImGui::Button("X")) {
            controlPoints.erase(controlPoints.begin() + selectedControlPoint);
            HasTransferFunctionModified = true;
        }
        if (selectedControlPoint != 0 && selectedControlPoint != controlPoints.size()  - 1 
            && ImGui::DragFloat("Position", &controlPoints[selectedControlPoint].position, 0.01f, 
                glm::max(0.02f, controlPoints[selectedControlPoint - 1].position), glm::min(0.98f, controlPoints[selectedControlPoint + 1].position))) {
            HasTransferFunctionModified = true;
        }
    }
    if (selectedControlPoint == 0 || selectedControlPoint == controlPoints.size() - 1) {
        ImGui::Separator();
    }

    if (HasTransferFunctionModified) { Create1DTransferFunction(); }

    ImGui::InputText("Enter the Transferfunction filename", TfFileName, sizeof(TfFileName));
    if (ImGui::Button("Save Transfer Function")) {
        if (TfFileName[0] != '\0')
        {
            std::string fullPath = "../TransferFunctions/" + std::string(TfFileName) + ".dat";
            SaveTransferFunction(fullPath);
        }
        else {
            std::cout << "Please enter a valid filename!" << std::endl;
        }
    }

    if (ImGui::Button("Load Transfer Function")) {
        showTFSelector = !showTFSelector;
    }
    if (showTFSelector) {
        ShowTransferFunctionSelector();
    }
    ImGui::End();
}

void GLFWindow::ShowTransferFunctionSelector() {
    // Scan folder for .dat files
    std::vector<std::string> tfFiles;
    for (const auto& entry : fs::directory_iterator(tfFolderPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".dat") {
            tfFiles.push_back(entry.path().filename().string());
        }
    }

    if (tfFiles.empty()) {
        ImGui::Text("No transfer function files found.");
        return;
    }

    static int selectedFileIndex = -1;

    if (ImGui::BeginCombo("Transfer Functions", selectedFileIndex == -1 ? "Select a file..." : tfFiles[selectedFileIndex].c_str())) {
        for (int i = 0; i < tfFiles.size(); ++i) {
            bool isSelected = (selectedFileIndex == i);
            if (ImGui::Selectable(tfFiles[i].c_str(), isSelected)) {
                selectedFileIndex = i;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // Load the selected transfer function
    if (selectedFileIndex >= 0) {
        std::string selectedFile = tfFolderPath + "/" + tfFiles[selectedFileIndex];
        if (LoadTransferFunction(selectedFile)) {
            Create1DTransferFunction();
        }
    }
}

void GLFWindow::SetUniforms()
{
    glUseProgram(ShaderProgram);

    GLint vCam_uniform = glGetUniformLocation(ShaderProgram, "camPosition");
    if (vCam_uniform == -1) {
        fprintf(stderr, "Could not bind location: camPosition\n");
        exit(0);
    }
    glUniform3fv(vCam_uniform, 1, glm::value_ptr(camposition));

    GLuint vstep_size = glGetUniformLocation(ShaderProgram, "stepSize");
    if (vstep_size == -1) {
        fprintf(stderr, "Could not bind location: vstep_size\n");
        exit(0);
    }
    glUniform1f(vstep_size, step_size);

    GLuint vExtentMin = glGetUniformLocation(ShaderProgram, "extentmin");
    if (vExtentMin == -1) {
        fprintf(stderr, "Could not bind location: vExtentMin\n");
        exit(0);
    }
    glUniform3f(vExtentMin, 0, 0, -VolumeSize.z);

    GLuint vExtentMax = glGetUniformLocation(ShaderProgram, "extentmax");
    if (vExtentMax == -1) {
        fprintf(stderr, "Could not bind location: vExtentMax\n");
        exit(0);
    }
    glUniform3f(vExtentMax, VolumeSize.x, VolumeSize.y, 0);

    GLuint tex1 = glGetUniformLocation(ShaderProgram, "texture3d");
    if (tex1 == -1) {
        fprintf(stderr, "Could not bind location: texture3d\n");
        exit(0);
    }
    else {
        unsigned int tex;
        glGenVertexArrays(1, &tex);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, volumeTex);
        glUniform1i(tex1, 0);
    }

    GLuint tex2 = glGetUniformLocation(ShaderProgram, "transferfun");
    if (tex2 == -1) {
        fprintf(stderr, "Could not bind location: transferfun\n");
        exit(0);
    }
    else {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, tfTex);
        glUniform1i(tex2, 1);
    }
}

void GLFWindow::SetupViewTransformation()
{
    viewT = glm::lookAt(camposition, camat, camup);

    //Pass-on the viewing matrix to the vertex shader
    glUseProgram(ShaderProgram);
    vView_uniform = glGetUniformLocation(ShaderProgram, "vView");
    if (vView_uniform == -1) {
        fprintf(stderr, "Could not bind location: vView\n");
        exit(0);
    }
    glUniformMatrix4fv(vView_uniform, 1, GL_FALSE, glm::value_ptr(viewT));
}

void GLFWindow::SetupModelTransformation()
{
    //Modelling transformations (Model -> World coordinates)
    modelT = glm::translate(glm::mat4(1.0f), glm::vec3(-VolumeSize.x / 2, -VolumeSize.y / 2, VolumeSize.z / 2));//Model coordinates are the world coordinates

    //Pass on the modelling matrix to the vertex shader
    glUseProgram(ShaderProgram);
    vModel_uniform = glGetUniformLocation(ShaderProgram, "vModel");
    if (vModel_uniform == -1) {
        fprintf(stderr, "Could not bind location: vModel\n");
        exit(0);
    }
    glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(modelT));
}

void GLFWindow::SetupProjectionTransformation()
{
    //Projection transformation
    projectionT = glm::perspective(45.0f, (GLfloat)Width / (GLfloat)Height, 0.1f, 800.0f);

    //Pass on the projection matrix to the vertex shader
    glUseProgram(ShaderProgram);
    vProjection_uniform = glGetUniformLocation(ShaderProgram, "vProjection");
    if (vProjection_uniform == -1) {
        fprintf(stderr, "Could not bind location: vProjection\n");
        exit(0);
    }
    glUniformMatrix4fv(vProjection_uniform, 1, GL_FALSE, glm::value_ptr(projectionT));

    // Updating the screen dimensions in `screenWidth` and `screenHeight`
    if (GLint widthLocation = glGetUniformLocation(ShaderProgram, "screen_width")) {
        glUniform1f(widthLocation, Width);
    }
    if (GLint heightLocation = glGetUniformLocation(ShaderProgram, "screen_height")) {
        glUniform1f(heightLocation, Height);
    }    
}

glm::vec3 GLFWindow::getTrackBallVector(double x, double y)
{
    glm::vec3 p = glm::vec3(2.0 * x / Width - 1.0, 2.0 * y / Height - 1.0, 0.0); //Normalize to [-1, +1]
    p.y = -p.y; //Invert Y since screen coordinate and OpenGL coordinates have different Y directions.

    float mag2 = p.x * p.x + p.y * p.y;
    if (mag2 <= 1.0f)
        p.z = sqrtf(1.0f - mag2);
    else
        p = glm::normalize(p); //Nearest point, close to the sides of the trackball
    return p;
}

void GLFWindow::Create3DVolumeTexture(GLubyte* Volume, float x_size, float y_size, float z_size)
{
    glUseProgram(ShaderProgram);

    glGenTextures(1, &volumeTex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, volumeTex);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, x_size, y_size, z_size, 0, GL_RED, GL_UNSIGNED_BYTE, Volume);

    VolumeSize = glm::vec3(x_size, y_size, z_size);
    CreateBoundingBox();

    SetupModelTransformation();                    // These funs will set and pass the Model, View, Transformation matrix to shaders
    SetupViewTransformation();
    SetupProjectionTransformation();
}

void GLFWindow::Create1DTransferFunction()
{
    int controlPointIndex = 0;
    for (int i = 0; i < 256; i++) {
        float t = float(i) / 255.0f;
        if (t == 0 || t == 1) {
            TransferFun[i * 4 + 0] = controlPoints[controlPointIndex].color.x;
            TransferFun[i * 4 + 1] = controlPoints[controlPointIndex].color.y;
            TransferFun[i * 4 + 2] = controlPoints[controlPointIndex].color.z;
            TransferFun[i * 4 + 3] = controlPoints[controlPointIndex].color.w;
        }
        else {
            if (t > controlPoints[controlPointIndex].position)
            {
                controlPointIndex++;
            }
            float time = (t - controlPoints[controlPointIndex - 1].position) / (controlPoints[controlPointIndex].position - controlPoints[controlPointIndex - 1].position);
            TransferFun[i * 4 + 0] = controlPoints[controlPointIndex - 1].color.x + time * (controlPoints[controlPointIndex].color.x - controlPoints[controlPointIndex - 1].color.x);
            TransferFun[i * 4 + 1] = controlPoints[controlPointIndex - 1].color.y + time * (controlPoints[controlPointIndex].color.y - controlPoints[controlPointIndex - 1].color.y);
            TransferFun[i * 4 + 2] = controlPoints[controlPointIndex - 1].color.z + time * (controlPoints[controlPointIndex].color.z - controlPoints[controlPointIndex - 1].color.z);
            TransferFun[i * 4 + 3] = controlPoints[controlPointIndex - 1].color.w + time * (controlPoints[controlPointIndex].color.w - controlPoints[controlPointIndex - 1].color.w);
        }
        
    }

    glUseProgram(ShaderProgram);

    glGenTextures(1, &tfTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, tfTex);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, TransferFun);                    // Making a 1d transfer function
    glBindTexture(GL_TEXTURE_1D, 0);
}

void GLFWindow::CreateBoundingBox()
{
    float xSize = VolumeSize.x, ySize = VolumeSize.y, zSize = VolumeSize.z;

    glUseProgram(ShaderProgram);

    //Bind shader variables
    int vVertex_attrib = glGetAttribLocation(ShaderProgram, "vVertex");
    if (vVertex_attrib == -1) {
        fprintf(stderr, "Could not bind location: vVertex\n");
        exit(0);
    }
    //Cube data
    GLfloat cube_vertices[] = {
        xSize - 1, ySize - 1, -zSize + 1, 0, ySize - 1, -zSize + 1, 0, 0, -zSize + 1, xSize - 1, 0, -zSize + 1,
        xSize - 1, ySize - 1, 0, 0, ySize - 1, 0, 0, 0, 0, xSize - 1, 0, 0
    };

    GLushort cube_indices[] = {
                0, 1, 2, 0, 2, 3, //Front
                4, 7, 5, 5, 7, 6, //Back
                1, 6, 2, 1, 5, 6, //Left
                0, 3, 4, 4, 7, 3, //Right
                0, 4, 1, 4, 5, 1, //Top
                2, 6, 3, 3, 6, 7 //Bottom
    };

    //Generate VAO object
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    //Create VBOs for the VAO
    //Position information (data + format)
    int nVertices = (6 * 2) * 3; //(6 faces) * (2 triangles each) * (3 vertices each)
    GLfloat* expanded_vertices = new GLfloat[nVertices * 3];
    for (int i = 0; i < nVertices; i++) {
        expanded_vertices[i * 3] = cube_vertices[cube_indices[i] * 3];
        expanded_vertices[i * 3 + 1] = cube_vertices[cube_indices[i] * 3 + 1];
        expanded_vertices[i * 3 + 2] = cube_vertices[cube_indices[i] * 3 + 2];
    }
    GLuint vertex_VBO;
    glGenBuffers(1, &vertex_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
    glBufferData(GL_ARRAY_BUFFER, nVertices * 3 * sizeof(GLfloat), expanded_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vVertex_attrib);
    glVertexAttribPointer(vVertex_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    delete[]expanded_vertices;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); //Unbind the VAO to disable changes outside this function.
}

bool GLFWindow::Run()
{
    if (glfwWindowShouldClose(Window)) return false;

    currentFrameTime = glfwGetTime();
    deltaTime = currentFrameTime - lastFrameTime;

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    RenderGUI();
    DrawTransferFunctionEditor();

    SetUniforms();                         // This will set all the uniform variable inside shaders

    glViewport(0, 0, Width, Height);
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(VAO);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    ImGui::Render();
    ImGui::EndFrame();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(Window);
    glfwSwapInterval(0);
    glfwPollEvents();

    lastFrameTime = currentFrameTime;

    return true;
}

bool GLFWindow::SaveTransferFunction(std::string filename)
{
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        std::cerr << "Failed to open file for saving: " << filename << std::endl;
        return false;
    }

    size_t size = controlPoints.size();
    ofs.write(reinterpret_cast<const char*>(&size), sizeof(size));

    for (const auto& point : controlPoints) {
        ofs.write(reinterpret_cast<const char*>(&point.position), sizeof(point.position));
        ofs.write(reinterpret_cast<const char*>(&point.color), sizeof(point.color));
    }

    ofs.close();
    return true;
}

bool GLFWindow::LoadTransferFunction(std::string filename)
{
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) {
        std::cerr << "Failed to open file for loading: " << filename << std::endl;
        return false;
    }

    size_t size = 0;
    ifs.read(reinterpret_cast<char*>(&size), sizeof(size));  // Read the size of the array

    controlPoints.resize(size);  // Resize the vector to hold the data
    for (auto& point : controlPoints) {
        ifs.read(reinterpret_cast<char*>(&point.position), sizeof(point.position));  // Read position
        ifs.read(reinterpret_cast<char*>(&point.color), sizeof(point.color));        // Read color
    }

    ifs.close();
    return true;
}

void GLFWindow::ResizeWindow(int width, int height)
{
    Width = width;
    Height = height;

    SetupProjectionTransformation();
}

void GLFWindow::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(Window);
    glfwTerminate();
}
