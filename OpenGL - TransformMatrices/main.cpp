//ImGUI Import
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

//Core Import
#include <glad/glad.h>
#include <glfw/glfw3.h>

//Auxiliar Libraries
#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//Internal Includes
#include "auxiliar_funcs.hpp"

int main(void)
{
	/* Objeto que Representa a Janela da Aplicação */
	GLFWwindow* window;
	if (int res = setup_app(window) != 0)
		return res;

	setup_shaders();
	create_nyan_square();

	/* Matrix do exemplo */
	glm::mat4 transfMat = glm::mat4(1);
	while (!glfwWindowShouldClose(window))
	{
		init_frame(transfMat);
		//==============================
		//===MODELS RENDERING GO HERE===
		//==============================
		
		/* Passando matrix de transformação para os shaders */
		GLint loc = glGetUniformLocation(programHandle2D, "transform");
		glUniformMatrix4fv(loc, 1, GL_TRUE, reinterpret_cast<float*>(&transfMat[0][0]));
		loc = glGetUniformLocation(programHandle3D, "transform");
		glUniformMatrix4fv(loc, 1, GL_TRUE, reinterpret_cast<float*>(&transfMat[0][0]));

		/* Desenhar o Nyan Cat */
		draw_nyan_square();

		//==============================
		//======BUT NOT AFTER HERE======
		//==============================
		end_frame(window);
	}

	/* Limpando valores */
	free_nyan_square();
	free_program();
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}