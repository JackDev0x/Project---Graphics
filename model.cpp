#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "lodepng.h"

class Model {
public:
    GLuint tex;
    GLuint normalTex;
    std::vector<glm::vec4> verts;
    std::vector<glm::vec4> norms;
    std::vector<unsigned int> indices;
    std::vector<glm::vec2> texCoords;
};