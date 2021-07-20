#ifndef CVLIB_CAMERA_H
#define CVLIB_CAMERA_H
#include <Utils.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <iostream>
#include <tuple>
#include <filesystem>
#include <cstdio>
#include <string>
#include <string_view>
namespace cvlib{
    class Camera {
        glm::vec3 m_Eye;
        glm::vec3 m_LookAt;
        glm::vec3 m_Vup;
        float     m_FovY;
        float     m_Aspect;
        float     m_Focal;
    public:
        Camera()noexcept :m_Eye{}, m_LookAt{}, m_Vup{}, m_FovY{}, m_Aspect{}, m_Focal{}{}
        Camera(const glm::vec3& eye,
               const glm::vec3& lookAt,
               const glm::vec3& vup,
               const float      fovY,
               const float      aspect,
               const float      focal = 1.0f)noexcept
            :m_Eye{ eye },
            m_LookAt{ lookAt },
            m_Vup{ vup },
            m_FovY{ fovY },
            m_Aspect{ aspect },
            m_Focal{ focal }{}
        //Direction
        inline glm::vec3 getDirection()const noexcept {
            return glm::normalize(m_LookAt - m_Eye);
        }
        inline void   setDirection(const glm::vec3& direction)noexcept {
            auto len = length(m_LookAt - m_Eye);
            m_Eye += len * glm::normalize(direction);
        }
        //Get And Set
        CVLIB_DECLARE_GET_AND_SET_BY_REFERENCE(Camera, glm::vec3, Eye, m_Eye);
        CVLIB_DECLARE_GET_AND_SET_BY_REFERENCE(Camera, glm::vec3, LookAt, m_LookAt);
        CVLIB_DECLARE_GET_AND_SET_BY_REFERENCE(Camera, glm::vec3, Vup, m_Vup);
        CVLIB_DECLARE_GET_AND_SET_BY_VALUE(Camera, float,   FovY, m_FovY);
        CVLIB_DECLARE_GET_AND_SET_BY_VALUE(Camera, float, Aspect, m_Aspect);
        CVLIB_DECLARE_GET_AND_SET_BY_VALUE(Camera, float,  Focal, m_Focal);
        auto getViewMatrix()const noexcept->glm::mat4 {
            return glm::lookAt(m_Eye, m_LookAt, m_Vup);
        }
        auto getProjMatrix()const noexcept->glm::mat4 {
            return glm::perspective(m_FovY, m_Aspect, 0.0f, m_Focal);
        }
        //getUVW
        void getUVW(glm::vec3& u, glm::vec3& v, glm::vec3& w)const noexcept {
            w = m_LookAt - m_Eye;
            //front
            u = glm::normalize(glm::cross(m_Vup, w));
            v = glm::normalize(glm::cross(    w, u));
            auto vlen = glm::length(w) * std::tanf(glm::pi<float>() * m_FovY / 360.0f);
            auto ulen = vlen * m_Aspect;
            u *= ulen;
            v *= vlen;
        }
        std::tuple<glm::vec3, glm::vec3, glm::vec3> getUVW()const noexcept {
            std::tuple<glm::vec3, glm::vec3, glm::vec3> uvw;
            this->getUVW(std::get<0>(uvw), std::get<1>(uvw), std::get<2>(uvw));
            return uvw;
        }
    };
    enum class CameraMovement :uint8_t {
        eForward  = 0,
        eBackward = 1,
        eLeft     = 2,
        eRight    = 3,
        eUp       = 4,
        eDown     = 5,
    };
    struct CameraController {
    private:
        inline static constexpr float defaultYaw   = 90.0f;
        inline static constexpr float defaultPitch = 0.0f;
        inline static constexpr float defaultSpeed = 1.0f;
        inline static constexpr float defaultSensitivity = 0.0125f;
        inline static constexpr float defaultZoom = 45.0f;
    private:
        glm::vec3 m_Position;
        glm::vec3 m_Front;
        glm::vec3 m_Up;
        glm::vec3 m_Right;
        float     m_Yaw;
        float     m_Pitch;
        float     m_MovementSpeed;
        float     m_MouseSensitivity;
        float     m_Zoom;
    public:
        CameraController(
            const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f),
            const glm::vec3& up       = glm::vec3(0.0f, 1.0f, 0.0f),
            float yaw = defaultYaw,
            float pitch = defaultPitch)noexcept :
            m_Position{ position },
            m_Up{ up },
            m_Yaw{ yaw },
            m_Pitch{ pitch },
            m_MouseSensitivity{ defaultSensitivity },
            m_MovementSpeed{ defaultSpeed },
            m_Zoom{ defaultZoom }{
            UpdateCameraVectors();
        }
        Camera GetCamera(float fovY, float aspect, float focal = 1.0f)const noexcept {
            return Camera(m_Position, m_Position + m_Front, m_Up, fovY, aspect,focal);
        }
        void ProcessKeyboard(CameraMovement mode, float deltaTime)noexcept {
            float velocity = m_MovementSpeed * deltaTime;
            if (mode == CameraMovement::eForward) {
                m_Position -= m_Front * velocity;
            }
            if (mode == CameraMovement::eBackward) {
                m_Position += m_Front * velocity;
            }
            if (mode == CameraMovement::eLeft) {
                m_Position -= m_Right * velocity;
            }
            if (mode == CameraMovement::eRight) {
                m_Position += m_Right * velocity;
            }
            if (mode == CameraMovement::eUp) {
                m_Position -= m_Up * velocity;
            }
            if (mode == CameraMovement::eDown) {
                m_Position += m_Up * velocity;
            }
        }
        void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)noexcept {
            xoffset *= m_MouseSensitivity;
            yoffset *= m_MouseSensitivity;
            m_Yaw -= xoffset;
            m_Pitch += yoffset;
            if (constrainPitch) {
                if (m_Pitch > 89.0f) {
                    m_Pitch = 89.0f;
                }
                if (m_Pitch < -89.0f) {
                    m_Pitch = -89.0f;
                }
            }
            UpdateCameraVectors();
        }
        void ProcessMouseScroll(float yoffset)noexcept
        {
            float next_zoom = m_Zoom - yoffset;
            if (next_zoom >= 1.0f && next_zoom <= 45.0f)
                m_Zoom = next_zoom;
            if (next_zoom <= 1.0f)
                m_Zoom = 1.0f;
            if (next_zoom >= 45.0f)
                m_Zoom = 45.0f;
        }
        void SetMouseSensitivity(float sensitivity)noexcept {
            m_MouseSensitivity = sensitivity;
        }
        void SetMovementSpeed(float speed)noexcept {
            m_MovementSpeed = speed;
        }
    private:
        // Calculates the front vector from the Camera's (updated) Euler Angles
        void UpdateCameraVectors()noexcept
        {
            // Calculate the new Front vector
            glm::vec3 front;
            float yaw   = glm::pi<float>() *(m_Yaw) / 180.0f;
            float pitch = glm::pi<float>() * (m_Pitch) / 180.0f;
            front.x     = cos(yaw) * cos(pitch);
            front.y     = sin(pitch);
            front.z     = sin(yaw) * cos(pitch);
            m_Front     = glm::normalize(front);
            m_Right     = glm::normalize(glm::cross(m_Up, m_Front));

        }
    };
}
#endif