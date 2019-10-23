#ifndef AUXILIAR_FUNCS
#define AUXILIAR_FUNCS
#include <fstream>

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void resize_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

static void check_compile_errors(GLuint shader)
{
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		GLchar* errorLog = new GLchar[maxLength];
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
		const char* shaderId = new char[2]{ '0' + (char)shader, '\0' };
		fprintf(stderr, "Shader %i Compilation Error: %s\n", shaderId, errorLog);

		// Exit with failure.
		glDeleteShader(shader); // Don't leak the shader.
	}
}

static void check_link_errors(GLuint program, GLuint vertexShader, GLuint fragmentShader)
{
	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, (int*)& isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		GLchar* errorLog = new GLchar[maxLength];
		glGetProgramInfoLog(program, maxLength, &maxLength, &errorLog[0]);
		const char programId = '0' + program;
		fprintf(stderr, "Program %i Linking Error: %s\n", programId, errorLog);

		glDeleteProgram(program);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}
}

const char* load_shader(const char* filePath) {
	std::ifstream in(filePath, std::ifstream::binary);
	if (!in.is_open()) {
		fprintf(stderr, "Arquivo nao encontrado\n");
		return nullptr;
	}
	in.seekg(0, std::ifstream::end);
	const int length = in.tellg();
	in.seekg(0, std::ifstream::beg);

	// allocate memory:
	char* buffer = new char[length + 1];

	// read data as a block:
	in.read(buffer, length);
	buffer[length] = '\0';
	return buffer;
}

static int setup_app(GLFWwindow*& window, const char* appName = "App", int width = 1024, int height = 768)
{
	/* Inicializar a Biblioteca */
	fprintf(stdout, "Initializing GLFW... ");
	if (!glfwInit()) {

		fprintf(stderr, "\nFailed to initialize GLFW!");
		getchar();
		return -1;
	}
	fprintf(stdout, "Done!\n");


	/* Configurar Callback de Erro */
	glfwSetErrorCallback(error_callback);

	/* Criar uma Janela na GLFW */
	fprintf(stdout, "Creating Window... ");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(width, height, appName, NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		fprintf(stderr, "\nFailed to create Window!");
		getchar();
		return -1;
	}
	fprintf(stdout, "Done!\n");

	/* Colocar a Janela em Contexto */
	glfwMakeContextCurrent(window);

	/* Iniciar GLAD */
	fprintf(stdout, "Initializing GLAD... ");
	if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0)
	{
		fprintf(stderr, "\nFailed to initialize GLAD!");
		getchar();
		return -1;
	}
	fprintf(stdout, "Done!\n");

	/* Definir Callback das teclas */
	glfwSetKeyCallback(window, key_callback);

	/* Definir Callback de resize */
	glfwSetFramebufferSizeCallback(window, resize_callback);

	/* V-Sync */
	glfwSwapInterval(1);

	/* Definir Cor de Background */
	glClearColor(0.6, 0.6, 1.0, 1.0);

	/* Configurando transparência */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* Flip Y das imagens carregadas com stb_image */
	stbi_set_flip_vertically_on_load(true);

	/* ImGUI Setup */
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 150");

	/* Return success code */
	return 0;
}

static GLuint vertexShaderHandle;
static GLuint fragmentShaderHandle;
static GLuint programHandle2D;
static GLuint programHandle3D;
static void setup_shaders()
{
	//Frag with texture
	const char* fragmentShaderText = load_shader("data/shaders/shader.frag");
	fragmentShaderHandle = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderHandle, 1, &fragmentShaderText, NULL);
	glCompileShader(fragmentShaderHandle);
	check_compile_errors(fragmentShaderHandle);
	delete fragmentShaderText;

	//2D transform
	const char* vertexShaderText = load_shader("data/shaders/shader2D.vert");
	vertexShaderHandle = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderHandle, 1, &vertexShaderText, NULL);
	glCompileShader(vertexShaderHandle);
	check_compile_errors(vertexShaderHandle);
	delete vertexShaderText;

	//2D program
	programHandle2D = glCreateProgram();
	glAttachShader(programHandle2D, vertexShaderHandle);
	glAttachShader(programHandle2D, fragmentShaderHandle);
	glLinkProgram(programHandle2D);
	check_link_errors(programHandle2D, vertexShaderHandle, fragmentShaderHandle);

	//Cleanup before loading 3D
	glDeleteShader(vertexShaderHandle);

	//3D transform
	vertexShaderText = load_shader("data/shaders/shader3D.vert");
	vertexShaderHandle = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderHandle, 1, &vertexShaderText, NULL);
	glCompileShader(vertexShaderHandle);
	check_compile_errors(vertexShaderHandle);
	delete vertexShaderText;

	//3D program
	programHandle3D = glCreateProgram();
	glAttachShader(programHandle3D, vertexShaderHandle);
	glAttachShader(programHandle3D, fragmentShaderHandle);
	glLinkProgram(programHandle3D);
	check_link_errors(programHandle3D, vertexShaderHandle, fragmentShaderHandle);

	//Shaders cleanup
	glDeleteShader(vertexShaderHandle);
	glDeleteShader(fragmentShaderHandle);
}

