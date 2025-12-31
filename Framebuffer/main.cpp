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
#include <lib/structs/transform.h>
#include <lib/primitives/cube.h>
#include <lib/primitives/plane.h>

#define STB_IMAGE_IMPLEMENTATION
#include <lib/stb_image.h>

#include <iostream>
#include <cstring>
#include <math.h>
#include <filesystem>
#include <map>

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

unsigned int imgToTexID(const char *filename, unsigned int *texture, GLint wrapMode) // ! check out model.TextureFromFile
{
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

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
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum format = GL_RED; // + This is defaulted to GL_RED to avoid warning.
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    return *texture;
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
    // glEnable(GL_STENCIL_TEST);
    // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // -- Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
    // glBlendFunc(GL_CONSTANT_COLOR, GL_SRC_ALPHA);
#pragma endregion

#pragma region // + Shader Init
    Shader litShader("dependencies/shaders/litObject.vs", "dependencies/shaders/litObject.fs");
    Shader lightSourceShader("dependencies/shaders/light.vs", "dependencies/shaders/light.fs");
    Shader singleColorShader("dependencies/shaders/singleColor.vs", "dependencies/shaders/singleColor.fs");
    Shader alphaShader("dependencies/shaders/alpha.vs", "dependencies/shaders/alpha.fs");
    Shader screenShader("dependencies/shaders/screen.vs", "dependencies/shaders/screen.fs");

    litShader.insertDirective(1, "#define NEAR_CLIP " + std::to_string(NEAR_CLIP));
    litShader.insertDirective(1, "#define FAR_CLIP " + std::to_string(FAR_CLIP));
    litShader.insertDirective(1, "#define NR_POINT " + std::to_string(POINT_LIGHT_NR));
    litShader.insertDirective(1, "#define NR_DIR " + std::to_string(DIR_LIGHT_NR));
    litShader.insertDirective(1, "#define NR_SPOT " + std::to_string(POINT_LIGHT_NR));
    litShader.insertDirective(1, "#define MAX_MATERIALS " + std::to_string(MAX_MATERIALS));

    alphaShader.insertDirective(1, "#define MAX_MATERIALS " + std::to_string(MAX_MATERIALS));
#pragma endregion

#pragma region // + Light Positions Init

    glm::vec3 lightPositions[] = {
        glm::vec3(-1.3f, 1.0f, -2.5f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -12.0f),
        glm::vec3(0.0f, 0.0f, -3.0f)};

#pragma endregion

#pragma region // + VAO, VBO

    // ! call only after a GL context has been made
    // initialize VAOs
    Cube::initialize();
    Plane::initialize();

#pragma endregion

#pragma region // + Framebuffer

    unsigned int FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO); // both read and write (indiv possibles)

    // -- Texture attachments
    unsigned int fbTexture;
    glGenTextures(1, &fbTexture);
    glBindTexture(GL_TEXTURE_2D, fbTexture);

    // * Since I have a retina display, getting actual size (retina has 2x size)
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

    // * dimensions are SCR, data is NULL (since we allocating and not filling[this happens in render loop])
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fbWidth, fbHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    // * don't care about mipmaps and wrapping, only filtering (in most cases)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // * created texture, now attach it to framebuffer

    // * target: framebuffer type (draw, read or both).
    // * attachment: type of attachment. This is color attachment. Can attach more than 1 color attachment (0).
    // * textarget: type of the texture you want to attach.
    // * texture: actual texture to attach.
    // * level: mipmap level. Set to 0.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbTexture, 0);

#pragma endregion

#pragma region // +. Renderbuffer

    // without this, depth testing goes haywire on frame buffer quad

    unsigned int RBO;
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);

    // * allocate storage for depth and stencil buffer (we won't be sampling these)
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fbWidth, fbHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, 0); // unbind after allocating memory

    // * attach RBO to framebuffer's depth and stencil attachment points
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind framebuffer

#pragma endregion

