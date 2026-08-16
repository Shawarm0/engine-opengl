#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <math.h>
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
	if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
		fprintf(stderr, "GLAD failed\n");
		glfwTerminate();
		return 1;
	}


	glClearColor(0.8f, 0.2f, 0.2f, 1.0f);
	glfwSwapInterval(1); // vsync


	// Start visual loop.
	while (!glfwWindowShouldClose(win)) {
		double mx = 0.0f;
		double my = 0.0f;

		glfwPollEvents();

		int w, h;
		glfwGetFramebufferSize(win, &w, &h);
		glViewport(0, 0, w, h);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwGetCursorPos(win, &mx, &my);

		printf("%f %f\n", mx, my);

		double time = glfwGetTime();
		glClearColor(0.8*sin(time), 0.2*sin(time), 0.2*sin(time), 1.0f);

		glfwSwapBuffers(win);
	}



	glfwTerminate();
	return 0;
}