static GLuint squareVAO;
static GLuint squareVBO;
static GLuint squareEBO;
static GLuint squareTEX;
static void create_nyan_square()
{
	/* Square data */
	float squareData[] = {
		//	  X		  Y		  Z		  U		  V
			-0.5f,	-0.5f,	+0.0f,	+0.0f,	+0.0f,	//v0 <= Baixo-esquerda
			-0.5f,	+0.5f,	+0.0f,	+0.0f,	+1.0f,	//v1 <= Cima-esquerda
			+0.5f,	+0.5f,	+0.0f,	+1.0f,	+1.0f,	//v2 <= Cima-direita
			+0.5f,	-0.5f,	+0.0f,	+1.0f,	+0.0f	//v3 <= Baixo-direita
	};
	unsigned int squareIds[] = {
		//	Triângulos no sentido anti-horário
			0,	2,	1,	//tri0 (v0, v2, v1)
			0,	3,	2	//tri1 (v0, v3, v2)
	};

	/* Configurando VAO, VBO & EBO*/
	glGenVertexArrays(1, &squareVAO);
	glBindVertexArray(squareVAO);

	glGenBuffers(1, &squareVBO);
	glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(squareData), squareData, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0								/* attribute location (tem que ser igual ao do shader) */,
		3								/* tamanho do dado: 1 vertice = XYZ = 3 floats */,
		GL_FLOAT						/* tipo de dado */,
		GL_FALSE						/* não normalizado */,
		sizeof(GLfloat) * 5				/* tamanho do bloco de dados (por vértice) */,
		(void*)(sizeof(GLfloat) * 0)	/* espaçamento a partir do inicio do bloco de dados */
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1								/* attribute location (tem que ser igual ao do shader) */,
		2								/* tamanho do dado: 1 cor = UV = 2 floats */,
		GL_FLOAT						/* tipo de dado */,
		GL_FALSE						/* não normalizado */,
		sizeof(GLfloat) * 5				/* tamanho do bloco de dados (por vértice) */,
		(void*)(sizeof(GLfloat) * 3)	/* espaçamento a partir do inicio do bloco de dados */
	);

	glGenBuffers(1, &squareEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, squareEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(squareIds), squareIds, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	/* Image data */
	int imgWidth, imgHeight, channels;
	unsigned char* imgData;
	if ((imgData = stbi_load("data/textures/nyan.png", &imgWidth, &imgHeight, &channels, 0)) == nullptr)
	{
		fprintf(stderr, "Error loading image: %s\n", "nyan.png");
		return;
	}
	/* Configurando textura */
	glGenTextures(1, &squareTEX);
	glBindTexture(GL_TEXTURE_2D, squareTEX);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(
		GL_TEXTURE_2D		/* Tipo de Textura */,
		0					/* MipMap level */,
		GL_RGBA				/* Formato Interno da Imagem em GPU*/,
		imgWidth			/* Largura da Imagem */,
		imgHeight			/* Altura da Imagem */,
		0					/* Borda (tem que ser 0) */,
		GL_RGBA				/* Formato do array (RGB para JPEG) */,
		GL_UNSIGNED_BYTE	/* Tipo de dado para cada pixel no array */,
		imgData				/* Ponteiro do array */
	);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(imgData);
	glBindTexture(GL_TEXTURE_2D, 0);
}

static int dimension = 2;
static int matMode = 2;
static void init_frame(glm::mat4& mat)
{
	glfwPollEvents();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	if (ImGui::Begin("Transform Matrix", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::MenuItem("2D (2x2)")) { dimension = 2; matMode = 2; }
			if (ImGui::MenuItem("2D (3x3)")) { dimension = 2; matMode = 3; }
			if (ImGui::MenuItem("3D (3x3)")) { dimension = 3; matMode = 3; }
			if (ImGui::MenuItem("3D (4x4)")) { dimension = 3; matMode = 4; }
			ImGui::EndMenuBar();
		}
		if (ImGui::Button("RESET"))
		{
			mat = glm::mat4(1);
		}
		ImGui::SameLine();
		if (dimension == 2)
		{
			ImGui::Text("2D Active");
		}
		else
		{
			ImGui::Text("3D Active");
		}
		if (matMode == 2) {
			ImGui::DragFloat2("##0", reinterpret_cast<float*>(&mat[0][0]), 0.01f);
			ImGui::DragFloat2("##1", reinterpret_cast<float*>(&mat[1][0]), 0.01f);
		}
		else if (matMode == 3) {
			ImGui::DragFloat3("##0", reinterpret_cast<float*>(&mat[0][0]), 0.01f);
			ImGui::DragFloat3("##1", reinterpret_cast<float*>(&mat[1][0]), 0.01f);
			ImGui::DragFloat3("##2", reinterpret_cast<float*>(&mat[2][0]), 0.01f);
		}
		else if (matMode == 4) {
			ImGui::DragFloat4("##0", reinterpret_cast<float*>(&mat[0][0]), 0.01f);
			ImGui::DragFloat4("##1", reinterpret_cast<float*>(&mat[1][0]), 0.01f);
			ImGui::DragFloat4("##2", reinterpret_cast<float*>(&mat[2][0]), 0.01f);
			ImGui::DragFloat4("##3", reinterpret_cast<float*>(&mat[3][0]), 0.01f);
		}
	}
	ImGui::End();
	ImGui::Render();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

static void end_frame(GLFWwindow* window)
{
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(window);
}

static void draw_nyan_square()
{
	glUseProgram(dimension == 3 ? programHandle3D : programHandle2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, squareTEX);
	glBindVertexArray(squareVAO);
	glDrawElements(
		GL_TRIANGLES,		// modo de desenho
		6,					// qtd de índices (2 tris = 6 verts)
		GL_UNSIGNED_INT,	// tipo de índice
		(void*)0			// offset
	);
	glBindVertexArray(0);
}

static void free_program()
{
	glDeleteProgram(programHandle2D);
}

static void free_nyan_square()
{
	glDeleteBuffers(1, &squareVAO);
	glDeleteBuffers(1, &squareVBO);
	glDeleteBuffers(1, &squareEBO);
	glDeleteTextures(1, &squareTEX);
}

#endif