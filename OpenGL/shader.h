#pragma once

#include "glad/glad.h"

#include "file_api.h"

#include "glm.hpp"
#include "gtc/type_ptr.hpp"

#include <string>

enum ShaderType
{
    VertexShader = 0,
    FragmentShader,
    GeometryShader,
    ProgramShader
};

class Shader
{
public:
    
    // Program id
    unsigned int ID;

    // Constructor
    Shader(wchar_t* vertexPath, wchar_t* fragmentPath, wchar_t* geometryPath = NULL)
    {
        char* vsShaderCode= (char*)Win32ReadFileToBuffer(vertexPath, NULL, NULL, false);
        char* fsShaderCode = (char*)Win32ReadFileToBuffer(fragmentPath, NULL, NULL, false);

        if (geometryPath != NULL)
        {
            char* gsShaderCode = (char*)Win32ReadFileToBuffer(geometryPath, NULL, NULL, false);

            // geometry Shader
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gsShaderCode, NULL);
            glCompileShader(geometry);
            CheckCompileErrors(geometry, GeometryShader);

            free(gsShaderCode);
        }

        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vsShaderCode, NULL);
        glCompileShader(vertex);
        CheckCompileErrors(vertex, VertexShader);

        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fsShaderCode, NULL);
        glCompileShader(fragment);
        CheckCompileErrors(fragment, FragmentShader);

        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);

        if (geometryPath != NULL)
            glAttachShader(ID, geometry);

        glLinkProgram(ID);
        CheckCompileErrors(ID, ProgramShader);

        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);

        if (geometryPath != NULL)
            glDeleteShader(geometry);

        free(vsShaderCode);
        free(fsShaderCode);
    }

    // Activate shader
    void Use()
    {
        glUseProgram(ID);
    }

    // Destructor.
    ~Shader()
    {
        glDeleteProgram(ID);
    }

    // Utility uniform functions

    void SetBool(const char* name, bool value)
    {
        glUniform1i(glGetUniformLocation(ID, name), (int)value);
    }

    void SetInt(const char* name, int value)
    {
        glUniform1i(glGetUniformLocation(ID, name), value);
    }

    void SetFloat(const char* name, float value)
    {
        glUniform1f(glGetUniformLocation(ID, name), value);
    }

    void SetMat4(const char* name, glm::mat4 mat4)
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, glm::value_ptr(mat4));
    }

    void SetBool(const std::string name, bool value)
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }

    void SetInt(const std::string name, int value)
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void SetFloat(const std::string name, float value)
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void SetMat4(const std::string name, glm::mat4 mat4)
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat4));
    }

    void SetVec4(const char* name, glm::vec4 vec4)
    {
        glUniform4fv(glGetUniformLocation(ID, name), 1, &vec4[0]);
    }

    void SetVec4(const char* name, float x, float y, float z, float w)
    {
        glUniform4f(glGetUniformLocation(ID, name), x, y, z, w);
    }

    void SetVec4(const std::string name, glm::vec4 vec4)
    {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &vec4[0]);
    }

    void SetVec4(const std::string name, float x, float y, float z, float w)
    {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }

    void SetVec3(const char* name, glm::vec3 vec3)
    {
        glUniform3fv(glGetUniformLocation(ID, name), 1, &vec3[0]);
    }

    void SetVec3(const char* name, float x, float y, float z)
    {
        glUniform3f(glGetUniformLocation(ID, name), x, y, z);
    }

    void SetVec3(const std::string name, glm::vec3 vec3)
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &vec3[0]);
    }

    void SetVec3(const std::string name, float x, float y, float z)
    {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }

    void SetVec2(const char* name, glm::vec2 vec2)
    {
        glUniform2fv(glGetUniformLocation(ID, name), 1, &vec2[0]);
    }

    void SetVec2(const char* name, float x, float y)
    {
        glUniform2f(glGetUniformLocation(ID, name), x, y);
    }

    void SetVec2(const std::string name, glm::vec2 vec2)
    {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &vec2[0]);
    }

    void SetVec2(const std::string name, float x, float y)
    {
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }

private:

    unsigned int vertex, fragment, geometry;

    void CheckCompileErrors(unsigned int shader, ShaderType type)
    {
        int success;
        char infoLog[2048];
        if (type != ProgramShader)
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 2048, NULL, infoLog);
                switch (type)
                {
                    case VertexShader:
                    {
                        wprintf(L"ERROR Vertex Shader compilation:\n    %hs \n", infoLog);
                    } break;

                    case FragmentShader:
                    {
                        wprintf(L"ERROR Fragment Shader compilation:\n    %hs \n", infoLog);
                    } break;

                    case GeometryShader:
                    {
                        wprintf(L"ERROR Geometry Shader compilation:\n    %hs \n", infoLog);
                    } break;

                    default:
                    {
                        wprintf(L"ERROR unexpected shader type\n");
                    }
                }
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 2048, NULL, infoLog);
                wprintf(L"ERROR Program linking compilation:\n    %hs \n", infoLog);
            }
        }
    }
};