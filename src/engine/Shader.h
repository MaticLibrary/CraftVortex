#pragma once
#include "Common.h"

class Shader {
public:
    Shader();
    ~Shader();

    bool loadFromSource(const std::string& vertexSrc, const std::string& fragmentSrc);
    bool loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);

    void use() const;

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

    GLuint getProgramID() const { return m_programID; }

private:
    GLuint m_programID = 0;
    mutable std::unordered_map<std::string, GLint> m_uniformLocations;

    GLint getUniformLocation(const std::string& name) const;
    static GLuint compileShader(GLenum type, const std::string& source);
};
