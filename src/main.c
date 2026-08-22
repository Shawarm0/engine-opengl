#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include "vec2.h"
#include "body.h"

#define WIN_W            900
#define WIN_H            900
#define CIRCLE_SEGMENTS  48

#define PLANET_MASS      400.0      /* with G = 10000.0 -> GM = 4e6 */
#define RADIUS_K         0.5        /* radius = RADIUS_K * sqrt(mass) */
#define DRAG_TO_VEL      1.5        /* world units dragged -> velocity */
#define DT               (1.0/60.0) /* fixed physics step, in seconds */
#define SUBSTEPS         4
#define MAX_FRAME_TIME   0.25       /* clamp, prevents the spiral of death */

/* ---------------------------------------------------------------- state */

typedef struct {
    BodyArray sim;
    Vec2      cam_center;
    double    cam_half;      /* half the visible HEIGHT, in world units */
    int       paused;
    int       dragging;
    Vec2      drag_start;    /* world coords, where the body will spawn */
    Vec2      drag_now;      /* world coords, current cursor */
} App;

/* ---------------------------------------------------------------- shaders */

static const char *VS_SRC =
    "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "uniform vec2  uCenter;\n"
    "uniform float uRadius;\n"
    "uniform vec2  uCamCenter;\n"
    "uniform vec2  uCamHalf;\n"
    "void main() {\n"
    "    vec2 world = uCenter + aPos * uRadius;\n"
    "    vec2 clip  = (world - uCamCenter) / uCamHalf;\n"
    "    gl_Position = vec4(clip, 0.0, 1.0);\n"
    "}\n";

static const char *FS_SRC =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec3 uColor;\n"
    "void main() { FragColor = vec4(uColor, 1.0); }\n";

static GLuint compile_shader(GLenum type, const char *src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, sizeof(log), NULL, log);
        fprintf(stderr, "shader compile failed:\n%s\n", log);
    }
    return s;
}

static GLuint make_program(const char *vs_src, const char *fs_src) {
    GLuint vs = compile_shader(GL_VERTEX_SHADER,   vs_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fs_src);
    GLuint p  = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(p, sizeof(log), NULL, log);
        fprintf(stderr, "program link failed:\n%s\n", log);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return p;
}

/* ---------------------------------------------------------------- geometry */

static GLuint circle_vao, circle_vbo;
static GLuint line_vao,   line_vbo;