#pragma region // + Model load

    stbi_set_flip_vertically_on_load(true);

    Model bagModel("media/backpack/backpack.obj");

#pragma endregion

#pragma region // + Cube inits

    Transform cubeTransforms[] = {
        Transform(glm::vec3(5.0, 0.0, 2.0), glm::vec3(2.0f, 2.0f, 1.0f)),
    };

    vector<Cube> cubes(sizeof(cubeTransforms) / sizeof(Transform));
    for (int i = 0; i < sizeof(cubeTransforms) / sizeof(Transform); i++)
    {
        cubes.push_back(Cube(cubeTransforms[i]));
    }

    // -----------------------------------------------------------------

    Transform outlineCubeTransforms[] = {
        Transform(glm::vec3(5.0, 0.0, 0.0), glm::vec3(2.0f, 2.0f, 1.0f)),
        Transform(glm::vec3(5.0, 4.0, 6.0), glm::vec3(2.0f, 2.0f, 1.0f)),
    };

    vector<Cube> outlineCubes(sizeof(outlineCubeTransforms) / sizeof(Transform));
    for (int i = 0; i < sizeof(outlineCubeTransforms) / sizeof(Transform); i++)
    {
        outlineCubes.push_back(Cube(outlineCubeTransforms[i]));
    }

#pragma endregion

#pragma region // + Floor init

    Transform floorTransform = Transform(glm::vec3(0.0, -2.0, 1.0), glm::vec3(15.0, 5.0, 15.0));

    Plane floor = Plane(floorTransform);

#pragma endregion

#pragma region // + Grass inits

    Transform grassTransforms[] = {
        Transform(glm::vec3(3.0, -1.0, -2.0), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f)),
        Transform(glm::vec3(3.0, -1.0, -2.0), glm::vec3(0.0f, -90.0f, 90.0f), glm::vec3(2.0f, 2.0f, 2.0f)), // lmao rotations… kill me
        Transform(glm::vec3(-1.0, -1.0, 3.0), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f)),
        Transform(glm::vec3(-1.0, -1.0, 3.0), glm::vec3(0.0f, -90.0f, 90.0f), glm::vec3(2.0f, 2.0f, 2.0f)),
        Transform(glm::vec3(-2.0, -1.0, 1.0), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f)),
        Transform(glm::vec3(-2.0, -1.0, 1.0), glm::vec3(0.0f, -90.0f, 90.0f), glm::vec3(2.0f, 2.0f, 2.0f)),
        Transform(glm::vec3(4.0, -1.0, -2.0), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f)),
        Transform(glm::vec3(4.0, -1.0, -2.0), glm::vec3(0.0f, -90.0f, 90.0f), glm::vec3(2.0f, 2.0f, 2.0f)),
        Transform(glm::vec3(1.0, -1.0, 1.0), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f)),
        Transform(glm::vec3(1.0, -1.0, 1.0), glm::vec3(0.0f, -90.0f, 90.0f), glm::vec3(2.0f, 2.0f, 2.0f)),
    };

    vector<Plane> grasses(sizeof(grassTransforms) / sizeof(Transform));
    for (int i = 0; i < sizeof(grassTransforms) / sizeof(Transform); i++)
    {
        grasses.push_back(Plane(grassTransforms[i]));
    }

#pragma endregion

#pragma region // + Window inits

    Transform windowTransforms[] = {
        Transform(glm::vec3(3.0, -1.0, -1.0), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f)),
        Transform(glm::vec3(2.0, -1.0, 2.0), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f)),
        Transform(glm::vec3(4.0, -1.0, -1.5), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f)),
    };

    vector<Plane> windows(sizeof(windowTransforms) / sizeof(Transform));
    for (int i = 0; i < sizeof(windowTransforms) / sizeof(Transform); i++)
    {
        windows.push_back(Plane(windowTransforms[i]));
    }

#pragma endregion

