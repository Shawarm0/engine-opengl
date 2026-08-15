#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

int main(void) {

	// OpenGL context initialisation failed.
	if (!glfwInit()) {
		fprintf(stderr, "glfwInit failed\n");
		return 1;
	}


	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif


	// Create window pointer.
	GLFWwindow *win = glfwCreateWindow(800, 800, "engine-opengl", NULL, NULL);
	if (!win) {
		fprintf(stderr, "window creation failed\n");
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(win);


	// Load GLAD.
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		fprintf(stderr, "GLAD failed\n");
		glfwTerminate();
		return 1;
	}

	glfwSwapinterval(1); // vsync


	// Start visual loop.
	while (!glfwWindowShouldClose(win)) {







	}


	return 0;
}
