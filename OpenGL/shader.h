#pragma once

#include "glad/glad.h"

#include "file_api.h"

#include "glm.hpp"
#include "gtc/type_ptr.hpp"

enum ShaderType
{
    VertexShader = 0,
    FragmentShader,
    ProgramShader
};

class Shader
{
public:
    
    // Program id
    unsigned int ID;

    // Constructor
    Shader(wchar_t* vertexPath, wchar_t* fragmentPath)
    {
        char* vsShaderCode= (char*)Win32ReadFileToBuffer(vertexPath, NULL, NULL, false);
        char* fsShaderCode = (char*)Win32ReadFileToBuffer(fragmentPath, NULL, NULL, false);

        // 2. compile shaders
        unsigned int vertex, fragment;

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
        glLinkProgram(ID);
        CheckCompileErrors(ID, ProgramShader);

        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);

        free(vsShaderCode);
        free(fsShaderCode);
    }

    // Activate shader
    void Use()
    {
        glUseProgram(ID);
    }

    // Utility uniform functions
    void SetBool(char* name, bool value)
    {
        glUniform1i(glGetUniformLocation(ID, name), (int)value);
    }

    void SetBool(const char* name, bool value)
    {
        SetBool((char*)name, value);
    }

    void SetInt(char* name, int value)
    {
        glUniform1i(glGetUniformLocation(ID, name), value);
    }

    void SetInt(const char* name, int value)
    {
        SetInt((char*)name, value);
    }

    void SetFloat(char* name, float value)
    {
        glUniform1f(glGetUniformLocation(ID, name), value);
    }

    void SetFloat(const char* name, float value)
    {
        SetFloat((char*)name, value);
    }

    void SetMat4(char* name, glm::mat4 mat4)
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, glm::value_ptr(mat4));
    }

    void SetMat4(const char* name, glm::mat4 mat4)
    {
        SetMat4((char*)name, mat4);
    }

private:

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

                    default:
                    {
                        wprintf(L"ERROR unexpected shader type");
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