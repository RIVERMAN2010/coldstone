#include "Shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    
    
    std::string vertexCode = readShaderFile(vertexPath);
    std::string fragmentCode = readShaderFile(fragmentPath);

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    if(vShaderCode == nullptr || fShaderCode == nullptr) {
        std::cerr << "Shader is empty!" << std::endl;
        return;
	}

    GLuint vertex, fragment;

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

std::string Shader::readShaderFile(const char* path) {
    std::string line;
	std::stringstream out;
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << path << std::endl;
        return "";
	}
    while (std::getline(file, line)) {
        if(line[0] == '#') {
            std::string includeDirective = "#include ";
            if (line.substr(0, includeDirective.size()) == includeDirective) {
                std::string includePath = line.substr(includeDirective.size());
                includePath.erase(0, includePath.find_first_not_of(" \""));
                includePath.erase(includePath.find_last_not_of(" \"") + 1);
                std::string includedContent = readShaderFile(includePath.c_str());
                out << includedContent << "\n";
                continue;
            }
		}
        out << line << "\n";
	}


	return out.str();
}
void Shader::destroy() const {
    glDeleteProgram(ID);
}

void Shader::use() const {
    glUseProgram(ID);
}
GLuint Shader::getLocation(const std::string& name) const {
    return glGetUniformLocation(ID, name.c_str());
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::checkCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}