#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <Camera.h>
#include <GLLib.h>
#include <Utils.h>
#include <Config.h>
#include <glm/gtx/string_cast.hpp>
#include <stb_image.h>
#include <stb_image_write.h>
#include <iostream>
#include <tuple>
#include <filesystem>
#include <cstdio>
#include <functional>
#include <memory>
#include <unordered_map>
#include <string>
#include <string_view>
namespace test1 {
	class  Window {
	public:
		Window()noexcept {}
		Window(const Window&) = delete;
		Window(Window&& window)noexcept :m_Window{ window.m_Window }, m_Width{ window.m_Width }, m_Height{ window.m_Height }{
			window.m_Window = nullptr;
			window.m_Width  = 0;
			window.m_Height = 0;
		}
		bool Initialize(GLFWwindow* baseContext, int width, int height, const char* title);
		bool ShouldClose()noexcept {
			return glfwWindowShouldClose(m_Window);
		}
		void PollEvents()noexcept {
			glfwPollEvents();
		}
		void SwapBuffers()noexcept {
			return glfwSwapBuffers(m_Window);
		}
		auto GetHandle()const noexcept-> GLFWwindow* {
			return m_Window;
		}
		auto GetWidth() const noexcept-> int {
			return m_Width;
		}
		auto GetHeight()const noexcept-> int {
			return m_Height;
		}
		auto GetAspect()const noexcept-> float {
			return static_cast<float>(m_Width) / static_cast<float>(m_Height);
		}
		auto GetCurrCursorPos() const noexcept -> glm::dvec2 { return m_CurrCursorPos; }
		auto GetPrevCursorPos() const noexcept -> glm::dvec2 { return m_PrevCursorPos; }
		auto GetDeltCursorPos() const noexcept -> glm::dvec2 { return m_DeltCursorPos; }
		auto GetCurrScrollPos() const noexcept -> glm::dvec2 { return m_CurrScrollPos; }
		auto GetPrevScrollPos() const noexcept -> glm::dvec2 { return m_PrevScrollPos; }
		auto GetDeltScrollPos() const noexcept -> glm::dvec2 { return m_DeltScrollPos; }
		bool IsPressedKey(int keyCode)const  {
			return glfwGetKey(m_Window, keyCode) == GLFW_PRESS;
		}
		bool IsPressedMouseButton(int buttonCode)const  {
			return glfwGetMouseButton(m_Window, buttonCode) == GLFW_PRESS;
		}
		bool IsResized()const noexcept {
			return m_IsResized;
		}
		void Terminate();
	private:
		static void WindowSizeCallback(GLFWwindow* window, int width, int height) {
			if (!window) {
				return;
			}
			test1::Window* thisWindow = reinterpret_cast<test1::Window*>(glfwGetWindowUserPointer(window));
			if (!thisWindow) {
				return;
			}
			if (thisWindow->m_Width != width || thisWindow->m_Height != height) {
				thisWindow->m_Width  = width;
				thisWindow->m_Height = height;
				thisWindow->m_IsResized = true;
			}
			else {
				thisWindow->m_IsResized = false;
			}
			
		}
		static void CursorPosCallback(GLFWwindow* window, double x, double y) {
			if (!window) {
				return;
			}
			test1::Window* thisWindow = reinterpret_cast<test1::Window*>(glfwGetWindowUserPointer(window));
			if (!thisWindow) {
				return;
			}
			thisWindow->m_DeltCursorPos = glm::dvec2(x, y) - thisWindow->m_CurrCursorPos;
			thisWindow->m_PrevCursorPos = thisWindow->m_CurrCursorPos;
			thisWindow->m_CurrCursorPos = glm::dvec2(x, y);
		}
		static void ScrollPosCallback(GLFWwindow* window, double xOffset, double yOffset)
		{
			if (!window) {
				return;
			}
			test1::Window* thisWindow = reinterpret_cast<test1::Window*>(glfwGetWindowUserPointer(window));
			if (!thisWindow) {
				return;
			}
			thisWindow->m_DeltScrollPos = glm::dvec2(xOffset, yOffset);
			thisWindow->m_PrevScrollPos = thisWindow->m_CurrScrollPos;
			thisWindow->m_CurrScrollPos+= thisWindow->m_DeltScrollPos;
		}
		GLFWwindow*	m_Window   = nullptr;
		int			m_Width    = 0;
		int			m_Height   = 0;
		bool        m_IsResized= false;
		glm::dvec2	m_CurrCursorPos   = { 0.0,0.0 };
		glm::dvec2	m_PrevCursorPos   = { 0.0,0.0 };
		glm::dvec2	m_DeltCursorPos   = { 0.0,0.0 };
		glm::dvec2	m_CurrScrollPos   = { 0.0,0.0 };
		glm::dvec2	m_PrevScrollPos   = { 0.0,0.0 };
		glm::dvec2	m_DeltScrollPos   = { 0.0,0.0 };
	};
	class  Timer {
	public:
		void Start() noexcept;
		void Update()noexcept;
		auto GetCurrTime() const noexcept -> double { return m_CurrTime; }
		auto GetPrevTime() const noexcept -> double { return m_PrevTime; }
		auto GetDeltTime() const noexcept -> double { return m_DeltTime; }
	private:
		double m_CurrTime = 0.0f;
		double m_PrevTime = 0.0f;
		double m_DeltTime = 0.0f;
	};
	template<GLenum ShaderType>
	class  Shader {
	public:
		Shader()noexcept {}
		Shader(Shader&& shader)noexcept :m_ID{ shader.m_ID } {
			shader.m_ID = 0;
		}
		bool Initialize(GLFWwindow* baseContext = glfwGetCurrentContext());
		void Terminate();
		void SetSourceFromData(const std::string_view& data);
		void SetSourceFromPath(const std::string_view& path);
		auto Compile() const-> bool;
		auto GetID()const -> GLuint { return m_ID; }
	private:
		GLuint m_ID = 0;
	};
	class  GraphicsProgram {
	public:
		GraphicsProgram()noexcept {}
		GraphicsProgram(GraphicsProgram&& program)noexcept :m_ID{ program.m_ID } {
			program.m_ID = 0;
		}
		bool Initialize(GLFWwindow* baseContext = glfwGetCurrentContext());
		void Terminate();
		template<GLenum ShaderType>
		void AttachShader(const Shader<ShaderType>& shader);
		auto Link() const -> bool;
		void Use()const ;
		auto GetID()const -> GLuint { return m_ID; }
		auto GetUniformLocation(const std::string& name)->GLint;
	private:
		GLuint                                 m_ID = 0;
		std::unordered_map<std::string, GLint> m_UniformLocs = {};
	};
	class  Mesh {
	public:
		struct Vertex {
			glm::vec3 position;
			glm::vec2 texCoord;
		};
		bool Initialize(GLFWwindow* baseContext = glfwGetCurrentContext()) {
			if (!baseContext) {
				return false;
			}
			glGenBuffers(1, &m_VBO);
			glGenBuffers(1, &m_IBO);
			glGenVertexArrays(1, &m_VAO);
			return true;
		}
		void Load(const std::vector<Mesh::Vertex>& vertices,
				  const std::vector<std::uint32_t>& indices) {
			{
				glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(Mesh::Vertex) * std::size(vertices), std::data(vertices), GL_STATIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(std::uint32_t) * std::size(indices), std::data(indices), GL_STATIC_DRAW);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			}
			{
				glBindVertexArray(m_VAO);
				glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), &static_cast<Mesh::Vertex*>(0)->position[0]);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), &static_cast<Mesh::Vertex*>(0)->texCoord[0]);
				glEnableVertexAttribArray(1);
				glBindVertexArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			}
			m_IndCnt = std::size(indices);
		}
		void Draw(GLenum mode = GL_TRIANGLES) {
			glBindVertexArray(m_VAO);
			glDrawElements(mode, m_IndCnt, GL_UNSIGNED_INT, 0);
		}
		void Terminate() {
			GLuint buffers[] = { m_VBO,m_IBO };
			glDeleteBuffers(2, buffers);
			glDeleteVertexArrays(1, &m_VAO);
			m_VAO = m_VBO = m_IBO = 0;
		}
		static Mesh RectMesh() {
			Mesh mesh = {};
			if (!mesh.Initialize()) {
				throw std::runtime_error("Failed To Create Rect Mesh");
			}
			std::vector<test1::Mesh::Vertex> vertices = {
			test1::Mesh::Vertex{{-1.0f,-1.0f,0.0f},{0.0f,0.0f}},
			test1::Mesh::Vertex{{ 1.0f,-1.0f,0.0f},{1.0f,0.0f}},
			test1::Mesh::Vertex{{ 1.0f, 1.0f,0.0f},{1.0f,1.0f}},
			test1::Mesh::Vertex{{-1.0f, 1.0f,0.0f},{0.0f,1.0f}}
			};
			std::vector<std::uint32_t>       indices = {
				0,1,2,2,3,0
			};
			mesh.Load(vertices, indices);
			return mesh;
		}
		static Mesh CircleMesh(int N) {
			Mesh mesh = {};
			if (!mesh.Initialize()) {
				throw std::runtime_error("Failed To Create Circle Mesh");
			}
			std::vector<test1::Mesh::Vertex> vertices = {};
			std::vector<std::uint32_t>       indices = {};
			vertices.resize(N + 1);
			vertices[0] = test1::Mesh::Vertex{ glm::vec3(0.0f,0.0f,0.0f), glm::vec2(0.5f) };
			for (int i = 0; i < N; ++i) {
				float theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(N);
				float cosT = std::cos(theta);
				float sinT = std::sin(theta);
				glm::vec2 pos = { cosT,sinT };
				vertices[i + 1] = test1::Mesh::Vertex{ glm::vec3(pos,0.0f), 0.5f * pos + glm::vec2(0.5f) };
			}
			indices.resize(3 * N);
			for (int i = 0; i < N; ++i) {
				indices[3 * i + 0] = 0;
				indices[3 * i + 1] = i + 1;
				indices[3 * i + 2] = i + 2;
			}
			indices[3 * N - 1] = 1;
			mesh.Load(vertices, indices);
			return mesh;
		}
	private:
		GLuint m_VAO    = 0;
		GLuint m_VBO    = 0;
		GLuint m_IBO    = 0;
		GLuint m_IndCnt = 0;
	};
	struct LightField {
		struct SurfaceCamera {
			glm::ivec2 st;
			glm:: vec2 xy;
			GLuint     texID;
			int        width;
			int        height;
		};
		std::vector<LightField::SurfaceCamera> surfaceCameras = {};
		glm::ivec2 							   stSizes        = {};
		glm::vec2							   stScales       = {};
		glm::vec2                              maxXY          = {};
		glm::vec2                              minXY          = {};
		float      							   focalLen       = 1.0f;
		float      							   aperture       = 1.0f;
		void Load(const std::string& path) {
			int    imageWidth  = 0;
			int    imageHeight = 0;
			int    imageComp   = 0;
			int    imageSCount = 0;
			int    imageTCount = 0;
			glm::vec2 maxRange = glm::vec2(-FLT_MAX, -FLT_MAX);
			glm::vec2 minRange = glm::vec2( FLT_MAX,  FLT_MAX);
			bool isFirst = true;
			for (const std::filesystem::directory_entry& dir : std::filesystem::directory_iterator(path)) {
				auto filepath = dir.path().string();
				auto filename = dir.path().filename().string();
				int x, y;
				float camX, camY;
				sscanf(filename.c_str(), "out_%d_%d_%f_%f_.png", &x, &y, &camX, &camY);
				imageSCount = std::max(imageSCount, x + 1);
				imageTCount = std::max(imageTCount, y + 1);
				maxRange    = glm::max(maxRange,glm::vec2(camX,camY));
				minRange    = glm::min(minRange,glm::vec2(camX,camY));
			}
			stSizes = glm::ivec2(imageSCount, imageTCount);
			maxXY   = maxRange;
			minXY   = minRange;
			surfaceCameras.resize(stSizes.x * stSizes.y);

			for (const std::filesystem::directory_entry& dir : std::filesystem::directory_iterator(path)) {
				auto filepath = dir.path().string();
				auto filename = dir.path().filename().string();
				int x, y;
				float camX, camY;
				sscanf(filename.c_str(), "out_%d_%d_%f_%f_.png", &x, &y, &camX, &camY);
				auto pixels = stbi_load(filepath.c_str(), &imageWidth, &imageHeight, &imageComp, 4);
				//std::cout << "\r" << "Load: " << filename <<" Width: " << imageWidth << " Height: " << imageHeight << std::string(10, ' ') << std::flush;
				std::cout  << x << "," << y << "," <<  camX << "," << camY << std::endl;
				if (!pixels) std::terminate();
				GLuint tex = 0;
				glGenTextures(1, &tex);
				glBindTexture(GL_TEXTURE_2D, tex);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
				glBindTexture(GL_TEXTURE_2D, 0);
				surfaceCameras[imageSCount * y + x].texID = tex;
				surfaceCameras[imageSCount * y + x].width = imageWidth;
				surfaceCameras[imageSCount * y + x].height= imageHeight;
				surfaceCameras[imageSCount * y + x].st    = { x,y };
				surfaceCameras[imageSCount * y + x].xy    = { camX,camY };
			}

			stScales.x = stScales.y = 0.0625f;
			stScales.x = stScales.y * static_cast<float>(imageWidth) / static_cast<float>(imageHeight);
			focalLen   = 1.0f;
			aperture   = 1.2f;
		}
	};
	class  Context {
	public:
		Context()noexcept {}
		Context(const Context&) = delete;
		bool Initialize() noexcept;
		void Terminate() noexcept;
		auto CreateWindow(int width, int height, const char* title) const -> std::shared_ptr<Window> {
			std::shared_ptr<Window> window = std::shared_ptr<Window>(new Window{});
			if (!window->Initialize(m_Window, width, height, title)) {
				throw std::runtime_error("Failed To Create Window");
			}
			return window;
		}
	private:
		GLFWwindow* m_Window = nullptr;
	};
	class  Application{
	public:
		bool InitContext(){
			m_Context = std::unique_ptr<Context>(new Context());
			if(!this->m_Context->Initialize()){
				return false;
			}
			return true;
		}
		void InitWindow(int width, int height, const char* title){
			m_Window  = m_Context->CreateWindow(width,height,title);
		}
		bool InitPrograms(){
			std::string header     = "#version 450 core\n";
			std::string uv2Color   = "vec3 UV2Color(in vec2 texCoord){ return vec3(texCoord.x,texCoord.y,1.0f-dot(texCoord,vec2(0.5f)));}\n";
			std::string pos2Camera = 
				"uniform vec3  cameraEye;\n"
				"uniform vec3  cameraW;\n"
				"uniform float cameraFocal;\n"
				"vec3 Pos2Camera(in vec3 pos){\n"
				"	vec3  direction = normalize(pos-cameraEye);\n"
				"	float len       = cameraFocal/dot(cameraW,direction);\n"
				"	return cameraEye + len*direction;\n"
				"}\n";
			std::string pos2Screen =
				"uniform vec2  screenEye;\n"
				"uniform float screenFocal;\n"
				"uniform vec2  screenSize;\n"
				"uniform vec2  screenApertures;\n"
				"vec2 Pos2Screen(in vec3 pos){\n"
				"	return (screenEye.xy+(pos.xy-screenEye.xy)*screenFocal/pos.z)/(2.0f*screenSize) + vec2(0.5f);\n"
				"}\n";
			std::string SampleTex =
				"uniform sampler2D tex;\n"
				"vec3 SampleTex(in vec2 uv){\n"
				"	return texture(tex,uv).xyz;\n"
				"}\n";
			std::string vsSource1 =
				"uniform mat4 model;\n"
				"uniform mat4 view;\n"
				"uniform mat4 proj;\n"
				"layout(location = 0) in vec3 vPosition;\n"
				"layout(location = 1) in vec2 vTexCoord;\n"
				"out vec4 fPosition;\n"
				"out vec2 fTexCoord;\n"
				"void main(){\n"
				"	fPosition   = model*vec4(vPosition,1.0f);\n"
				"	gl_Position = proj*view*fPosition;\n"
				"	fTexCoord   = vTexCoord;\n"
				"}\n";
			std::string vsSource2 =
				"layout(location = 0) in vec3 vPosition;\n"
				"layout(location = 1) in vec2 vTexCoord;\n"
				"out vec2 fTexCoord;\n"
				"void main(){\n"
				"	gl_Position = vec4(vPosition,1.0f);\n"
				"	fTexCoord   = vTexCoord;\n"
				"}\n";
			std::string fsSource1 =
				//"uniform vec3 dbgColor;\n"
				"in vec4 fPosition;\n"
				"in vec2 fTexCoord;\n"
				"layout(location = 0) out vec4 fColor;\n"
				"void main(){\n"
				"	vec3 cameraPos = Pos2Camera(fPosition.xyz);\n"
				"	vec2 screenPos = Pos2Screen(cameraPos);\n"
				"	vec2 weights   = pow(max(vec2(1.0f)-abs(fPosition.xy-screenEye.xy)/screenApertures,vec2(0.0f)),vec2(2.0f));\n"
				//"   vec2 weights   = vec2(max(vec2(1.0f)-length(fPosition.xy-screenEye.xy)/screenApertures,vec2(0.0f)));\n"
				"	if(screenPos.x <0.0f || screenPos.x > 1.0f || screenPos.y < 0.0f || screenPos.y  >1.0f){ discard;}\n"
				//"	if(fTexCoord.x <0.01f || fTexCoord.x > 0.99f || fTexCoord.y < 0.01f || fTexCoord.y>0.99f){ fColor = vec4(dbgColor,1.0f); return;}\n"
				"	fColor = vec4(SampleTex(vec2(1.0-screenPos.x,1.0-screenPos.y)),1.0f)*(weights.x+weights.y);\n"
				"}\n";
			std::string fsSource2 =
				"uniform vec3 dbgColor;\n"
				"in vec4 fPosition;\n"
				"in vec2 fTexCoord;\n"
				"layout(location = 0) out vec4 fColor;\n"
				"void main(){\n"
				"	if(fTexCoord.x <0.01f || fTexCoord.x > 0.99f || fTexCoord.y < 0.01f || fTexCoord.y  >0.99f){ fColor = vec4(dbgColor,1.0f); return;}\n"
				"	fColor = vec4(0.0f,0.0f,0.0f,0.0f);\n"
				"}\n";
			std::string fsSource3 = 
				"in vec2 fTexCoord;\n"
				"uniform sampler2D tex;\n"
				"layout(location = 0) out vec4 fColor;\n"
				"void main(){\n"
				"	vec4 color = texture(tex,fTexCoord);\n"
				"	fColor     = vec4(color.xyz/color.w,1.0f);\n"
				"}\n";
			auto vertShader1 = test1::Shader<GL_VERTEX_SHADER>();
			{
				if (!vertShader1.Initialize()) {
					return false;
				}
				vertShader1.SetSourceFromData(header + vsSource1);
				if (!vertShader1.Compile()) {
					return false;
				}
			}
			auto vertShader2 = test1::Shader<GL_VERTEX_SHADER>();
			{
				if (!vertShader2.Initialize()) {
					return false;
				}
				vertShader2.SetSourceFromData(header + vsSource2);
				if (!vertShader2.Compile()) {
					return false;
				}
			}
			auto fragShader1 = test1::Shader<GL_FRAGMENT_SHADER>();
			{
				if (!fragShader1.Initialize()) {
					return false;
				}
				fragShader1.SetSourceFromData(header + uv2Color + pos2Screen + pos2Camera + SampleTex + fsSource1);
				if (!fragShader1.Compile()) {
					return false;
				}
			}
			auto fragShader2 = test1::Shader<GL_FRAGMENT_SHADER>();
			{
				if (!fragShader2.Initialize()) {
					return false;
				}
				fragShader2.SetSourceFromData(header + fsSource2);
				if (!fragShader2.Compile()) {
					return false;
				}
			}
			auto fragShader3 = test1::Shader<GL_FRAGMENT_SHADER>();
			{
				if (!fragShader3.Initialize()) {
					return false;
				}
				fragShader3.SetSourceFromData(header + fsSource3);
				if (!fragShader3.Compile()) {
					return false;
				}
			}
			//RenderPass1 Accum
			{
				if (!m_Program["accum"].Initialize()) {
					return false;
				}
				m_Program["accum"].AttachShader(vertShader1);
				m_Program["accum"].AttachShader(fragShader1);
				if (!m_Program["accum"].Link()) {
					return false;
				}
			}
			//RenderPass2 Line(Visualize)
			{
				if (!m_Program["line"].Initialize()) {
					return false;
				}
				m_Program["line"].AttachShader(vertShader1);
				m_Program["line"].AttachShader(fragShader2);
				if (!m_Program["line"].Link()) {
					return false;
				}
			}
			//RenderPass3 Finish(Visualize)
			{
				if (!m_Program["finish"].Initialize()) {
					return false;
				}
				m_Program["finish"].AttachShader(vertShader2);
				m_Program["finish"].AttachShader(fragShader3);
				if (! m_Program["finish"].Link()) {
					return false;
				}
			}
			vertShader1.Terminate();
			vertShader2.Terminate();
			fragShader1.Terminate();
			fragShader2.Terminate();
			fragShader3.Terminate();
		}
		void InitTimer(){
			m_Timer.Start();
		}
		void InitMeshes(){
			m_RectMesh = Mesh::RectMesh();
			m_CircMesh = Mesh::CircleMesh(25);
		}
		void LoadLightField(const std::string& path){
		  	m_LightField.Load(path);
		}
		void InitFramebuffer(){
			int fbWidth, fbHeight;
			glfwGetFramebufferSize(m_Window->GetHandle(), &fbWidth, &fbHeight);
		  	glGenTextures(1, &m_RenderTexture);
		  	glBindTexture(GL_TEXTURE_2D,  m_RenderTexture);
		   	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, fbWidth, fbHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glGenFramebuffers(1, &m_Framebuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_RenderTexture, 0);
			GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, DrawBuffers);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				throw std::runtime_error("Failed To Init FrameBuffer!");
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		void InitCamera(){
			m_CameraController    = cvlib::CameraController(glm::vec3(0.0f,0.0f,-1.0f));
		}
		void UpdateState(){
			if (m_Window->IsPressedKey(GLFW_KEY_R)) {
				m_CameraController = cvlib::CameraController(glm::vec3(0.0f, 0.0f, -1.0f));
				m_Update     = true;
			}
			if (m_Window->IsPressedKey(GLFW_KEY_W)) {
				m_CameraController.ProcessKeyboard(cvlib::CameraMovement::eForward, m_Timer.GetDeltTime());
				m_Update     = true;
			}
			if (m_Window->IsPressedKey(GLFW_KEY_S)) {
				m_CameraController.ProcessKeyboard(cvlib::CameraMovement::eBackward, m_Timer.GetDeltTime());
				m_Update = true;
			}
			if (m_Window->IsPressedKey(GLFW_KEY_A)) {
				m_CameraController.ProcessKeyboard(cvlib::CameraMovement::eLeft, m_Timer.GetDeltTime());
				m_Update = true;
			}
			if (m_Window->IsPressedKey(GLFW_KEY_D)) {
				m_CameraController.ProcessKeyboard(cvlib::CameraMovement::eRight, m_Timer.GetDeltTime());
				m_Update = true;
			}
			if (m_Window->IsPressedKey(GLFW_KEY_LEFT)) {
				m_CameraController.ProcessKeyboard(cvlib::CameraMovement::eLeft, m_Timer.GetDeltTime());
				m_Update = true;
			}
			if (m_Window->IsPressedKey(GLFW_KEY_RIGHT)) {
				m_CameraController.ProcessKeyboard(cvlib::CameraMovement::eRight, m_Timer.GetDeltTime());
				m_Update = true;
			}
			if (m_Window->IsPressedKey(GLFW_KEY_UP)) {
				m_CameraController.ProcessKeyboard(cvlib::CameraMovement::eUp, m_Timer.GetDeltTime());
				m_Update = true;
			}
			if (m_Window->IsPressedKey(GLFW_KEY_DOWN)) {
				m_CameraController.ProcessKeyboard(cvlib::CameraMovement::eDown, m_Timer.GetDeltTime());
				m_Update = true;
			}
			if (m_Window->IsPressedMouseButton(GLFW_MOUSE_BUTTON_LEFT)) {
				m_CameraController.ProcessMouseMovement(-m_Window->GetDeltCursorPos().x, m_Window->GetDeltCursorPos().y);
				m_Update = true;
			}
			m_Timer.Update();
		}
		void MainLoop(){
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);
			while (!m_Window->ShouldClose()) {
				m_Window->PollEvents();
				const float focal  = 0.05f * m_Window->GetCurrScrollPos().y;
				const float aspect = m_Window->GetAspect();
				const float fovY   = glm::radians(60.0f);
				const auto mainCamera = m_CameraController.GetCamera(fovY, aspect, focal);
				const auto eye   = mainCamera.getEye();
				const auto at    = mainCamera.getLookAt();
				const auto vup   = mainCamera.getVup();
				const auto w     = mainCamera.getDirection();
				const auto screenSize      = m_LightField.stScales * glm::vec2(m_LightField.stSizes);
				const auto screenFocal     = m_LightField.focalLen;
				const auto screenApertures = m_LightField.stScales * m_LightField.aperture;
				if (m_Window->IsResized()) {
					glDeleteTextures(1, &m_RenderTexture);
					glDeleteFramebuffers(1, &m_Framebuffer);
					this->InitFramebuffer();
				}
				{
					glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
					glClear(GL_COLOR_BUFFER_BIT);
					glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
					glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
					m_Program["accum"].Use();
					{
						for (auto& surfaceCamera : m_LightField.surfaceCameras)
						{
							//auto& camera = lightField.surfaceCameras[lightField.stSizes.y / 2 * lightField.stSizes.x + lightField.stSizes.x / 2];
							//auto st           = glm::vec2(surfaceCamera.st) - (glm::vec2(lightField.stSizes - glm::ivec2(1))) / 2.0f;
							glm::vec2 screenEye = screenSize * (surfaceCamera.xy - glm::vec2(m_LightField.maxXY + m_LightField.minXY) / 2.0f) / glm::vec2(m_LightField.maxXY - m_LightField.minXY);
							//glm::vec2 screenEye = lightField.stScales * st;
							auto modelMatrix = glm::translate(glm::identity<glm::mat4>(), glm::vec3(screenEye, 0.0f)) * glm::scale(glm::identity<glm::mat4>(), glm::vec3(screenApertures, 1.0f));
							auto viewMatrix = mainCamera.getViewMatrix();
							auto projMatrix = mainCamera.getProjMatrix();
							glActiveTexture(GL_TEXTURE0);
							glBindTexture(GL_TEXTURE_2D, surfaceCamera.texID);
							glUniform1i( m_Program["accum"].GetUniformLocation("tex"), 0);
							glUniform2fv(m_Program["accum"].GetUniformLocation("screenEye"), 1, &screenEye[0]);
							glUniform2fv(m_Program["accum"].GetUniformLocation("screenSize"), 1, &screenSize[0]);
							glUniform2fv( m_Program["accum"].GetUniformLocation("screenApertures"),1,&screenApertures[0]);
							glUniform1fv(m_Program["accum"].GetUniformLocation("screenFocal"), 1, &m_LightField.focalLen);
							glUniform3fv(m_Program["accum"].GetUniformLocation("cameraEye"), 1, &eye[0]);
							glUniform3fv(m_Program["accum"].GetUniformLocation("cameraW"), 1, &w[0]);
							glUniform1fv(m_Program["accum"].GetUniformLocation("cameraFocal"), 1, &focal);
							//glUniform3f( mainProgram.GetUniformLocation("dbgColor"), 1.0f, 0.0f, 0.0f);
							glUniformMatrix4fv(m_Program["accum"].GetUniformLocation("model"), 1, GL_FALSE, &modelMatrix[0][0]);
							glUniformMatrix4fv(m_Program["accum"].GetUniformLocation("view"), 1, GL_FALSE, &viewMatrix[0][0]);
							glUniformMatrix4fv(m_Program["accum"].GetUniformLocation("proj"), 1, GL_FALSE, &projMatrix[0][0]);
							m_RectMesh.Draw();
						}
					}
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
				}
				{
					glClear(GL_COLOR_BUFFER_BIT);
					glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
					glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
#if 0
					{
						m_Program["line"].Use();
						{
							auto modelMatrix = glm::translate(glm::scale(glm::identity<glm::mat4>(), glm::vec3(screenSize / 2.0f, 1.0f)), glm::vec3(0.0f, 0.0f, 0.0f));
							auto viewMatrix = mainCamera.getViewMatrix();
							auto projMatrix = mainCamera.getProjMatrix();
							glUniform3f(m_Program["line"].GetUniformLocation("dbgColor"), 0.0f, 0.0f, 1.0f);
							glUniformMatrix4fv(m_Program["line"].GetUniformLocation("model"), 1, GL_FALSE, &modelMatrix[0][0]);
							glUniformMatrix4fv(m_Program["line"].GetUniformLocation("view"), 1, GL_FALSE, &viewMatrix[0][0]);
							glUniformMatrix4fv(m_Program["line"].GetUniformLocation("proj"), 1, GL_FALSE, &projMatrix[0][0]);
							m_RectMesh.Draw();
							modelMatrix = glm::translate(glm::scale(glm::identity<glm::mat4>(), glm::vec3(screenSize / 2.0f, 1.0f)), glm::vec3(0.0f, 0.0f, m_LightField.focalLen));
							glUniformMatrix4fv(m_Program["line"].GetUniformLocation("model"), 1, GL_FALSE, &modelMatrix[0][0]);
							m_RectMesh.Draw();
						}
					}
#endif
					m_Program["finish"].Use();
					{
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, m_RenderTexture);
						glUniform1i(m_Program["finish"].GetUniformLocation("tex"), 0);
						m_RectMesh.Draw();
					}
				}
				m_Window->SwapBuffers();
				this->UpdateState();
				std::cout << "\r" << "focal: " << focal << " eye: " << glm::to_string(eye) << " at: " << glm::to_string(at) << " focal: " << focal << std::string(5, ' ') << std::flush;
			}
		}
	private:
		std::unique_ptr<Context> 						m_Context 		   = nullptr;
		std::shared_ptr<Window>  						m_Window  		   = nullptr;
		std::unordered_map<std::string,GraphicsProgram> m_Program 		   = {};
		Timer					                        m_Timer   		   = {};
		cvlib::CameraController                         m_CameraController = {};
		Mesh                                            m_RectMesh         = {};
		Mesh                                            m_CircMesh		   = {};
		LightField                                      m_LightField       = {};
		GLuint                                          m_Framebuffer      = 0;
		GLuint                                          m_RenderTexture    = 0;
		bool                                            m_Update  		   = false;
	};
	void   Timer::Start() noexcept
	{
		glfwSetTime(0.0f);
		m_DeltTime = 0.0f;
		m_PrevTime = 0.0f;
		m_CurrTime = 0.0f;
	}
	void   Timer::Update() noexcept
	{
		double time = glfwGetTime();
		m_DeltTime = time - m_CurrTime;
		m_PrevTime = m_CurrTime;
		m_CurrTime = time;
	}
	bool   Context::Initialize() noexcept
	{
		if (!glfwInit()) { return false; }
		glfwWindowHint(GLFW_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		m_Window = glfwCreateWindow(1, 1, "Context", nullptr, nullptr);
		if (!m_Window) { return false; }
		glfwMakeContextCurrent(m_Window);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			return false;
		}
		return true;
	}
	void   Context::Terminate() noexcept
	{
		if (m_Window) {
			glfwDestroyWindow(m_Window);
			m_Window = nullptr;
		}
		glfwTerminate();
	}
	bool   Window::Initialize(GLFWwindow* baseContext, int width, int height, const char* title)
	{
		if (!baseContext) {
			return false;
		}
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, baseContext);
		if (!window) {
			return false;
		}
		m_Window  = window;
		m_Width   = width;
		m_Height  = height;
		m_CurrCursorPos = { 0.0,0.0 };
		m_PrevCursorPos = { 0.0,0.0 };
		m_DeltCursorPos = { 0.0,0.0 };
		glfwMakeContextCurrent(window);
		glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));
	    glfwSetCursorPosCallback( window, CursorPosCallback);
		glfwSetScrollCallback(    window, ScrollPosCallback);
		glfwSetWindowSizeCallback(window, WindowSizeCallback);
		return true;
	}
	void   Window::Terminate() {
		if (m_Window) {
			glfwDestroyWindow(m_Window);
			m_Window = nullptr;
		}
		m_Width = 0;
		m_Height = 0;
		glfwTerminate();
	}
	template<GLenum ShaderType>
	bool Shader<ShaderType>::Initialize(GLFWwindow* baseContext)
	 {
		 if (!baseContext) { return false;  }
		 m_ID = glCreateShader(ShaderType);
		 return true;
	 }
	template<GLenum ShaderType>
	void Shader<ShaderType>::Terminate()
	 {
		 glDeleteShader(m_ID);
		 m_ID = 0;
	 }
	template<GLenum ShaderType>
	void Shader<ShaderType>::SetSourceFromData(const std::string_view& data)
	 {
		 const char* const ptr = data.data();
		 glShaderSource(m_ID, 1, &ptr, nullptr);
	 }
	template<GLenum ShaderType>
	void Shader<ShaderType>::SetSourceFromPath(const std::string_view& path)
	 {
		 std::ifstream file(path);
		 if (file.fail()) {
			 throw std::runtime_error("Failed To Load Shader Source!");
		 }
		 std::string source((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
		 this->SetSourceFromData(source);
	 }
	template<GLenum ShaderType>
	auto Shader<ShaderType>::Compile() const -> bool
	 {
		 GLint status;
		 glCompileShader(m_ID);
		 glGetShaderiv(m_ID, GL_COMPILE_STATUS, &status);
		 {
			 GLint logSiz = 0;
			 glGetShaderiv(m_ID, GL_INFO_LOG_LENGTH, &logSiz);
			 std::string log;
			 log.resize(logSiz);
			 GLsizei logLen = 0;
			 glGetShaderInfoLog(m_ID, log.size(), &logLen, log.data());
			 log.resize(logLen);
			 if (logLen > 0) {
				 std::cout << log << "\n";
			 }
		 }
		 return status==GL_TRUE;
	 }
	bool GraphicsProgram::Initialize(GLFWwindow* baseContext)
	 {
		 if (!baseContext) { return false; }
		 m_ID = glCreateProgram();
		 return true;
	 }
	void GraphicsProgram::Terminate()
	 {
		 glDeleteProgram(m_ID);
		 m_ID = 0;
	 }
	auto GraphicsProgram::Link() const -> bool
	 {
		 GLint status;
		 glLinkProgram(m_ID);
		 glGetProgramiv(m_ID, GL_LINK_STATUS, &status);
		 {
			 GLint logSiz = 0;
			 glGetProgramiv(m_ID, GL_INFO_LOG_LENGTH, &logSiz);
			 std::string log;
			 log.resize(logSiz);
			 GLsizei logLen = 0;
			 glGetProgramInfoLog(m_ID, log.size(), &logLen, log.data());
			 log.resize(logLen);
			 if (logLen > 0) {
				 std::cout << log << "\n";
			 }
		 }
		 return status == GL_TRUE;
	 }
	void GraphicsProgram::Use() const
	 {
		 glUseProgram(m_ID);
	 }
	auto GraphicsProgram::GetUniformLocation(const std::string& name) -> GLint
	 {
		 if (m_UniformLocs.count(name) == 0) {
			 GLint loc = glGetUniformLocation(m_ID, name.data());
			 if (loc<0) {
				 return 0;
			 }
			 m_UniformLocs[name] = loc;
			 return loc;
		 }
		 else {
			 return m_UniformLocs.at(name);
		 }
	 }
	template<GLenum ShaderType>
	void GraphicsProgram::AttachShader(const Shader<ShaderType>& shader)
	 {
		 glAttachShader(m_ID, shader.GetID());
	 }
}
int main(){
	test1::Application app = {};
	app.InitContext();
	app.InitWindow(640,480,"title");
	app.InitPrograms();
	app.InitFramebuffer();
	app.LoadLightField(CV_FINAL_DATA_DIR"/taroto/rectified/");
	app.InitMeshes();
	app.InitCamera();
	app.InitTimer();
	app.MainLoop();
}