static void geometry_init(void) {
    /* unit circle as a triangle fan: center, then rim, last point == first */
    float verts[(CIRCLE_SEGMENTS + 2) * 2];
    verts[0] = 0.0f;
    verts[1] = 0.0f;
    for (int i = 0; i <= CIRCLE_SEGMENTS; i++) {
        float a = (float)i / CIRCLE_SEGMENTS * 2.0f * 3.14159265f;
        verts[(i + 1) * 2 + 0] = cosf(a);
        verts[(i + 1) * 2 + 1] = sinf(a);
    }

    glGenVertexArrays(1, &circle_vao);
    glGenBuffers(1, &circle_vbo);
    glBindVertexArray(circle_vao);
    glBindBuffer(GL_ARRAY_BUFFER, circle_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    /* two-vertex buffer, rewritten every frame while dragging */
    glGenVertexArrays(1, &line_vao);
    glGenBuffers(1, &line_vbo);
    glBindVertexArray(line_vao);
    glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

/* ---------------------------------------------------------------- input */

static Vec2 screen_to_world(App *app, GLFWwindow *win, double mx, double my) {
    int w, h;
    glfwGetWindowSize(win, &w, &h);          /* window coords, not framebuffer */
    if (w <= 0 || h <= 0) return app->cam_center;
    double aspect = (double)w / (double)h;
    double ndc_x  = (mx / (double)w) * 2.0 - 1.0;
    double ndc_y  = 1.0 - (my / (double)h) * 2.0;   /* flip: screen y is down */
    return (Vec2){
        app->cam_center.x + ndc_x * app->cam_half * aspect,
        app->cam_center.y + ndc_y * app->cam_half
    };
}

static void mouse_button_cb(GLFWwindow *win, int button, int action, int mods) {
    (void)mods;
    App *app = glfwGetWindowUserPointer(win);
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;

    double mx, my;
    glfwGetCursorPos(win, &mx, &my);
    Vec2 wpos = screen_to_world(app, win, mx, my);

    if (action == GLFW_PRESS) {
        app->dragging   = 1;
        app->drag_start = wpos;
        app->drag_now   = wpos;
    } else if (action == GLFW_RELEASE && app->dragging) {
        app->dragging = 0;
        Vec2 drag = v2_sub(wpos, app->drag_start);
        Vec2 vel  = v2_scale(drag, DRAG_TO_VEL);   /* zero drag -> zero vel */
        bodyArray_push(&app->sim, bodyCreate(PLANET_MASS, app->drag_start, vel));
    }
}

static void key_cb(GLFWwindow *win, int key, int sc, int action, int mods) {
    (void)sc; (void)mods;
    App *app = glfwGetWindowUserPointer(win);
    if (action != GLFW_PRESS) return;

    switch (key) {
        case GLFW_KEY_SPACE:  app->paused = !app->paused;        break;
        case GLFW_KEY_C:      bodyArray_clear(&app->sim);        break;
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(win, 1);  break;
    }
}

static void scroll_cb(GLFWwindow *win, double xoff, double yoff) {
    (void)xoff;
    App *app = glfwGetWindowUserPointer(win);
    app->cam_half *= pow(0.9, yoff);
    if (app->cam_half < 10.0)     app->cam_half = 10.0;
    if (app->cam_half > 100000.0) app->cam_half = 100000.0;
}

/* ---------------------------------------------------------------- main */

int main(void) {
    App app = {0};
    app.cam_center = (Vec2){0.0, 0.0};
    app.cam_half   = 300.0;
    if (!bodyArray_init(&app.sim)) {
        fprintf(stderr, "bodyArray_init failed\n");
        return 1;
    }

    if (!glfwInit()) { fprintf(stderr, "glfwInit failed\n"); return 1; }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWwindow *win = glfwCreateWindow(WIN_W, WIN_H, "orbit", NULL, NULL);
    if (!win) { fprintf(stderr, "window creation failed\n"); glfwTerminate(); return 1; }
    glfwMakeContextCurrent(win);

	const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	if (mode) printf("reported refresh: %d Hz\n", mode->refreshRate);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        fprintf(stderr, "GLAD failed\n");
        glfwTerminate();
        return 1;
    }
    glfwSwapInterval(1);

    glfwSetWindowUserPointer(win, &app);      /* callbacks reach app through this */
    glfwSetMouseButtonCallback(win, mouse_button_cb);
    glfwSetKeyCallback(win, key_cb);
    glfwSetScrollCallback(win, scroll_cb);

    geometry_init();
    GLuint prog = make_program(VS_SRC, FS_SRC);

    GLint loc_center    = glGetUniformLocation(prog, "uCenter");
    GLint loc_radius    = glGetUniformLocation(prog, "uRadius");
    GLint loc_camcenter = glGetUniformLocation(prog, "uCamCenter");
    GLint loc_camhalf   = glGetUniformLocation(prog, "uCamHalf");
    GLint loc_color     = glGetUniformLocation(prog, "uColor");

    double prev_time   = glfwGetTime();
    double accumulator = 0.0;

    double fps_timer  = prev_time;
    int    fps_frames = 0;
    double fps        = 0.0;

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        /* ---- timing ---- */
        double now        = glfwGetTime();
        double frame_time = now - prev_time;
        prev_time = now;
        if (frame_time > MAX_FRAME_TIME) frame_time = MAX_FRAME_TIME;

        if (app.dragging) {
            double mx, my;
            glfwGetCursorPos(win, &mx, &my);
            app.drag_now = screen_to_world(&app, win, mx, my);
        }

        /* ---- physics: fixed steps, drawn from a bank of real elapsed time ---- */
        if (!app.paused && !app.dragging) {
            accumulator += frame_time;
            while (accumulator >= DT) {
                double sub_dt = DT / SUBSTEPS;
                for (int s = 0; s < SUBSTEPS; s++) {
                    compute_forces(app.sim.data, app.sim.count);
                    integrate(app.sim.data, app.sim.count, sub_dt);
                }
                accumulator -= DT;
            }
        } else {
            accumulator = 0.0;   /* don't bank time while paused */
        }

        /* ---- render ---- */
        int fbw, fbh;
        glfwGetFramebufferSize(win, &fbw, &fbh);
        glViewport(0, 0, fbw, fbh);
        glClearColor(0.04f, 0.04f, 0.07f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        double aspect = (fbh > 0) ? (double)fbw / (double)fbh : 1.0;

        glUseProgram(prog);
        glUniform2f(loc_camcenter, (float)app.cam_center.x, (float)app.cam_center.y);
        glUniform2f(loc_camhalf,   (float)(app.cam_half * aspect), (float)app.cam_half);

        glBindVertexArray(circle_vao);
        glUniform3f(loc_color, 0.55f, 0.72f, 1.0f);
        for (size_t i = 0; i < app.sim.count; i++) {
            Body *b = &app.sim.data[i];
            glUniform2f(loc_center, (float)b->pos.x, (float)b->pos.y);
            glUniform1f(loc_radius, (float)(RADIUS_K * sqrt(b->mass)));
            glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_SEGMENTS + 2);
        }

        if (app.dragging) {
            /* ghost of the body about to be spawned */
            glUniform3f(loc_color, 0.9f, 0.85f, 0.4f);
            glUniform2f(loc_center, (float)app.drag_start.x, (float)app.drag_start.y);
            glUniform1f(loc_radius, (float)(RADIUS_K * sqrt(PLANET_MASS)));
            glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_SEGMENTS + 2);

            /* velocity line: uCenter = 0, uRadius = 1 -> vertices ARE world coords */
            float seg[4] = {
                (float)app.drag_start.x, (float)app.drag_start.y,
                (float)app.drag_now.x,   (float)app.drag_now.y
            };
            glBindVertexArray(line_vao);
            glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(seg), seg);
            glUniform2f(loc_center, 0.0f, 0.0f);
            glUniform1f(loc_radius, 1.0f);
            glDrawArrays(GL_LINES, 0, 2);
        }

        /* ---- fps, averaged over half a second ---- */
        fps_frames++;
        if (now - fps_timer >= 0.5) {
            fps = fps_frames / (now - fps_timer);
            fps_frames = 0;
            fps_timer  = now;

            char title[160];
            snprintf(title, sizeof(title), "orbit  |  %.0f fps  |  bodies: %zu  |  %s",
                     fps, app.sim.count, app.paused ? "PAUSED" : "running");
            glfwSetWindowTitle(win, title);
        }

        glfwSwapBuffers(win);
    }

    glDeleteVertexArrays(1, &circle_vao);
    glDeleteVertexArrays(1, &line_vao);
    glDeleteBuffers(1, &circle_vbo);
    glDeleteBuffers(1, &line_vbo);
    glDeleteProgram(prog);
    bodyArray_free(&app.sim);
    glfwTerminate();
    return 0;
}
