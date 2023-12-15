#ifndef SHADER_H
#define SHADER_H

#include <string>

//GLM Libs
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
    //State
    unsigned int ID;

    //constructor
    Shader() {}

    //Set the shader as active
    Shader& Use();

    //Compile the shader
    void Compile(const char* vertexSource, const char* fragmentSource, const char* geometrySource = nullptr); //Note: Geometry source code is optional

    //Utility
    void SetFloat(const char* name, float value, bool useShader = false);
    void SetInteger(const char* name, int value, bool useShader = false);
    void SetVector2f(const char* name, float x, float y, bool useShader = false);
    void SetVector2f(const char* name, const glm::vec2& value, bool useShader = false);
    void SetVector3f(const char* name, float x, float y, float z, bool useShader = false);
    void SetVector3f(const char* name, const glm::vec3& value, bool useShader = false);
    void SetVector4f(const char* name, float x, float y, float z, float w, bool useShader = false);
    void SetVector4f(const char* name, const glm::vec4& value, bool useShader = false);
    void SetMatrix4(const char* name, const glm::mat4& matrix, bool useShader = false);

private:

    //Check if compilation or linking failed and if so print log
    void CheckCompileErrors(unsigned int object, std::string type);

};

#endif