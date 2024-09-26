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

#include "stb_image.h"

unsigned int LoadTexture(const char* path, bool gammaCorrection);
void RenderCube();
void RenderQuad();

const int WindowWidth = 1920;
const int WindowHeight = 1080;

float exposure = 1.0f;

double deltaTime = 0.0, lastFrame = 0.0, currentFrame = 0.0;

// camera
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
float lastX = WindowWidth / 2.0f;
float lastY = WindowHeight / 2.0f;
bool firstMouse = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

Shader* geometryPassShader;
Shader* lightPassShader;
Shader* lightBoxShader;

void RecompileShaders()
{
    if (geometryPassShader != NULL)
        geometryPassShader->~Shader();

    if (lightPassShader != NULL)
        lightPassShader->~Shader();

    if (lightBoxShader != NULL)
        lightBoxShader->~Shader();

    geometryPassShader = new Shader((wchar_t*)L"../res/shaders/gBuffer.vert", (wchar_t*)L"../res/shaders/gBuffer.frag");
    lightPassShader = new Shader((wchar_t*)L"../res/shaders/deferredRendering.vert", (wchar_t*)L"../res/shaders/deferredRendering.frag");
    lightBoxShader = new Shader((wchar_t*)L"../res/shaders/lightBox.vert", (wchar_t*)L"../res/shaders/lightBox.frag");
}

bool isShaderButtonReleased = true;
bool isEButtonOnePressed = true;

bool isSomethingEnable = true;

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

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (isEButtonOnePressed)
        {
            isEButtonOnePressed = false;
            isSomethingEnable = !isSomethingEnable;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE)
    {
        isEButtonOnePressed = true;
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

// meshes
unsigned int planeVAO;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WindowWidth, WindowHeight, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        wprintf(L"Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Set 0 to disable vsync.
    glfwSwapInterval(1);

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

    glViewport(0, 0, WindowWidth, WindowHeight);

    RecompileShaders();

    // configure g-buffer framebuffer
    // ------------------------------
    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    unsigned int gPosition, gNormal, gAlbedoSpec;

    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WindowWidth, WindowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WindowWidth, WindowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WindowWidth, WindowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WindowWidth, WindowHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        wprintf(L"Framebuffer not complete!\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // lighting info
    // -------------
    const unsigned int NR_LIGHTS = 32;
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;
    srand(13);
    for (unsigned int i = 0; i < NR_LIGHTS; i++)
    {
        // calculate slightly random offsets
        float xPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
        float yPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 4.0);
        float zPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
        lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
        // also calculate random color
        float rColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
        float gColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
        float bColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
        lightColors.push_back(glm::vec3(rColor, gColor, bColor));
    }

    stbi_set_flip_vertically_on_load(true);
    Model backpack(std::string("../res/models/backpack/backpack.obj"));
    std::vector<glm::vec3> objectPositions;
    objectPositions.push_back(glm::vec3(-3.0, -0.5, -3.0));
    objectPositions.push_back(glm::vec3(0.0, -0.5, -3.0));
    objectPositions.push_back(glm::vec3(3.0, -0.5, -3.0));
    objectPositions.push_back(glm::vec3(-3.0, -0.5, 0.0));
    objectPositions.push_back(glm::vec3(0.0, -0.5, 0.0));
    objectPositions.push_back(glm::vec3(3.0, -0.5, 0.0));
    objectPositions.push_back(glm::vec3(-3.0, -0.5, 3.0));
    objectPositions.push_back(glm::vec3(0.0, -0.5, 3.0));
    objectPositions.push_back(glm::vec3(3.0, -0.5, 3.0));

    // Draw in wireframe mode.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window))
    {
        currentFrame = glfwGetTime();

        processInput(window);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WindowWidth / (float)WindowHeight, 0.01f, 1000.0f);

        glEnable(GL_DEPTH_TEST);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. geometry pass: render scene's geometry/color data into gbuffer
        // -----------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 model = glm::mat4(1.0f);
        geometryPassShader->Use();
        geometryPassShader->SetMat4("projection", projection);
        geometryPassShader->SetMat4("view", view);
        for (unsigned int i = 0; i < objectPositions.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, objectPositions[i]);
            model = glm::scale(model, glm::vec3(0.5f));
            geometryPassShader->SetMat4("model", model);
            backpack.Draw(*geometryPassShader);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
        // -----------------------------------------------------------------------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        lightPassShader->Use();
        lightPassShader->SetInt("gPosition", 0);
        lightPassShader->SetInt("gNormal", 1);
        lightPassShader->SetInt("gAlbedoSpec", 2);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

        // send light relevant uniforms
        for (unsigned int i = 0; i < lightPositions.size(); i++)
        {
            char positionName[256];
            memset(positionName, 0, 256);
            snprintf(positionName, 256, "lights[%d].Position", i);

            char colorName[256];
            memset(colorName, 0, 256);
            snprintf(colorName, 256, "lights[%d].Color", i);

            char linearName[256];
            memset(linearName, 0, 256);
            snprintf(linearName, 256, "lights[%d].Linear", i);

            char attenuationName[256];
            memset(attenuationName, 0, 256);
            snprintf(attenuationName, 256, "lights[%d].Quadratic", i);

            lightPassShader->SetVec3(positionName, lightPositions[i]);
            lightPassShader->SetVec3(colorName, lightColors[i]);

            // update attenuation parameters and calculate radius
            const float linear = 0.01f;
            const float quadratic = 1.8f;
            lightPassShader->SetFloat(linearName, linear);
            lightPassShader->SetFloat(attenuationName, quadratic);
        }

        lightPassShader->SetVec3("viewPos", camera.Position);
        // finally render quad
        RenderQuad();

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        //// 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
        //// ----------------------------------------------------------------------------------
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
        //// blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
        //// the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the
        //// depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
        glBlitFramebuffer(0, 0, WindowWidth, WindowHeight, 0, 0, WindowWidth, WindowHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 3. render lights on top of scene
        // --------------------------------
        lightBoxShader->Use();
        lightBoxShader->SetMat4("projection", projection);
        lightBoxShader->SetMat4("view", view);
        for (unsigned int i = 0; i < lightPositions.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, lightPositions[i]);
            model = glm::scale(model, glm::vec3(0.125f));
            lightBoxShader->SetMat4("model", model);
            lightBoxShader->SetVec3("lightColor", lightColors[i]);
            RenderCube();
        }

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

// RenderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void RenderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
             // bottom face
             -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
              1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
              1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
              1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
             -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             // top face
             -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
              1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
              1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
              1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// RenderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void RenderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

unsigned int LoadTexture(char const* path, bool gammaCorrection)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum internalFormat = {};
        GLenum dataFormat = {};

        if (nrComponents == 1)
        {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3)
        {
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        wprintf(L"Texture failed to load at path: %s\n", path);
        stbi_image_free(data);
    }

    return textureID;
}