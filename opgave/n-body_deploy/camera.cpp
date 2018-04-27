#include "camera.h"

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

glm::mat4 g_projection_mat(1.f);
glm::mat4 g_trans_mat(1.f);
glm::mat4 g_rot_mat(1.f);
glm::mat4 g_mvp_mat;


void init_camera()
{
    g_projection_mat = glm::perspective(glm::radians(45.0f), 4.0f / 4.0f, 1.f, 1000.f);
    moveCamera(0.0f, .0f, -10.f);
}

const float* getMVPMat()
{
    g_mvp_mat = g_projection_mat * g_trans_mat * g_rot_mat * glm::scale(glm::mat4(1.0), glm::vec3(0.10, 0.10, 0.10));
    return glm::value_ptr(g_mvp_mat);
}

void moveCamera(float dx, float dy, float dz)
{
    g_trans_mat = glm::translate(g_trans_mat, glm::vec3(dx, dy, dz));
}

void rotateCamera(float pan, float tilt)
{
    pan /= 100;
    tilt /= 100;
    g_rot_mat = glm::rotate(g_rot_mat, pan, glm::vec3(0.f, 1.f, 0.f));
    g_rot_mat = glm::rotate(g_rot_mat, tilt, glm::vec3(1.f, 0.f, 0.f));

}
