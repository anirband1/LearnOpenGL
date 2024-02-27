#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include <lib/shader_s.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const int NUM_VERTICES = 3;

float nRand()
{
    return ((float)rand() / INT32_MAX);
}

int main()
{
    // glfw: initialize and configure
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

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    Shader shaderProgram("dependencies/shaders/basic.vs", "dependencies/shaders/basic.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    // float vertices[] = {
    //     // vertices         // color
    //     -0.5f, -0.5f, 0.0f, 0.5f, 0.3f, 0.6f, // SW
    //     -0.5f, 0.5f, 0.0f, 0.8f, 0.2f, 0.3f,  // NW
    //     0.5f, -0.5f, 0.0f, 0.1f, 0.2f, 0.8f,  // SE
    //     0.5f, 0.5f, 0.0f, 0.4f, 0.7f, 0.2f    // NE
    // };

    int numVerticesWColor = 6 * NUM_VERTICES;
    float vertices[numVerticesWColor];

    srand(69);

    for (int i = 0; i < numVerticesWColor; i += 6)
    {
        vertices[i + 0] = nRand() * 2.0 - 1.0;
        vertices[i + 1] = nRand() * 2.0 - 1.0;
        vertices[i + 2] = nRand() * 2.0 - 1.0;
        vertices[i + 3] = nRand();
        vertices[i + 4] = nRand();
        vertices[i + 5] = nRand();
    }

    // unsigned int indices[] = {
    //     0, 1, 2,
    //     2, 1, 3};

    int indices[3 * (NUM_VERTICES - 2)]; // number of indices req for simple triangulate

    // int v = 1;
    // for (int i = 0; i < 3 * (NUM_VERTICES - 2); i += 3)
    // {
    //     indices[i + 0] = 0;
    //     indices[i + 1] = v;
    //     indices[i + 2] = v + 1;
    //     v++;
    // }

    int v = 0;
    for (int i = 0; i < 3 * (NUM_VERTICES - 2); i += 3)
    {
        indices[i + 0] = v % NUM_VERTICES;
        indices[i + 1] = (v + 1) % NUM_VERTICES;
        indices[i + 2] = (v + 2) % NUM_VERTICES;
        v++;
    }

    int numIndices = sizeof(indices) / sizeof(indices[0]);

    unsigned int VBO, EBO, VAO; // * VAO automatically binds EBO

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // ! This shouldn't be uncommented as A VAO stores the glBindBuffer calls when the target is GL_ELEMENT_ARRAY_BUFFER.
    // ! This also means it stores its unbind calls so make sure you don't unbind the element array buffer before unbinding your VAO,
    // ! otherwise it doesn't have an EBO configured.
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    // glFrontFace(GL_CW);

    // render loop
    // -----------

    // int numOfVertices = (sizeof(vertices) / sizeof(vertices[0])) / 3;

    int toggle = 1;

    // for (int i = 0; i < numVerticesWColor; i++)
    // {
    //     std::cout << vertices[i] << std::endl;
    // }

    while (!glfwWindowShouldClose(window))
    {
        // input
        // ------
        processInput(window);

        // render
        // ------
        float R = sin(toggle * 0.01 + rand() * 1);
        float G = sin(toggle * 0.01 + rand() * 1);
        float B = sin(toggle * 0.01 + rand() * 1);
        glClearColor(R, G, B, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // line vs fill
        toggle % 2 == 0 ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        toggle++;

        // assign shaders to program and use
        shaderProgram.use();

        /*
        float timeVal = glfwGetTime();
        float greenVal = sin(timeVal) / 2.0 + 0.5;
        int vertexColorLocation = glGetUniformLocation(shaderProgram, "outColor");
        glUniform4f(vertexColorLocation, 0.3, greenVal, 0.2, 1.0);
        */

        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

        // glDrawArrays(GL_TRIANGLES, 0, numOfVertices);

        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
        // *    primitive type
        // *    num of elements to be drawn (indices)
        // *    type of indices
        // *    offset / pass

        glBindVertexArray(0); // no need to unbind it every time

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    shaderProgram.del();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    // glViewport(0, 0, width, height);
    glViewport(0, 0, width, height);
}
