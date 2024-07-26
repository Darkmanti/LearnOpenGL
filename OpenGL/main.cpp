#include "glad/glad.h"
#include "glfw3.h"

#include <stdio.h>
#include <math.h>

#include "file_api.h"
#include "shader.h"
#include "camera.h"

#include "model.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

const int windowWidth = 1280;
const int windowHeight = 720;

double deltaTime = 0.0, lastFrame = 0.0, currentFrame = 0.0;

// camera
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
float lastX = windowWidth / 2.0f;
float lastY = windowHeight / 2.0f;
bool firstMouse = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

Shader* frameBufferShader;
Shader* transparentGrassShader;
Shader* modelLoadingShader;
Shader* outlineShader;
Shader* lightObjectShader;
Shader* lightSourceShader;
Shader* skyboxShader;
Shader* reflectedObjectShader;
Shader* geometryTestShader;
Shader* modelNormalsShader;
Shader* modelLoadingDefaultShader;
Shader* modelLoadingInstancedShader;

void RecompileShaders()
{
    if (frameBufferShader != NULL)
        frameBufferShader->~Shader();

    if (transparentGrassShader != NULL)
        transparentGrassShader->~Shader();

    if (modelLoadingShader != NULL)
        modelLoadingShader->~Shader();

    if (outlineShader != NULL)
        outlineShader->~Shader();

    if (lightObjectShader != NULL)
        lightObjectShader->~Shader();

    if (lightObjectShader != NULL)
        lightObjectShader->~Shader();

    if (skyboxShader != NULL)
        skyboxShader->~Shader();

    if (reflectedObjectShader != NULL)
        reflectedObjectShader->~Shader();

    if (geometryTestShader != NULL)
        geometryTestShader->~Shader();

    if (modelNormalsShader != NULL)
        modelNormalsShader->~Shader();

    if (modelLoadingDefaultShader != NULL)
        modelLoadingDefaultShader->~Shader();

    if (modelLoadingInstancedShader != NULL)
        modelLoadingInstancedShader->~Shader();

    frameBufferShader = new Shader((wchar_t*)L"../res/shaders/frameBuffer.vert", (wchar_t*)L"../res/shaders/frameBuffer.frag");
    transparentGrassShader = new Shader((wchar_t*)L"../res/shaders/transparentGrass.vert", (wchar_t*)L"../res/shaders/transparentGrass.frag");
    modelLoadingShader = new Shader((wchar_t*)L"../res/shaders/modelLoading.vert", (wchar_t*)L"../res/shaders/modelLoading.frag", (wchar_t*)L"../res/shaders/modelLoading.geom");
    modelNormalsShader = new Shader((wchar_t*)L"../res/shaders/modelNormals.vert", (wchar_t*)L"../res/shaders/modelNormals.frag", (wchar_t*)L"../res/shaders/modelNormals.geom");
    outlineShader = new Shader((wchar_t*)L"../res/shaders/stencil.vert", (wchar_t*)L"../res/shaders/stencil.frag");
    lightObjectShader = new Shader((wchar_t*)L"../res/shaders/lightObject.vert", (wchar_t*)L"../res/shaders/lightObject.frag");
    lightSourceShader = new Shader((wchar_t*)L"../res/shaders/lightSource.vert", (wchar_t*)L"../res/shaders/lightSource.frag");
    skyboxShader = new Shader((wchar_t*)L"../res/shaders/skybox.vert", (wchar_t*)L"../res/shaders/skybox.frag");
    reflectedObjectShader = new Shader((wchar_t*)L"../res/shaders/reflectedObject.vert", (wchar_t*)L"../res/shaders/reflectedObject.frag");
    geometryTestShader = new Shader((wchar_t*)L"../res/shaders/geometryTest.vert", (wchar_t*)L"../res/shaders/geometryTest.frag", (wchar_t*)L"../res/shaders/geometryTest.geom");
    modelLoadingDefaultShader = new Shader((wchar_t*)L"../res/shaders/modelLoadingDefault.vert", (wchar_t*)L"../res/shaders/modelLoadingDefault.frag");
    modelLoadingInstancedShader = new Shader((wchar_t*)L"../res/shaders/modelLoadingInstanced.vert", (wchar_t*)L"../res/shaders/modelLoadingInstanced.frag");
}

