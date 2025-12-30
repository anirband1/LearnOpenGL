// Minecraft fly controls
// scroll to zoom

// TODO check how many shaders are active after using insertDirective   {shader_s.h}
// TODO check whether the correct shader is deleted if adding del() to compileAndLink() {shader_s.h}
// TODO make it so that if useTextures disabled, model doesnt LOAD textures.
// TODO make a transform struct
// TODO outline function (pass in VAO, Transform struct)

#pragma region // + INCLUDE

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <lib/constants.h>
#include <lib/shader_s.h>
#include <lib/camera.h>
#include <lib/lights.h>
#include <lib/model.h>

#define STB_IMAGE_IMPLEMENTATION
#include <lib/stb_image.h>

#include <iostream>
#include <cstring>
#include <math.h>
#include <filesystem>

#pragma endregion

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xPos, double yPos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 objColor = glm::vec3(1.0f, 0.5f, 0.31f);
glm::vec3 sunDir = glm::vec3(0.0f, -1.0f, 0.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lightStrength = 3.0f;

bool firstMouse = true;
bool useTextures = true;

float lastX, lastY;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

void imgToTexID(const char *filename, unsigned int *texture, GLint wrapMode) // ! check out model.TextureFromFile
{
    glGenTextures(1, texture); // +

    glBindTexture(GL_TEXTURE_2D, *texture); // +

    //* Wrapping method
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    // ! huh this is stupid doing it redundant
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

    // -- 1. Backface Culling // this speeds up model rendering by a lot
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // -- 2. Z-Buffer
    glEnable(GL_DEPTH_TEST);

    glDepthFunc(GL_LESS); // default

    // -- Stencil testing
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
#pragma endregion

#pragma region // + Shader Init
    Shader litShader("dependencies/shaders/litObject.vs", "dependencies/shaders/litObject.fs");
    Shader lightSourceShader("dependencies/shaders/light.vs", "dependencies/shaders/light.fs");
    Shader singleColorShader("dependencies/shaders/singleColor.vs", "dependencies/shaders/singleColor.fs");

    litShader.insertDirective(1, "#define NEAR_CLIP " + std::to_string(NEAR_CLIP));
    litShader.insertDirective(1, "#define FAR_CLIP " + std::to_string(FAR_CLIP));
    litShader.insertDirective(1, "#define NR_POINT " + std::to_string(POINT_LIGHT_NR));
    litShader.insertDirective(1, "#define NR_DIR " + std::to_string(DIR_LIGHT_NR));
    litShader.insertDirective(1, "#define NR_SPOT " + std::to_string(POINT_LIGHT_NR));
    litShader.insertDirective(1, "#define MAX_MATERIALS " + std::to_string(MAX_MATERIALS));
#pragma endregion

#pragma region // + Cube Vertices Init

    float vertices[] = {
        // positions          // normals        // texture coords
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, //
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   //
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,  //
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   //
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, //
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,  //

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  //
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   //
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   //
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  //
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //

        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   //
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  //
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  //
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   //

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   //
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  //
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   //
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  //

        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, //
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,  //
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   //
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   //
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,  //
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, //

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   //
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  //
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   //
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,  //
    };

    glm::vec3 lightPositions[] = {
        glm::vec3(-1.3f, 1.0f, -2.5f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -12.0f),
        glm::vec3(0.0f, 0.0f, -3.0f)};
    // ---------------------------------------------------

    // int numIndices = sizeof(indices) / sizeof(indices[0]); // ----
    int numDrawnVertices = (sizeof(vertices) / sizeof(vertices[0])) / 2; // *3/6

#pragma endregion

#pragma region // + Light buffers, VAO

    unsigned int cubeVAO, lightVAO;
    unsigned int VBO;

    glGenVertexArrays(1, &lightVAO);
    glGenVertexArrays(1, &cubeVAO);

    glGenBuffers(1, &VBO);

    // cube
    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0); // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float))); // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float))); // texCoord
    glEnableVertexAttribArray(2);

    // light
    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

#pragma endregion

#pragma region // + Textures and Pre-Loop

    litShader.use();
    litShader.setBool("useTextures", useTextures);

    for (int i = 0; i < POINT_LIGHT_NR; i++)
    {
        PointLight(&litShader, lightColor, lightStrength, lightPositions[i], i);
    }

    for (int i = 0; i < DIR_LIGHT_NR; i++)
    {
        DirectionalLight(&litShader, lightColor, lightStrength, sunDir, i);
    }

    for (int i = 0; i < SPOT_LIGHT_NR; i++)
    {
        SpotLight(&litShader, lightColor, lightStrength, lightPositions[i], camera.LookDir, 12.5f, 17.5f, i);
    }

    if (!useTextures)
        litShader.setVec3("basicMaterial.albedo", glm::value_ptr(objColor));

#pragma endregion

#pragma region // + Model load

    stbi_set_flip_vertically_on_load(true);

    Model bagModel("media/backpack/backpack.obj");

#pragma endregion

    // + RENDER LOOP
    // -----------

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.09f, 0.11f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // for after image, remove color buffer bit

        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glEnable(GL_STENCIL_TEST);

        litShader.use();

#pragma region CAMERA
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, (float)NEAR_CLIP, (float)FAR_CLIP);

        litShader.setVec3("viewPos", glm::value_ptr(camera.Position));

        litShader.setVec3("spotLights[0].lightPos", glm::value_ptr(camera.Position));
        litShader.setVec3("spotLights[0].lightDir", glm::value_ptr(camera.LookDir));

        litShader.setMat4("view", glm::value_ptr(view));
        litShader.setMat4("projection", glm::value_ptr(projection));

