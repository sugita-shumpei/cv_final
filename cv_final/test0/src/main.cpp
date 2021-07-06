#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <Camera.h>
#include <GLLib.h>
#include <Utils.h>
#include <Config.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <iostream>
#include <tuple>
#include <filesystem>
#include <cstdio>
#include <string>
#include <string_view>
class Application{
private:
    struct Vertex {
        glm::vec3 position;
        glm::vec2 texCoord;
    };
    struct LightMapHeader {
        int width;
        int height;
        int sCount;
        int tCount;
    };
    struct CameraData {
        alignas(16) glm::vec3 eye = glm::vec3(0, 0, 0);
        alignas(16) glm::vec3 u   = glm::vec3(1, 0, 0);
        alignas(16) glm::vec3 v   = glm::vec3(0, 1, 0);
        alignas(16) glm::vec3 w   = glm::vec3(0, 0, 1);
    };
public:
    int Run(){
        this->initContext();
        this->loadProgram();
        this->loadLightMap(std::filesystem::path(CV_FINAL_DATA_DIR"/chess/rectified"));
        this->initCamera();
        this->initScreen();
        this->mainLoop();
        this->cleanUp();
        return 0;
    }
private:
    void    initContext() {
        if (glfwInit() == GLFW_FALSE) {
            throw std::runtime_error("Failed To Initialize GLFW!");
        }
        glfwWindowHint(GLFW_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        m_Window = glfwCreateWindow(m_Width, m_Height, "titel", nullptr, nullptr);
        glfwMakeContextCurrent(m_Window);
        glfwSetWindowUserPointer(m_Window, this);
        glfwSetCursorPosCallback(m_Window, cursorPosCallback);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw std::runtime_error("Failed To Initialize GLAD!");
        }
    }
    void    loadProgram() {
        m_Program = glCreateProgram();
        auto loadShaderFromSource = [](GLenum shaderType, const std::string_view& shaderSource) {
            const char* shaderSourcePtr = shaderSource.data();
            GLuint shader = glCreateShader(shaderType);
            glShaderSource(shader, 1, &shaderSourcePtr, nullptr);
            glCompileShader(shader);
            GLint  status;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
            if (status != GL_TRUE) {
                std::cerr << "Failed To Compile Shader\n";
            }
            GLint logLength;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength > 0) {
                std::string log(logLength + 1, ' ');
                glGetShaderInfoLog(shader, logLength, nullptr, log.data());
                log.resize(logLength);
                std::cout << log << std::endl;
            }
            return shader;
        };
        auto vsShader = loadShaderFromSource(GL_VERTEX_SHADER, vsSource);
        auto fsShader = loadShaderFromSource(GL_FRAGMENT_SHADER, fsSource);
        glAttachShader(m_Program, vsShader);
        glAttachShader(m_Program, fsShader);
        glLinkProgram(m_Program);
        {
            GLint  status;
            glGetProgramiv(m_Program, GL_LINK_STATUS, &status);
            if (status != GL_TRUE) {
                std::cerr << "Failed To Compile Shader\n";
            }
            GLint logLength;
            glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength > 0) {
                std::string log(logLength + 1, ' ');
                glGetProgramInfoLog(m_Program, logLength, nullptr, log.data());
                log.resize(logLength);
                std::cout << log << std::endl;
            }
        }
        glDeleteShader(vsShader);
        glDeleteShader(fsShader);
    }
    void    loadLightMap(const std::filesystem::path& imageFileDir ) {
        int    imageWidth  = 0;
        int    imageHeight = 0;
        int    imageComp   = 0;
        int    imageSCount = 0;
        int    imageTCount = 0;
        bool isFirst = true;
        for (const std::filesystem::directory_entry& dir : std::filesystem::directory_iterator(imageFileDir)) {
            auto filepath = dir.path().string();
            auto filename = dir.path().filename().string();
            int x, y;
            float camX, camY;
            sscanf(filename.c_str(), "out_%d_%d_%f_%f_.png", &x, &y, &camX, &camY);
            std::cout << x << " " << y << std::endl;
            if (isFirst) {
                auto* image = stbi_load(filepath.c_str(), &imageWidth, &imageHeight, &imageComp, 4);
                isFirst = false;
            }
            imageSCount = std::max(imageSCount, x + 1);
            imageTCount = std::max(imageTCount, y + 1);
        }
        m_LightMapHeader.width = imageWidth;
        m_LightMapHeader.height = imageHeight;
        m_LightMapHeader.sCount = imageSCount;
        m_LightMapHeader.tCount = imageTCount;
        glGenBuffers(1, &m_LightMapSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightMapSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned char) * 4 * imageWidth * imageHeight * imageSCount * imageTCount, nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(LightMapHeader), &m_LightMapHeader);
        for (const std::filesystem::directory_entry& dir : std::filesystem::directory_iterator(imageFileDir)) {
            auto filepath = dir.path().string();
            auto filename = dir.path().filename().string();
            int x, y;
            float camX, camY;
            sscanf(filename.c_str(), "out_%d_%d_%f_%f_.png", &x, &y, &camX, &camY);
            std::cout << x << "," << y << "," << camX << "," << camY << std::endl;
            int uvIndex = imageSCount * y + x;
            auto* image = stbi_load(filepath.c_str(), &imageWidth, &imageHeight, &imageComp, 4);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(LightMapHeader) + sizeof(unsigned char) * 4 * uvIndex * imageWidth * imageHeight, sizeof(unsigned char) * 4 * imageWidth * imageHeight, image);
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        float imageAspect = static_cast<float>(imageWidth) / static_cast<float>(imageHeight);
        if (imageAspect > 1.0f) {
            m_Height = m_Width / imageAspect;
        }
        else {
            m_Width  = m_Height * imageAspect;
        }
        glfwSetWindowSize(m_Window, m_Width, m_Height);
    }
    void    initCamera(){
        m_CameraController = cvlib::CameraController(glm::vec3(0.0f, 0.0f, -10.0f));
        cvlib::Camera camera = m_CameraController.GetCamera(m_CameraFovY, static_cast<float>(m_Width) / static_cast<float>(m_Height));
        auto [u, v, w] = camera.getUVW();
        CameraData cameraData = {};
        cameraData.eye = camera.getEye();
        cameraData.u = u;
        cameraData.v = v;
        cameraData.w = w;
        glGenBuffers(1, &m_CameraSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_CameraSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(CameraData), &cameraData, GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
    void    initScreen() {
        glGenBuffers(1, &m_ScreenVBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_ScreenVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * std::size(vertices), std::data(vertices), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glGenBuffers(1, &m_ScreenIBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * std::size(indices), std::data(indices), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glGenVertexArrays(1, &m_ScreenVAO);
        glBindVertexArray(m_ScreenVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_ScreenVBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenIBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), &static_cast<Vertex*>(0)->position);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &static_cast<Vertex*>(0)->texCoord);
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    void    mainLoop() {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        this->initState();
        GLint resolutionLoc  = glGetUniformLocation(m_Program, "resolution");
        glm::vec2 resolution = glm::vec2(1.0f / m_Width, 1.0f / m_Height);
        while (!glfwWindowShouldClose(m_Window)) {
            glfwPollEvents();
            if (m_CameraUpdate) {
                cvlib::Camera camera  = m_CameraController.GetCamera(m_CameraFovY, static_cast<float>(m_Width) / static_cast<float>(m_Height));
                auto [u, v, w] = camera.getUVW();
                CameraData cameraData = {};
                cameraData.eye = camera.getEye();
                cameraData.u = u;
                cameraData.v = v;
                cameraData.w = w;
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_CameraSSBO);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(CameraData), &cameraData);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
                m_CameraUpdate = false;
            }
            glClear(GL_COLOR_BUFFER_BIT);
            glViewport(0, 0, m_Width, m_Height);
            glUseProgram(m_Program);
            glBindVertexArray(m_ScreenVAO);
            glUniform2f(resolutionLoc, resolution.x, resolution.y);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_LightMapSSBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_CameraSSBO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glfwSwapBuffers(m_Window);
            this->updateState();
        }
    }
    void    cleanUp() {
        glDeleteProgram(m_Program);
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }
    void    initState() {
        glfwSetTime(0.0f);
        this->m_CurrentTime = 0.0f;
        this->m_DeltaTime   = 0.0f;
        double xPos, yPos;
        glfwGetCursorPos(m_Window, &xPos, &yPos);
        glm::vec2 newCursorPos   = glm::vec2(xPos, yPos);
        this->m_DeltaCursorPos   = newCursorPos - this->m_CurrentCursorPos;
        this->m_CurrentCursorPos = newCursorPos;
        this->m_CameraUpdate     = false;
    }
    void    updateState() {
        float newTime = glfwGetTime();
        this->m_DeltaTime   = newTime - this->m_CurrentTime;
        this->m_CurrentTime = newTime;
        if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS) {
            m_CameraController.ProcessKeyboard(cvlib::CameraMovement::eForward, m_DeltaTime);
            m_CameraUpdate = true;
        }
        if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS) {
            m_CameraController.ProcessKeyboard(cvlib::CameraMovement::eBackward, m_DeltaTime);
            m_CameraUpdate = true;
        }
        if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS) {
            m_CameraController.ProcessKeyboard(cvlib::CameraMovement::eLeft, m_DeltaTime);
            m_CameraUpdate = true;
        }
        if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS) {
            m_CameraController.ProcessKeyboard(cvlib::CameraMovement::eRight, m_DeltaTime);
            m_CameraUpdate = true;
        }
        if (glfwGetKey(m_Window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            m_CameraController.ProcessKeyboard(cvlib::CameraMovement::eLeft, m_DeltaTime);
            m_CameraUpdate = true;
        }
        if (glfwGetKey(m_Window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            m_CameraController.ProcessKeyboard(cvlib::CameraMovement::eRight, m_DeltaTime);
            m_CameraUpdate = true;
        }
        if (glfwGetKey(m_Window, GLFW_KEY_UP) == GLFW_PRESS) {
            m_CameraController.ProcessKeyboard(cvlib::CameraMovement::eUp, m_DeltaTime);
            m_CameraUpdate = true;
        }
        if (glfwGetKey(m_Window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            m_CameraController.ProcessKeyboard(cvlib::CameraMovement::eDown, m_DeltaTime);
            m_CameraUpdate = true;
        }
        if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            m_CameraController.ProcessMouseMovement(-m_DeltaCursorPos.x, m_DeltaCursorPos.y);
            m_CameraUpdate = true;
        }
    }
    static void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
        Application* app = nullptr;
        if (app = static_cast<Application*>(glfwGetWindowUserPointer(window))) {
            glm::vec2 newCursorPos  = glm::vec2(xPos, yPos);
            app->m_DeltaCursorPos   = newCursorPos - app->m_CurrentCursorPos;
            app->m_CurrentCursorPos = newCursorPos;
        }
    }