bool isShaderButtonReleased = true;

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, (float)deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, (float)deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, (float)deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, (float)deltaTime);

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (isShaderButtonReleased)
        {
            isShaderButtonReleased = false;
            RecompileShaders();
        }
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE)
    {
        isShaderButtonReleased = true;
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int LoadCubemap(std::vector<std::string> faces)
{
    stbi_set_flip_vertically_on_load(false);

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            wprintf(L"Cubemap texture failed to load at path: %S \n", faces[i].c_str());
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    stbi_set_flip_vertically_on_load(true);

    return textureID;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        wprintf(L"Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Set 0 to disable vsync.
    glfwSwapInterval(0);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        wprintf(L"Failed to initialize GLAD");
        return -1;
    }

    glViewport(0, 0, windowWidth, windowHeight);

    // Creating frame buffer.
    unsigned int frameBuffer;
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    unsigned int frameBufferTexture;
    glGenTextures(1, &frameBufferTexture);
    glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTexture, 0);

    // Creating depth stencil buffer.
    unsigned int depthStencilBufferTexture;
    glGenTextures(1, &depthStencilBufferTexture);
    glBindTexture(GL_TEXTURE_2D, depthStencilBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, windowWidth, windowHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilBufferTexture, 0);

    // Creating render buffer.
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        wprintf(L"ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //glFrontFace(GL_CCW);

    float quadVertices[] =
    {   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    float transparentVertices[] =
    {
        // Positions.        // Texture coords.
        0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

        0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
        1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };

    float cubeVertices[] =
    {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f
    };

    float skyboxVertices[] =
    {
        // Positions.
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    float points[] =
    {
        -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // top-left
         0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // top-right
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
        -0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // bottom-left
    };

    RecompileShaders();

    // Points VAO.
    unsigned int pointsVAO, pointsVBO;
    glGenVertexArrays(1, &pointsVAO);
    glGenBuffers(1, &pointsVBO);
    glBindVertexArray(pointsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    // Skybox VAO.
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Screen quad VAO.
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // Grass.

    unsigned int grassVBO, grassVAO;
    glGenBuffers(1, &grassVBO);
    glGenVertexArrays(1, &grassVAO);

    glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);

    glBindVertexArray(grassVAO);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // TextCoord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Light objects.

    unsigned int VBO, lightObjectVAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &lightObjectVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glBindVertexArray(lightObjectVAO);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // TextCoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Light source.

    unsigned int lightSourceVAO;
    glGenVertexArrays(1, &lightSourceVAO);
    glBindVertexArray(lightSourceVAO);

    // Bind to match the patter.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Light source VAO.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int diffuseTexture, specularTexture;
    int width, height, channels;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("../res/container2.png", &width, &height, &channels, 3);

    glGenTextures(1, &diffuseTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    data = stbi_load("../res/container2_specular.png", &width, &height, &channels, 3);

    glGenTextures(1, &specularTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    unsigned int transparentGrassTexture;

    stbi_set_flip_vertically_on_load(false);
    data = stbi_load("../res/grass.png", &width, &height, &channels, 4);

    glGenTextures(1, &transparentGrassTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, transparentGrassTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    stbi_set_flip_vertically_on_load(true);

    std::vector<std::string> faces
    {
        "../res/skybox/right.jpg",
        "../res/skybox/left.jpg",
        "../res/skybox/top.jpg",
        "../res/skybox/bottom.jpg",
        "../res/skybox/front.jpg",
        "../res/skybox/back.jpg"
    };

    unsigned int skyboxTexture = LoadCubemap(faces);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(camera.Zoom), (float)windowWidth / (float)windowHeight, 0.01f, 1000.0f);

    glm::vec3 lightSourcePos(1.0f, 1.0f, 4.0f);

    // Setup transparent grass.
    std::vector<glm::vec3> grass;
    grass.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
    grass.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
    grass.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
    grass.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
    grass.push_back(glm::vec3(0.5f, 0.0f, -0.6f));

    // Setup models.
    Model backpackModel("../res/models/backpack/backpack.obj");

    Model planetModel("../res/models/planet/planet.obj");
    Model rockModel("../res/models/rock/rock.obj");

    glm::vec3 planetCenter = glm::vec3(glm::vec3(20.0f, 0.0f, -30.0f));

    int amount = 10000;
    glm::mat4* modelMatrices;
    modelMatrices = new glm::mat4[amount];

    float phi = glm::pi<float>() * (sqrt(5.0f) * 1.0f);
    float magnitude = 15.0f;

    for (int i = 0; i < amount; i++)
    {
        float y = (1.0f - (i / (float)(amount - 1.0f)) * 2.0f);
        float radius = sqrt(1 - y * y);

        y *= magnitude;

        float theta = phi * i;

        float x = ((cos(theta) * radius) * magnitude) + planetCenter.x;
        float z = ((sin(theta) * radius) * magnitude) + planetCenter.z;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, y, z));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        modelMatrices[i] = model;
    }

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

    for (unsigned int i = 0; i < rockModel.Meshes.size(); i++)
    {
        unsigned int VAO = rockModel.Meshes[i].VAO;
        glBindVertexArray(VAO);

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }

    // Draw in wireframe mode.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window))
    {
        currentFrame = glfwGetTime();

        processInput(window);

        glEnable(GL_DEPTH_TEST);

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseTexture);

        glm::mat4 viewProjectionMatrix = projection * camera.GetViewMatrix();

        // Render skybox.
        glDepthFunc(GL_LEQUAL);
        skyboxShader->Use();
        skyboxShader->SetInt("skybox", 0);
        skyboxShader->SetMat4("view", glm::mat4(glm::mat3(camera.GetViewMatrix())));
        skyboxShader->SetMat4("projection", projection);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        // Render light object.
        glm::mat4 model = glm::mat4(1.0f);

        lightObjectShader->Use();
        lightObjectShader->SetInt("material.diffuse", 0);
        lightObjectShader->SetInt("material.specular", 1);
        lightObjectShader->SetFloat("material.shininess", 64.0f);

        lightObjectShader->SetVec3("viewPos", camera.Position);

        // Directional light.
        lightObjectShader->SetVec3("directionalLight.direction", -0.2f, -1.0f, -0.3f);
        lightObjectShader->SetVec3("directionalLight.ambient", 0.05f, 0.05f, 0.05f);
        lightObjectShader->SetVec3("directionalLight.diffuse", 0.4f, 0.4f, 0.4f);
        lightObjectShader->SetVec3("directionalLight.specular", 0.5f, 0.5f, 0.5f);

        // Point light.

        lightObjectShader->SetVec3("pointLight.position", lightSourcePos);

        lightObjectShader->SetVec3("pointLight.ambient", 0.1f, 0.1f, 0.1f);
        lightObjectShader->SetVec3("pointLight.diffuse", 0.5f, 0.5f, 0.5f);
        lightObjectShader->SetVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);

        lightObjectShader->SetFloat("pointLight.constant", 1.0f);
        lightObjectShader->SetFloat("pointLight.linear", 0.09f);
        lightObjectShader->SetFloat("pointLight.quadratic", 0.032f);

        // Spotlight.

        lightObjectShader->SetVec3("spotLight.position", camera.Position);
        lightObjectShader->SetVec3("spotLight.direction", camera.Front);

        lightObjectShader->SetVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        lightObjectShader->SetVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        lightObjectShader->SetVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);

        lightObjectShader->SetFloat("spotLight.constant", 1.0f);
        lightObjectShader->SetFloat("spotLight.linear", 0.09f);
        lightObjectShader->SetFloat("spotLight.quadratic", 0.032f);

        lightObjectShader->SetFloat("spotLight.innerCutOff", glm::cos(glm::radians(12.5f)));
        lightObjectShader->SetFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        // Activate texture.

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularTexture);

        // Disable stencil writing.
        glStencilMask(0x00);

        glBindVertexArray(lightObjectVAO);

        for (int i = 0; i < 10; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(2.0f * i, 0.0f, 0.0f));
            lightObjectShader->SetMat4("mvpMatrix", viewProjectionMatrix * model);
            lightObjectShader->SetMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Render light source.
        lightSourceShader->Use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightSourcePos);
        model = glm::scale(model, glm::vec3(0.2f));
        lightSourceShader->SetMat4("mvpMatrix", viewProjectionMatrix * model);

        glBindVertexArray(lightSourceVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Render model.
        modelLoadingDefaultShader->Use();

        // Enable stencil test.
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);

        // Render the loaded model.
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(4.0f, 0.0f, -5.0f));
        modelLoadingDefaultShader->SetMat4("mvpMatrix", viewProjectionMatrix * model);
        backpackModel.Draw(*modelLoadingDefaultShader);

        // Render outline.
        // Disable stencil writing.
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);

        outlineShader->Use();

        outlineShader->SetMat4("mvpMatrix", viewProjectionMatrix * model);
        outlineShader->SetVec4("outlineColor", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        outlineShader->SetFloat("scale", 0.02f);
        backpackModel.Draw(*outlineShader);

        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glEnable(GL_DEPTH_TEST);

        // Render model with explode.
        modelLoadingShader->Use();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-4.0f, 0.0f, -5.0f));
        modelLoadingShader->SetMat4("mvpMatrix", viewProjectionMatrix * model);
        modelLoadingShader->SetFloat("time", (float)glfwGetTime());
        backpackModel.Draw(*modelLoadingShader);

        // Render model with normals.
        modelLoadingShader->Use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(12.0f, 0.0f, -5.0f));
        modelLoadingShader->SetMat4("mvpMatrix", viewProjectionMatrix * model);
        modelLoadingShader->SetFloat("time", (3.0f * glm::pi<float>()) / 2.0f);
        backpackModel.Draw(*modelLoadingShader);

        modelNormalsShader->Use();

        modelNormalsShader->SetMat4("model", model);
        modelNormalsShader->SetMat4("projection", projection);
        modelNormalsShader->SetMat4("view", camera.GetViewMatrix());
        backpackModel.Draw(*modelNormalsShader);

        // Render instanced models.
        modelLoadingDefaultShader->Use();

        // Draw planet.
        model = glm::mat4(1.0f);
        model = glm::translate(model, planetCenter);
        model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
        modelLoadingDefaultShader->SetMat4("mvpMatrix", viewProjectionMatrix * model);
        planetModel.Draw(*modelLoadingDefaultShader);

        // Draw meteorites.
        modelLoadingInstancedShader->Use();
        modelLoadingInstancedShader->SetInt("texture_diffuse1", 0);
        modelLoadingInstancedShader->SetMat4("view", camera.GetViewMatrix());
        modelLoadingInstancedShader->SetMat4("projection", projection);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rockModel.LoadedTextures[0].id);
        for (int i = 0; i < rockModel.Meshes.size(); i++)
        {
            glBindVertexArray(rockModel.Meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, (unsigned int)(rockModel.Meshes[i].indices.size()), GL_UNSIGNED_INT, 0, amount);
            glBindVertexArray(0);
        }

        // Render reflected object.
        reflectedObjectShader->Use();
        reflectedObjectShader->SetInt("skybox", 0);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.0f, 0.0f));
        reflectedObjectShader->SetMat4("mvpMatrix", viewProjectionMatrix * model);
        reflectedObjectShader->SetMat4("model", model);
        reflectedObjectShader->SetVec3("viewPos", camera.Position);

        glBindVertexArray(lightObjectVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Render grass.
        transparentGrassShader->Use();
        transparentGrassShader->SetInt("diffuseTexture", 0);

        glBindVertexArray(grassVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, transparentGrassTexture);

        std::map<float, glm::vec3> sorted;
        for (unsigned int i = 0; i < grass.size(); i++)
        {
            float distance = glm::length(camera.Position - grass[i]);
            sorted[distance] = grass[i];
        }

        for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, it->second);
            transparentGrassShader->SetMat4("mvpMatrix", viewProjectionMatrix * model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // Render points.
        // geometryTestShader->Use();
        // glBindVertexArray(pointsVAO);
        // glDrawArrays(GL_POINTS, 0, 4);
        // glBindVertexArray(0);

        // Second pass.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        frameBufferShader->Use();
        glBindVertexArray(quadVAO);
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        double fps = 1000 / (deltaTime * 1000);
        double frameTime = deltaTime;

        char title[256];
        memset(title, 0, 256);
        snprintf(title, 256, "FPS: %.f   frame time: %f\n", 1000 / (deltaTime * 1000), deltaTime);

        glfwSetWindowTitle(window, title);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    return 0;
}