#pragma endregion

#pragma region STENCIL & Z-TESTING

        singleColorShader.use();
        singleColorShader.setMat4("view", glm::value_ptr(view));
        singleColorShader.setMat4("projection", glm::value_ptr(projection));

        litShader.use();

        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);

        // + pass 1 normal cube
        // -- render cube 1
        glm::mat4 cube1Transform = glm::mat4(1.0f);
        glm::mat3 cube1NormalMat = glm::mat3(1.0);

        cube1Transform = glm::translate(cube1Transform, glm::vec3(5.0, 0.0, 0.0));
        cube1Transform = glm::scale(cube1Transform, glm::vec3(2.0f, 2.0f, 1.0f));
        cube1NormalMat = glm::mat3(glm::transpose(glm::inverse(cube1Transform)));

        litShader.setMat3("normalMat", glm::value_ptr(cube1NormalMat));
        litShader.setMat4("model", glm::value_ptr(cube1Transform));

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, numDrawnVertices);

        // -- render cube 2
        glm::mat4 cube2Transform = glm::mat4(1.0f);
        glm::mat3 cube2NormalMat = glm::mat3(1.0);

        cube2Transform = glm::translate(cube2Transform, glm::vec3(5.0, 0.5, 0.0));
        cube2Transform = glm::scale(cube2Transform, glm::vec3(1.99f, 1.0f, 0.99f)); // for more 9's, breaks down as camera goes farther
        cube2NormalMat = glm::mat3(glm::transpose(glm::inverse(cube2Transform)));

        litShader.setMat3("normalMat", glm::value_ptr(cube2NormalMat));
        litShader.setMat4("model", glm::value_ptr(cube2Transform));

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, numDrawnVertices);

        // + -------------------------------------------------------------------------------

        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);

        float scale = 1.1f;

        singleColorShader.use();

        // + pass 2 stencil solid color
        // -- render cube 1
        cube1Transform = glm::mat4(1.0f);
        cube1NormalMat = glm::mat3(1.0);

        cube1Transform = glm::translate(cube1Transform, glm::vec3(5.0, 0.0, 0.0));
        cube1Transform = glm::scale(cube1Transform, glm::vec3(2.0f, 2.0f, 1.0f) * scale);
        cube1NormalMat = glm::mat3(glm::transpose(glm::inverse(cube1Transform)));

        singleColorShader.setMat3("normalMat", glm::value_ptr(cube1NormalMat));
        singleColorShader.setMat4("model", glm::value_ptr(cube1Transform));

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, numDrawnVertices);

        // -- render cube 2
        cube2Transform = glm::mat4(1.0f);
        cube2NormalMat = glm::mat3(1.0);

        cube2Transform = glm::translate(cube2Transform, glm::vec3(5.0, 0.5, 0.0));
        cube2Transform = glm::scale(cube2Transform, glm::vec3(1.99f, 1.0f, 0.99f)); // for more 9's, breaks down as camera goes farther
        cube2NormalMat = glm::mat3(glm::transpose(glm::inverse(cube2Transform)));

        singleColorShader.setMat3("normalMat", glm::value_ptr(cube2NormalMat));
        singleColorShader.setMat4("model", glm::value_ptr(cube2Transform));

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, numDrawnVertices);

        // + -------------------------------------------------------------------------------
        // back to defaults
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glDisable(GL_STENCIL_TEST);

        litShader.use();

#pragma endregion

#pragma region MODEL

        // render the loaded model
        glm::mat4 modelTransform = glm::mat4(1.0f);
        glm::mat3 normalMat = glm::mat3(1.0);

        modelTransform = glm::translate(modelTransform, glm::vec3(0.0, 0.0, 0.0)); // translate it down so it's at the center of the scene
        // modelTransform = glm::translate(modelTransform, glm::vec3(sin(glfwGetTime()), sin(glfwGetTime() + glm::radians(90.0)), sin(2 * glfwGetTime() + glm::radians(180.0)))); // translate it down so it's at the center of the scene
        // modelTransform = glm::rotate(modelTransform, (float)glfwGetTime(), glm::vec3(0.5f, 0.3f, 0.4f));
        modelTransform = glm::scale(modelTransform, glm::vec3(1.0f, 1.0f, 1.0f)); // it's a bit too big for our scene, so scale it down
        normalMat = glm::mat3(glm::transpose(glm::inverse(modelTransform)));

        litShader.setMat3("normalMat", glm::value_ptr(normalMat));
        litShader.setMat4("model", glm::value_ptr(modelTransform));

        bagModel.Draw(&litShader);

#pragma endregion

#pragma region LIGHT SOURCES

        lightSourceShader.use();
        lightSourceShader.setMat4("view", glm::value_ptr(view));
        lightSourceShader.setMat4("projection", glm::value_ptr(projection));

        glBindVertexArray(lightVAO);
        for (int i = 0; i < (sizeof(lightPositions) / sizeof(lightPositions[0])); i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, lightPositions[i]);
            model = glm::scale(model, glm::vec3(0.1));

            lightSourceShader.setMat4("model", glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, numDrawnVertices);
        }

#pragma endregion

        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // + END RENDER LOOP

    glDeleteVertexArrays(1, &lightVAO);
    glDeleteVertexArrays(1, &cubeVAO);
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
