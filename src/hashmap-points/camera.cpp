/*
 * camera.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "atto/opencl/opencl.hpp"
#include "camera.hpp"
using namespace atto;

/**
 * Camera::update
 * Update the camera state.
 */
void Camera::update(void)
{
    /* Compute camera front and right direction vectors. */
    m_front = math::vec3f{
        std::cos(m_pitch) * std::cos(m_yaw),
        std::sin(m_pitch),
        std::cos(m_pitch) * std::sin(m_yaw)};
    m_front = math::normalize(m_front);
    m_right = math::normalize(math::cross(m_front, m_up));

    /* Compute camera transforms. */
    m_view = math::lookat(m_eye, m_eye + m_front, m_up);
    m_persp = math::perspective(m_fovy, m_aspect, m_znear, m_zfar);
}

/**
 * Camera::reset
 * Reset the camera state.
 */
void Camera::reset(void)
{
    /* Reset camera view state. */
    m_yaw = 0.5f * M_PI;
    m_pitch = 0.0f;
    m_eye = math::vec3f{0.0f, 0.0f, 0.0f};
    m_up =  math::vec3f{0.0f, 1.0f, 0.0f};

    /* Reset camera lens parameters. */
    m_fovy = 0.5f * M_PI;
    m_aspect = 1.0f;
    m_znear = 0.01f;
    m_zfar = 10.0f;

    /* Update camera transforms. */
    update();
}

/**
 * Camera::move
 * Move the camera along the view direction w.
 */
void Camera::move(const math::vec3f &dir)
{
    lookat(m_eye + dir, m_front, m_up);
}

/**
 * Camera::rotate_yaw
 * Rotate the camera around the up direction.
 */
void Camera::rotate_yaw(const float angle)
{
    m_yaw += angle;
    update();
}

/**
 * Camera::rotate_pitch
 * Rotate the camera around the right direction u.
 */
void Camera::rotate_pitch(const float angle)
{
    const float max_pitch = 0.999f*M_PI;
    const float min_pitch = -max_pitch;
    m_pitch += angle;
    m_pitch = std::min(std::max(m_pitch, min_pitch), max_pitch);
    update();
}

/**
 * Camera::zoom
 * Zoom the camera field of view.
 */
void Camera::zoom(const float fovy)
{
    const float max_fovy = 0.999f*M_PI;
    const float min_fovy = 0.001f*M_PI;
    m_fovy += fovy;
    m_fovy = std::min(std::max(m_fovy, min_fovy), max_fovy);
    update();
}

/**
 * Camera::lookat
 * Set the camera lookat state.
 */
void Camera::lookat(
    const math::vec3f &eye,
    const math::vec3f &ctr,
    const math::vec3f &up)
{
    m_eye = eye;
    m_up = up;
    math::vec3f f = math::normalize(ctr - eye);
    // m_yaw = std::atan2(f(2), f(0));
    // m_pitch = std::asin(f(1));
    update();
}

/**
 * Camera::perspective
 * Set the camera projection lens.
 */
void Camera::perspective(
    const float fovy,
    const float aspect,
    const float znear,
    const float zfar)
{
    m_fovy = fovy;
    m_aspect = aspect;
    m_znear = znear;
    m_zfar = zfar;
    update();
}

/** ---------------------------------------------------------------------------
 * to_string
 * Serialize camera state.
 */
std::string to_string(const Camera &camera)
{
    std::ostringstream ss;

    ss << "yaw "    << camera.m_yaw << "\n";
    ss << "pitch "  << camera.m_pitch << "\n";
    ss << "eye "    << math::to_string(camera.m_eye) << "\n";
    ss << "front "  << math::to_string(camera.m_front) << "\n";
    ss << "right "  << math::to_string(camera.m_right) << "\n";

    ss << "fovy "   << camera.m_fovy   << "\n";
    ss << "aspect " << camera.m_aspect << "\n";
    ss << "znear "  << camera.m_znear  << "\n";
    ss << "zfar "   << camera.m_zfar   << "\n";

    ss << "view\n"  << math::to_string(camera.m_view) << "\n";
    ss << "persp\n" << math::to_string(camera.m_persp) << "\n";

    return ss.str();
}
