// W to move forward
// A to move left
// S to move backward
// D to move right
// up arrow to move up
// down arrow to move down
// ] to increase opactity of second tex
// [ to decrease opactity of second tex
// scroll to zoom

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <lib/shader_s.h>
#include <lib/camera.h>

#define STB_IMAGE_IMPLEMENTATION
#include <lib/stb_image.h>

#include <iostream>
#include <math.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xPos, double yPos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 objColor = glm::vec3(1.0f, 0.5f, 0.31f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool firstMouse = true;
bool useTextures = false;

float lastX, lastY;
float opacity;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

float nRand()
{
    return ((float)rand() / INT32_MAX);
}

float clamp(float val, float min, float max)
{
    val = val < min ? min : val;
    val = val > max ? max : val;
    return val;
}

void imgToTexID(const char *filename, unsigned int *texture, GLint wrapMode)
{
    glGenTextures(1, texture); // +

    glBindTexture(GL_TEXTURE_2D, *texture); // +

    //* Wrapping method
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    //* Filtering method
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // scale down
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  // scale up

    //* Mipmaps interpolate method
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels; // number of color channels
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
}

void printActiveVAO()
{
    GLint activeVAO;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &activeVAO);
    std::cout << "Active VAO: " << activeVAO << std::endl;
}

int main()
{
#pragma region 'glfw: initialize and configure'
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

#pragma endregion

#pragma region 'glad'
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
#pragma endregion

#pragma region 'global OpenGL states'
    // * Two ways to render it such that depth of points are respected:

    // -- 1. Backface Culling
    // glFrontFace(GL_CW);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);

    // -- 2. Z-Buffer
    glEnable(GL_DEPTH_TEST);
#pragma endregion

    // build and compile our shader program
    Shader litShader("dependencies/shaders/litObject.vs", "dependencies/shaders/litObject.fs");
    Shader lightSourceShader("dependencies/shaders/light.vs", "dependencies/shaders/light.fs");

    // + Inits

    float vertices[] = {
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,  //
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,   //
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,   //
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,    //
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, //
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f,  //
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f,  //
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f,   //
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 1, 3,
        1, 5, 3,
        3, 5, 7,
        5, 4, 7,
        7, 4, 6,
        4, 0, 6,
        6, 0, 2,
        2, 3, 6,
        6, 3, 7,
        4, 5, 0,
        0, 5, 1};

    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f, 2.0f, -2.5f),
        glm::vec3(1.5f, 0.2f, -1.5f),
        glm::vec3(-1.3f, 1.0f, -1.5f)};

    glm::vec3 lightPositions[] = {
        glm::vec3(-1.3f, 1.0f, -2.5f),
        glm::vec3(-0.2f, 1.9f, -0.5f)};
    // ---------------------------------------------------

    int numIndices = sizeof(indices) / sizeof(indices[0]);

    // + Object buffers, VAO
    unsigned int VBO, EBO, VAO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // + Light buffers, VAO
    // we only need VAo as vertices are already defined, they just have different properties this time.

    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // TEXTURES

    unsigned int texture1, texture2;

    if (useTextures)
    {
        imgToTexID("media/voldemort.jpeg", &texture1, GL_REPEAT);
        imgToTexID("media/planets.jpeg", &texture2, GL_CLAMP_TO_EDGE);
    }

    // RENDER LOOP
    // -----------

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.09f, 0.11f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // + CAMERA

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.01f, 100.0f);

        // + OBJECT

        litShader.use();
        litShader.setVec3("objColor", glm::value_ptr(objColor));
        litShader.setVec3("lightColor", glm::value_ptr(lightColor));

        litShader.setFloat("opacity", opacity);

        litShader.setMat4("view", glm::value_ptr(view));
        litShader.setMat4("projection", glm::value_ptr(projection));

        // TEXTURE
        if (useTextures)
        {
            litShader.setInt("imageTexture1", 0);
            litShader.setInt("imageTexture2", 1);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture2);
        }

        // TRANSFORM

        glBindVertexArray(VAO);
        for (int i = 0; i < (sizeof(cubePositions) / sizeof(cubePositions[0])); i++)
        {
            float rot = 20.0f * i;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, (float)glfwGetTime() + glm::radians(rot), glm::vec3(0.5f, 0.3f, 0.4f));

            litShader.setMat4("model", glm::value_ptr(model));

            glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
        }

        // + LIGHT SOURCES

        lightSourceShader.use();
        lightSourceShader.setMat4("view", glm::value_ptr(view));
        lightSourceShader.setMat4("projection", glm::value_ptr(projection));

        glBindVertexArray(lightVAO);
        for (int i = 0; i < (sizeof(lightPositions) / sizeof(lightPositions[0])); i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, lightPositions[i]);

            lightSourceShader.setMat4("model", glm::value_ptr(model));

            glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
        }

        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);

    litShader.del();
    lightSourceShader.del();

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float delOpacity = 0.01;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
        opacity -= delOpacity;

    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
        opacity += delOpacity;

    opacity = clamp(opacity, 0.0, 1.0);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xPos, double yPos)
{
    if (firstMouse)
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    float xOffset = xPos - lastX;
    float yOffset = yPos - lastY;
    lastX = xPos;
    lastY = yPos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll((float)yoffset);
}