#pragma region // + Screen Quad

    // Since its only one quad, i'm making it manually cuz i know all the data
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f,  //
        -1.0f, -1.0f, 0.0f, 0.0f, //
        1.0f, -1.0f, 1.0f, 0.0f,  //

        -1.0f, 1.0f, 0.0f, 1.0f, //
        1.0f, -1.0f, 1.0f, 0.0f, //
        1.0f, 1.0f, 1.0f, 1.0f   //
    };

    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glBindVertexArray(0);

#pragma endregion

#pragma region // + Pre-Loop

#pragma region // -- Textures

    litShader.use();
    litShader.setBool("useTextures", useTextures);

    litShader.setVec3("basicMaterial.albedo", glm::value_ptr(objColor));
    litShader.setVec3("basicMaterial.specular", glm::value_ptr(objColor));

    unsigned int voldy_texture, grass_texture, window_texture;
    imgToTexID("media/voldemort.jpeg", &voldy_texture, GL_CLAMP_TO_EDGE);
    imgToTexID("media/grass.png", &grass_texture, GL_CLAMP_TO_EDGE);
    imgToTexID("media/blending_transparent_window.png", &window_texture, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE0 + voldy_texture);
    litShader.setInt("textureMaterials[1].albedo", voldy_texture);
    litShader.setInt("textureMaterials[1].specular", voldy_texture);
    litShader.setInt("textureMaterials[1].normal", voldy_texture);
    glBindTexture(GL_TEXTURE_2D, voldy_texture);

    glActiveTexture(GL_TEXTURE0 + grass_texture);
    litShader.setInt("textureMaterials[2].albedo", grass_texture);
    litShader.setInt("textureMaterials[2].specular", grass_texture);
    litShader.setInt("textureMaterials[2].normal", grass_texture);
    glBindTexture(GL_TEXTURE_2D, grass_texture);

    glActiveTexture(GL_TEXTURE0 + grass_texture);
    glBindTexture(GL_TEXTURE_2D, grass_texture);

    litShader.setInt("activeMaterial", 0);

    alphaShader.use();
    glActiveTexture(GL_TEXTURE0 + grass_texture);
    alphaShader.setInt("texArray[0]", grass_texture);
    glBindTexture(GL_TEXTURE_2D, grass_texture);

    glActiveTexture(GL_TEXTURE0 + window_texture);
    alphaShader.setInt("texArray[1]", window_texture);
    glBindTexture(GL_TEXTURE_2D, window_texture);

    alphaShader.setInt("activeTexture", window_texture);

    litShader.use();

#pragma endregion

#pragma region // -- Lights

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
        SpotLight(&litShader, lightColor, lightStrength, lightPositions[i], camera.LookDir, 12.5f, 17.5f, i); // ! lightPositions[i]
    }

#pragma endregion

#pragma region // -- Outline

    Outline outlineProperties;
    outlineProperties.outlineColor = glm::vec3(0.84, 0.568, 0.06);
    outlineProperties.outlineShader = &singleColorShader;
    outlineProperties.outlineThickness = 0.0;

#pragma endregion

#pragma region // -- Blend depth

    std::multimap<float, Plane> sorted;

#pragma endregion

#pragma region // -- Framebuffer check

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw;

#pragma endregion

#pragma endregion

    // + RENDER LOOP
    // -----------

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

#pragma region FRAMEBUFFER RENDERING

        // * bind to framebuffer and draw scene as we normally would to color texture
        glBindFramebuffer(GL_FRAMEBUFFER, FBO); // bind to our framebuffer (off-screen rendering)
        glEnable(GL_DEPTH_TEST);                // enable depth testing (is disabled for screen quad)

