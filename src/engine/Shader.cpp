#include "Shader.h"
#include <fstream>
#include <sstream>

Shader::Shader() {}

Shader::~Shader() {
    if (m_programID != 0) {
        glDeleteProgram(m_programID);
    }
}

GLuint Shader::compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader compile error (" << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << "): " << infoLog << "\n";
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool Shader::loadFromSource(const std::string& vertexSrc, const std::string& fragmentSrc) {
    GLuint vert = compileShader(GL_VERTEX_SHADER, vertexSrc);
    if (!vert) return false;

    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);
    if (!frag) {
        glDeleteShader(vert);
        return false;
    }

    m_programID = glCreateProgram();
    glAttachShader(m_programID, vert);
    glAttachShader(m_programID, frag);
    glLinkProgram(m_programID);

    GLint success;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(m_programID, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader link error: " << infoLog << "\n";
        glDeleteShader(vert);
        glDeleteShader(frag);
        return false;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    return true;
}

bool Shader::loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::ifstream vFile(vertexPath);
    std::ifstream fFile(fragmentPath);

    if (!vFile.is_open() || !fFile.is_open()) {
        std::cerr << "Could not open shader files: " << vertexPath << ", " << fragmentPath << "\n";
        return false;
    }

    std::stringstream vStream, fStream;
    vStream << vFile.rdbuf();
    fStream << fFile.rdbuf();

    return loadFromSource(vStream.str(), fStream.str());
}

void Shader::use() const {
    glUseProgram(m_programID);
}

GLint Shader::getUniformLocation(const std::string& name) const {
    auto it = m_uniformLocations.find(name);
    if (it != m_uniformLocations.end()) {
        return it->second;
    }
    GLint loc = glGetUniformLocation(m_programID, name.c_str());
    m_uniformLocations[name] = loc;
    return loc;
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(getUniformLocation(name), (int)value);
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat));
}
