#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// * we declare all the input vertex attributes in the vertex shader with the in keyword.
// * Right now we only care about position data so we only need a single vertex attribute.
const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n" // * we decide pos data should go to loc 0,
                                                                         // * later on pass 0 for pos (other nums for other attribs)
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                 "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                                   "}\n\0";

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);                 // we are using OpenGL version ->3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);                 // we are using OpenGL version 3.3<-
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // core vs compatibility

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
    // ------------------------------------
    // vertex shader
    // * Unsigned int because this is the ID of the shader, we always reference the shader by its ID. (same for VBO, VAO)
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success); // * iv describes the parameters returned, in this case a vector of ints.
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    // link shaders
    // * To use the compiled shaders we link them to a shader program object and activate this shader program when rendering objects.
    // * The activated shader program's shaders will be used when we issue render calls.
    // * When linking the shaders into a program it links the outputs of each shader to the inputs of the next shader.
    // * This is also where you'll get linking errors if your outputs and inputs do not match.
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // left
        0.5f, -0.5f, 0.0f,  // top
        0.0f, 0.5f, 0.0f    // right
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO); // * Like any object in OpenGL, this buffer has a unique ID corresponding to it, so we can generate one with a buffer ID using this func
    // * the '1' tells OpenGL that we want to make only one buffer object

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    // * We bind the newly created buffer to the GL_ARRAY_BUFFER target.
    // * Now, any buffer calls we make (on GL_ARRAY_BUFFER target) will be used to configure the currently bound buffer, which is VBO.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // * function specifically targeted to copy user-defined data into the currently bound buffer.
    // * copies the previously defined vertex data into the buffer's memory
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // +
    // * the vertex shader allows us to specify any input we want in the form of vertex attributes
    // * it means we have to manually specify what part of our input data goes to which vertex attribute in the vertex shader
    // * we can tell OpenGL how it should interpret the vertex data (per vertex attribute) using glVertexAttribPointer:
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    // *    The first parameter specifies which vertex attribute we want to configure.
    // *    Remember that we specified the location of the position vertex attribute in the vertex shader with layout (location = 0).
    // *    This sets the location of the vertex attribute to 0 and since we want to pass data to this vertex attribute, we pass in 0.

    // *    The next argument specifies the size of the vertex attribute. The vertex attribute is a vec3 so it is composed of 3 values.

    // *    The third argument specifies the type of the data which is GL_FLOAT (a vec* in GLSL consists of floating point values).

    // *    The next argument specifies if we want the data to be normalized.
    // *    If we're inputting integer data types (int, byte) and we've set this to GL_TRUE,
    // *    the integer data is normalized to 0 (or -1 for signed data) and 1 when converted to float.

    // *    The fifth argument is known as the stride and tells us the space between consecutive vertex attributes.
    // *    Since the next set of position data is located exactly 3 times the size of a float away we specify that value as the stride.
    // *    Note that since we know that the array is tightly packed (there is no space between the next vertex attribute value)
    // *    we could've also specified the stride as 0 to let OpenGL determine the stride (this only works when values are tightly packed).
    // *    Whenever we have more vertex attributes we have to carefully define the spacing between each vertex attribute.

    // *    The last parameter is of type void* and thus requires a weird cast. This is the offset of where the position data begins in the buffer.
    // *    Since the position data is at the start of the data array this value is just 0.

    // * Each vertex attribute takes its data from memory managed by a VBO and which VBO it takes its data from (there can be multiple VBOs)
    // * is determined by the VBO currently bound to GL_ARRAY_BUFFER when calling glVertexAttribPointer.
    // * Since the previously defined VBO is still bound before calling glVertexAttribPointer vertex attribute 0 is now associated with its vertex data.

    // +

    glEnableVertexAttribArray(0); // * the vertex attribute location as its argument; vertex attributes are disabled by default.
    // * Without enabling the attribute array, OpenGL won't feed the corresponding vertex data to your shader.
    // * Called after vertexAttrib, binds all the properties defined there to VAO, 0 in args refers to layout location (since vertexAttrib has 0)

    // * Usually when you have multiple objects you want to draw,
    // * you first generate/configure all the VAOs (and thus the required VBO and attribute pointers) and store those for later use.
    // * The moment we want to draw one of our objects, we take the corresponding VAO, bind it, draw the object and unbind the VAO again.

    // this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    // uncomment this call to draw in wireframe polygons.
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------

    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // state setting function
        glClear(GL_COLOR_BUFFER_BIT);         // state using function

        // draw our first triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

        // * function that draws primitives using the currently active shader,
        // * the previously defined vertex attribute configuration and with the VBO's vertex data (indirectly bound via the VAO).
        glDrawArrays(GL_TRIANGLES, 0, 3);
        // *    The first argument is the OpenGL primitive type we would like to draw.

        // *    The second argument specifies the starting index of the vertex array we'd like to draw; we just leave this at 0.

        // *    The last argument specifies how many vertices we want to draw, which is 3.

        // glBindVertexArray(0); // no need to unbind it every time

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

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
