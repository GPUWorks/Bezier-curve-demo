#include <iostream>
#include <list>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw_gl3.h>

using namespace std;

const unsigned int WINDOW_HEIGHT = 720;
const unsigned int WINDOW_WIDTH = 1280;

float deltaTime = 0.0f;    // time between current frame and last frame
float lastFrame = 0.0f;

struct Point {
    float x;
    float y;
    Point(float _x, float _y) : x(_x), y(_y) {};
};

bool animation = false, first_iter = false;

list<Point> curve;
list<Point> vertices;

unsigned int pointVAO, pointVBO, lineVAO, lineVBO;

void drawPoints(list<Point> vertices, glm::vec4 color, Shader shader);
void drawline(list<Point> vertices, float t, glm::vec4 color, Shader shader);

void processInput(GLFWwindow *window);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void changePoints(GLFWwindow* window);

int main(int argc, const char * argv[]) {
    
    /* ---------------------- Init ---------------------- */
    if (!glfwInit()) {
        cout << "Failed to initialize GLFW" << endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // Complication on MacOS
#endif
    
    // Create GLFW window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "hw4", NULL, NULL);
    if (!window) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }
    
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfwGL3_Init(window, true);
    
    // Setup style
    ImGui::StyleColorsDark();
    
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    
    
    /* ---------------------- Shader ---------------------- */
    // Xcode can only use absolute path!!
    Shader shader("C:/Users/Ruby/Desktop/Bezier-curve-demo/Bezier-curve-demo/src/shader.vs", "C:/Users/Ruby/Desktop/Bezier-curve-demo/Bezier-curve-demo/src/shader.fs");
    GLuint shader_program = shader.getShaderProgram();
    
    /* ---------------------- Render loop ---------------------- */
    glUseProgram(shader_program);
    
    glGenVertexArrays(1, &pointVAO);
    glGenBuffers(1, &pointVBO);
    glBindVertexArray(pointVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glEnable(GL_LINE_SMOOTH);
    float t = 0.0f;
    ImVec4 color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        processInput(window);
        glfwPollEvents();
        
        if (vertices.empty()) {
            continue;
        }
        
        if (animation) {
            t += 0.005f;
            if (t > 1.0f) t = 0.0f;
            drawline(vertices, t, glm::vec4(0.0f, 0.5f, 0.3f, 1.0f), shader);
            drawPoints(curve, glm::vec4(color.x, color.y, color.z, 1.0f), shader);
        } else {
            curve.clear();
            for (float a = 0; a < 1.0f; a += 0.001f) {
                drawline(vertices, a, glm::vec4(0.0f, 0.5f, 0.3f, 1.0f), shader);
            }
            first_iter = true;
            drawPoints(vertices, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), shader);
            drawPoints(curve, glm::vec4(color.x, color.y, color.z, 1.0f), shader);
        }

        ImGui_ImplGlfwGL3_NewFrame();
        {
            ImGui::ColorEdit3("Curve color", (float*)&color);
            ImGui::Text("\n");
            ImGui::Checkbox("Animation", &animation);
        }
        
        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    
    /* ---------------------- Deallocate the sources ---------------------- */
    glfwTerminate();
    return 0;
}

bool deleted = false;
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
        if (!deleted && !vertices.empty()) vertices.pop_back();
        curve.clear();
        deleted = true;
    }
    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_RELEASE) {
        deleted = false;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (!animation) {
//            if (glfwGetKey(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_REPEAT)
                glfwSetMouseButtonCallback(window, NULL);
            changePoints(window);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        if (!animation) {
            glfwSetMouseButtonCallback(window, mouseButtonCallback);
        }
    }
}

void changePoints(GLFWwindow* window) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    xpos = (xpos / WINDOW_WIDTH) * 2 - 1;
    ypos = 1 - (ypos / WINDOW_HEIGHT) * 2;
    float min_dist = 9999.0f;
    int min_idx = 0;
    auto min_vertex = vertices.begin();
    for (auto vertex = vertices.begin(); vertex != vertices.end(); vertex++) {
        float cur_dist = abs(xpos - vertex->x) + abs(ypos - vertex->y);
        if (cur_dist < min_dist) {
            min_vertex = vertex;
            min_dist = cur_dist;
        }
    }
    min_vertex->x = xpos;
    min_vertex->y = ypos;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        curve.clear();
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        Point tmp(float((xpos / WINDOW_WIDTH) * 2 - 1), float(1 - (ypos / WINDOW_HEIGHT) * 2));
        vertices.push_back(tmp);
        curve.clear();
    }
}

void drawPoints(list<Point> vertices, glm::vec4 color, Shader shader) {
    shader.setUniform4fv("vertexColor", color);
    for (auto vertex = vertices.begin(); vertex != vertices.end(); vertex++) {
        glBindVertexArray(pointVAO);
        glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), (float*)&(*vertex), GL_DYNAMIC_DRAW);
        glPointSize(4);
        glDrawArrays(GL_POINTS, 0, 1);
    }
}

void drawline(list<Point> vertices, float t, glm::vec4 color, Shader shader) {
    shader.setUniform4fv("vertexColor", color);
    
    // The point on curve, exit recursion
    if (vertices.size() == 1) {
        curve.push_back(vertices.front());
        return;
    }
    
    // Vertices for next function call
    list<Point> next_iter_vertices;
	int size = vertices.size();
	float points[50];

    // Calculate the t-section points
    int idx = 0;
    for (auto vertex = vertices.begin(); ;) {
        Point cur = *vertex;
		points[idx++] = cur.x, points[idx++] = cur.y;
		
		vertex++;
		if (vertex == vertices.end()) break;
	
		Point next = *(vertex);
        Point next_iter_point(t * cur.x + (1 - t) * next.x, t * cur.y + (1 - t) * next.y);
        next_iter_vertices.push_back(next_iter_point);
    }
    
    // Draw lines for vision
    if (animation || first_iter) {
        glBindVertexArray(lineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
        glBufferData(GL_ARRAY_BUFFER, size * 2 * sizeof(float), points, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINE_STRIP, 0, vertices.size());
        first_iter = false;
    }
    
    // Change line colors at each recursion
    if (color.x >= 1.0f) {
        if (color.y >= 1.0f) {
            color.z += 0.25;
        } else {
            color.y += 0.25;
        }
    } else {
        color.x += 0.25;
    }
    
    // recursive call
    drawline(next_iter_vertices, t, color, shader);
}

