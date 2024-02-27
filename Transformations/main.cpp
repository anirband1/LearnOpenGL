#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include <lib/shader_s.h>
#define STB_IMAGE_IMPLEMENTATION
#include <lib/stb_image.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float opacity;

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

    // build and compile our shader program
    Shader shaderProgram("dependencies/shaders/basic.vs", "dependencies/shaders/basic.fs");

    // + vertices

    float vertices[] = {
        // vertices         // color          // texture
        -0.9f, -0.9f, 0.0f, 0.9f, 0.3f, 0.6f, 0.0f, 0.0f, // SW
        -0.9f, 0.9f, 0.0f, 0.8f, 0.2f, 0.3f, 0.0f, 1.0f,  // NW
        0.9f, -0.9f, 0.0f, 0.1f, 0.2f, 0.8f, 1.0f, 0.0f,  // SE
        0.9f, 0.9f, 0.0f, 0.4f, 0.7f, 0.2f, 1.0f, 1.0f    // NE
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 1, 3};

    // + ---------------------------------------------------

    int numIndices = sizeof(indices) / sizeof(indices[0]);

    unsigned int VBO, EBO, VAO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // ! This shouldn't be uncommented as A VAO stores the glBindBuffer calls when the target is GL_ELEMENT_ARRAY_BUFFER.
    // ! This also means it stores its unbind calls so make sure you don't unbind the element array buffer before unbinding your VAO,
    // ! otherwise it doesn't have an EBO configured.
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // TEXTURES

    unsigned int texture1, texture2;

    imgToTexID("media/cat2.jpeg", &texture1, GL_REPEAT);
    imgToTexID("media/planets.jpeg", &texture2, GL_CLAMP_TO_EDGE);

    // render loop
    // -----------

    shaderProgram.use();
    shaderProgram.setInt("imageTexture1", 0);
    shaderProgram.setInt("imageTexture2", 1);

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.09f, 0.11f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shaderProgram.setFloat("opacity", opacity);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2); // commenting both ActiveTexture calls still makes planets show up since it default assigns texel 0;
        glBindVertexArray(VAO);

        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0); // no need to unbind it every time

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    shaderProgram.del();

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float delOpacity = 0.01;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        opacity += delOpacity;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        opacity -= delOpacity;

    opacity = clamp(opacity, 0.0, 1.0);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}