#pragma endregion

        glClearColor(0.09f, 0.11f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // for after image, remove color buffer bit

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

        singleColorShader.use();
        singleColorShader.setMat4("view", glm::value_ptr(view));
        singleColorShader.setMat4("projection", glm::value_ptr(projection));

        litShader.use();

#pragma endregion

        // -- Opaque Stuff

#pragma region PRIMITIVES

        for (int i = 0; i < outlineCubes.size(); i++)
        {
            outlineCubes[i].Draw(&litShader, outlineProperties);
        }

        litShader.setInt("activeMaterial", 1);

        for (int i = 0; i < cubes.size(); i++)
        {
            cubes[i].Draw(&litShader);
        }

#pragma endregion

#pragma region MODEL

        Transform modelTransform = Transform(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));

        glm::mat4 modelMat = modelTransform.modelMatx;
        glm::mat3 normalMat = modelTransform.normalMatx;

        litShader.setInt("activeMaterial", 0);

        litShader.setMat3("normalMat", glm::value_ptr(normalMat));
        litShader.setMat4("model", glm::value_ptr(modelMat));

        outlineProperties.transform = modelTransform;
        bagModel.IsOutlineEnabled(true, outlineProperties);
        bagModel.Draw(&litShader);

#pragma endregion

#pragma region FLOOR

        litShader.use();
        litShader.setMat4("view", glm::value_ptr(view));
        litShader.setMat4("projection", glm::value_ptr(projection));

        litShader.setBool("useTextures", false);
        floor.Draw(&litShader);
        litShader.setBool("useTextures", true);

#pragma endregion

        // -- Transparent Stuff

#pragma region GRASSES
        // not including this stuff in sorting cuz alpha discards

        alphaShader.use();
        alphaShader.setMat4("view", glm::value_ptr(view));
        alphaShader.setMat4("projection", glm::value_ptr(projection));

        alphaShader.setInt("activeTexture", 0);

        glDisable(GL_CULL_FACE);
        for (int i = 0; i < grasses.size(); i++)
        {
            grasses[i].Draw(&alphaShader);
        }
        glEnable(GL_CULL_FACE);

#pragma endregion

#pragma region WINDOWS

        // grasses before this, alphaShader taking vals from that

        sorted.clear();
        for (int i = 0; i < windows.size(); i++) // * map automatically sorts based on key
        {
            float dist = glm::length(camera.Position - windows[i].transform.getPosition());
            // sorted[dist] = windows[i];
            sorted.insert({dist, windows[i]});
        }

        alphaShader.setInt("activeTexture", 1);

        glDisable(GL_CULL_FACE);
        for (std::multimap<float, Plane>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); it++)
        {
            it->second.Draw(&alphaShader);
        }
        glEnable(GL_CULL_FACE);

#pragma endregion

#pragma region LIGHT SOURCES

        lightSourceShader.use();
        lightSourceShader.setMat4("view", glm::value_ptr(view));
        lightSourceShader.setMat4("projection", glm::value_ptr(projection));

        glBindVertexArray(Cube::VAO);
        for (int i = 0; i < (sizeof(lightPositions) / sizeof(lightPositions[0])); i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, lightPositions[i]);
            model = glm::scale(model, glm::vec3(0.1));

            lightSourceShader.setMat4("model", glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, Cube::NUM_VERTICES);
        }

#pragma endregion

#pragma region RENDER FROM FRAMEBUFFER TO SCREEN

        // * writing to our framebuffer means nothing is drawn on screen (off-screen rendering)
        // * to show stuff on screen, use default fb (0)
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // write to default framebuffer instead of ours
        glDisable(GL_DEPTH_TEST);             // disable depth test so screen-space quad isn't discarded due to depth test.
        glClear(GL_COLOR_BUFFER_BIT);         // why not depth? disabled depth test

        screenShader.use();
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0 + fbTexture);
        screenShader.setInt("screenTexture", fbTexture);
        glBindTexture(GL_TEXTURE_2D, fbTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);

#pragma endregion

        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);

        // swap buffers can be implemented with renderbuffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // + END RENDER LOOP

    glDeleteVertexArrays(1, &Cube::VAO);
    glDeleteBuffers(1, &Cube::VBO);
    glDeleteVertexArrays(1, &Plane::VAO);
    glDeleteBuffers(1, &Plane::VBO);

    glDeleteFramebuffers(1, &FBO);

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
