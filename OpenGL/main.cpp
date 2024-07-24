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

    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

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

    Shader transparentGrassShader((wchar_t*)L"../res/shaders/transparentGrass.vert", (wchar_t*)L"../res/shaders/transparentGrass.frag");

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

    Shader lightObjectShader((wchar_t*)L"../res/shaders/lightObject.vert", (wchar_t*)L"../res/shaders/lightObject.frag");
    Shader lightSourceShader((wchar_t*)L"../res/shaders/lightSource.vert", (wchar_t*)L"../res/shaders/lightSource.frag");

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

    transparentGrassShader.Use();
    transparentGrassShader.SetInt("diffuseTexture", 0);

    lightObjectShader.Use();
    lightObjectShader.SetInt("material.diffuse", 0);
    lightObjectShader.SetInt("material.specular", 1);
    lightObjectShader.SetFloat("material.shininess", 64.0f);

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

    // Setup model.
    Shader modelLoadingShader((wchar_t*)L"../res/shaders/modelLoading.vert", (wchar_t*)L"../res/shaders/modelLoading.frag");
    Shader outlineShader((wchar_t*)L"../res/shaders/stencil.vert", (wchar_t*)L"../res/shaders/stencil.frag");
    Model backpackModel("../res/models/backpack/backpack.obj");

    // Draw in wireframe mode.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window))
    {
        currentFrame = glfwGetTime();

        processInput(window);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseTexture);

        glm::mat4 viewProjectionMatrix = projection * camera.GetViewMatrix();

        // Render light object.
        glm::mat4 model = glm::mat4(1.0f);

        lightObjectShader.Use();
        lightObjectShader.SetVec3("viewPos", camera.Position);

        // Directional light.
        lightObjectShader.SetVec3("directionalLight.direction", -0.2f, -1.0f, -0.3f);
        lightObjectShader.SetVec3("directionalLight.ambient", 0.05f, 0.05f, 0.05f);
        lightObjectShader.SetVec3("directionalLight.diffuse", 0.4f, 0.4f, 0.4f);
        lightObjectShader.SetVec3("directionalLight.specular", 0.5f, 0.5f, 0.5f);

        // Point light.

        lightObjectShader.SetVec3("pointLight.position", lightSourcePos);

        lightObjectShader.SetVec3("pointLight.ambient", 0.1f, 0.1f, 0.1f);
        lightObjectShader.SetVec3("pointLight.diffuse", 0.5f, 0.5f, 0.5f);
        lightObjectShader.SetVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);

        lightObjectShader.SetFloat("pointLight.constant", 1.0f);
        lightObjectShader.SetFloat("pointLight.linear", 0.09f);
        lightObjectShader.SetFloat("pointLight.quadratic", 0.032f);

        // Spotlight.

        lightObjectShader.SetVec3("spotLight.position", camera.Position);
        lightObjectShader.SetVec3("spotLight.direction", camera.Front);

        lightObjectShader.SetVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        lightObjectShader.SetVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        lightObjectShader.SetVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);

        lightObjectShader.SetFloat("spotLight.constant", 1.0f);
        lightObjectShader.SetFloat("spotLight.linear", 0.09f);
        lightObjectShader.SetFloat("spotLight.quadratic", 0.032f);

        lightObjectShader.SetFloat("spotLight.innerCutOff", glm::cos(glm::radians(12.5f)));
        lightObjectShader.SetFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

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
            lightObjectShader.SetMat4("mvpMatrix", viewProjectionMatrix * model);
            lightObjectShader.SetMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Render light source.
        lightSourceShader.Use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightSourcePos);
        model = glm::scale(model, glm::vec3(0.2f));
        lightSourceShader.SetMat4("mvpMatrix", viewProjectionMatrix * model);

        glBindVertexArray(lightSourceVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Render model.
        modelLoadingShader.Use();

        // Enable stencil test.
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);

        // Render the loaded model.
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(4.0f, 0.0f, -5.0f));
        modelLoadingShader.SetMat4("mvpMatrix", viewProjectionMatrix * model);
        backpackModel.Draw(modelLoadingShader);

        // Disable stencil writing.
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);

        outlineShader.Use();

        outlineShader.SetMat4("mvpMatrix", viewProjectionMatrix * model);
        outlineShader.SetVec4("outlineColor", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        outlineShader.SetFloat("scale", 0.02f);
        backpackModel.Draw(outlineShader);

        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glEnable(GL_DEPTH_TEST);

        // Render grass.
        transparentGrassShader.Use();
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
            transparentGrassShader.SetMat4("mvpMatrix", viewProjectionMatrix * model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        wprintf(L"FPS: %.f      frame time: %f\n", 1000 / (deltaTime * 1000), deltaTime);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    return 0;
}