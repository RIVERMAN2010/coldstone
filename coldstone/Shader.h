#pragma once
#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    Shader(const std::string& vertexSource, const std::string& fragmentSource, bool isSourceCode = false);
    ~Shader();

    void use() const;
    void destroy() const;
    unsigned int GetID() const { return ID; }

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

    int getLocation(const std::string& name) const;

private:
    unsigned int ID;
    std::string readShaderFile(const char* path);
    void checkCompileErrors(unsigned int shader, std::string type);
};