private:
    GLFWwindow*             m_Window           = nullptr;
    int                     m_Width            = 640;
    int                     m_Height           = 480;
    LightMapHeader          m_LightMapHeader   = {};
    GLuint                  m_Program          = 0;
    GLuint                  m_ScreenVAO        = 0;
    GLuint                  m_ScreenVBO        = 0;
    GLuint                  m_ScreenIBO        = 0;
    GLuint                  m_LightMapSSBO     = 0;
    GLuint                  m_CameraSSBO       = 0;
    cvlib::CameraController m_CameraController = {};
    float                   m_CameraFovY       = 70.f;
    //
    bool                    m_CameraUpdate     = false;
    float                   m_CurrentTime      = 0.0f;
    float                   m_DeltaTime        = 0.0f;
    glm::vec2               m_CurrentCursorPos = {};
    glm::vec2               m_DeltaCursorPos   = {};
private:
    static inline constexpr std::string_view vsSource =
        "#version 450 core\n"
        "layout(location=0) in vec3 vPosition;\n"
        "layout(location=1) in vec2 vTexCoord;\n"
        "out vec2 fTexCoord;\n"
        "void main(){\n"
        "   gl_Position = vec4(vPosition,1.0);\n"
        "   fTexCoord = vTexCoord;\n"
        "}\n";
    static inline constexpr std::string_view fsSource =
        "#version 450 core\n"
        "layout(std430, binding=0) buffer LightMap{\n"
        "   int  width;\n"
        "   int height;\n"
        "   int sCount;\n"
        "   int tCount;\n"
        "   unsigned char data[];\n "
        "} lightMap;\n"
        "vec3 ReadLightMap(int s, int t, int u, int v){\n"
        "   int i = lightMap.sCount*t+s;\n"
        "   int j = lightMap.width*v+u;\n"
        "   int k = lightMap.width*lightMap.height*i+j;\n"
        "   return vec3(float(lightMap.data[4*k+0])/255.f,float(lightMap.data[4*k+1])/255.f,float(lightMap.data[4*k+2])/255.f);\n"
        "}\n"
        "layout(std430, binding=1) buffer Camera{\n"
        "   vec3 eye;\n"
        "   vec3 u;\n"
        "   vec3 v;\n"
        "   vec3 w;\n"
        "} camera;\n"
        "uniform float time;\n"
        "uniform vec2  resolution;\n"
        "in vec2 fTexCoord;\n"
        "layout(location=0) out vec4 fragColor;\n"
        "bool IsValidTexCoord(vec2 uv){\n"
        "   return (0.0<=uv.x)&&(uv.x<=1.0f)&&(0.0<=uv.y)&&(uv.y<=1.0f);\n"
        "}\n"
        "float random (in vec2 st) {\n"
        "   return fract(sin(dot(st.xy,vec2(12.9898, 78.233))) *43758.5453123);\n"
        "}\n"
        "vec3 mainCalc1(vec2 texCoord){\n"
        "   vec3  dir     = normalize(camera.u * (texCoord.x-0.5f) + camera.v * (texCoord.y-0.5f) + camera.w);\n"
        "   float stLen   = (0.0f-camera.eye.z)/dir.z;\n"
        "   vec2  st      = camera.eye.xy + dir.xy*stLen + vec2(0.5f);\n"
        "   float uvLen   = (1.0f-camera.eye.z)/dir.z;\n"
        "   vec2  uv      = (camera.eye.xy + dir.xy*uvLen)/2.0f + vec2(0.5f);\n"
        "   if(!IsValidTexCoord(uv)||!IsValidTexCoord(st)){\n"
        "       return vec3(0.0f,0.0f,0.0f);\n"
        "   }\n"
        "   else{\n"
        "       vec3 color = vec3(0.0f,0.0f,0.0f);\n"
        "       float s    = lightMap.sCount*st.x;\n"
        "       float t    = lightMap.tCount*st.y;\n"
        "       float u    = lightMap.width *uv.x;\n"
        "       float v    = lightMap.height*uv.y;\n"
        "       if(float(int(s))>=lightMap.sCount-2){return vec3(1.0,0.0,0.0);}\n"
        "       if(float(int(t))>=lightMap.tCount-2){return vec3(1.0,0.0,0.0);}\n"
        "       for(int dv = 0;dv<2;++dv){\n"
        "           float wv = abs(float(int(v+dv))-v);\n"
        "           for(int du = 0;du<2;++du){\n"
        "               float wu = abs(float(int(u+du))-u);\n"
        "               for(int dt = 0;dt<2;++dt){\n"
        "                   float wt = t<lightMap.tCount-2?abs(float(int(t+dt))-t):0.5f;\n"
        "                   for(int ds = 0;ds<2;++ds){\n"
        "                       float ws = s<lightMap.sCount-2?abs(float(int(s+ds))-s):0.5f;\n"
        "                       float w  = ws*wt*wu*wv;\n"
        "                       color += vec3(ReadLightMap(clamp(int(s+ds),0,lightMap.sCount-1),clamp(int(t+dt),0,lightMap.tCount-1),clamp(int(u+du),0,lightMap.width-1),clamp(int(v+dv),0,lightMap.height-1)))* w;\n"
        "                   }\n"
        "               }\n"
        "           }\n"
        "       }\n"
        "       return color;\n"  
        "   }\n"
        "}\n"
        "vec3 mainCalc2(vec2 texCoord){\n"
        "   vec3  dir     = normalize(camera.u * (texCoord.x-0.5f) + camera.v * (texCoord.y-0.5f) + camera.w);\n"
        "   float stLen   = (0.0f-camera.eye.z)/dir.z;\n"
        "   vec2  st      = camera.eye.xy + dir.xy*stLen + vec2(0.5f);\n"
        "   float uvLen   = (1.0f-camera.eye.z)/dir.z;\n"
        "   vec2  uv      = camera.eye.xy + dir.xy*uvLen + vec2(0.5f);\n"
        "   if(!IsValidTexCoord(uv)||!IsValidTexCoord(st)){\n"
        "       return vec3(0.0f,0.0f,0.0f);\n"
        "   }\n"
        "   else{\n"
        "       int s      = int(lightMap.sCount*st.x);\n"
        "       int t      = int(lightMap.tCount*st.y);\n"
        "       int u      = int(lightMap.width *uv.x);\n"
        "       int v      = int(lightMap.height*uv.y);\n"
        "       vec3 color = vec3(ReadLightMap(s,t,u,v));\n"
        "       return color;\n"
        "   }\n"
        "}\n"
        "void main(){\n"
        "   fragColor = vec4(mainCalc2(fTexCoord),1.0f);\n"
        "}\n";
    static inline constexpr Vertex   vertices[] = {
        {{-1.0,-1.0,0.0},{0.0f,1.0f}},
        {{ 1.0,-1.0,0.0},{1.0f,1.0f}},
        {{ 1.0, 1.0,0.0},{1.0f,0.0f}},
        {{-1.0, 1.0,0.0},{0.0f,0.0f}}
    };
    static inline constexpr uint32_t indices[] = {
        0,1,2,
        2,3,0
    };
};
int main(){
    Application app = {};
    return app.Run();
}