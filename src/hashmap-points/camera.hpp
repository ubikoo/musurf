/*
 * camera.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef CAMERA_H_
#define CAMERA_H_

/**
 * Camera
 * Basic camera implementation.
 */
struct Camera {
    /* Camera view parameters. */
    float m_yaw;
    float m_pitch;
    atto::math::vec3f m_eye;
    atto::math::vec3f m_up;
    atto::math::vec3f m_front;
    atto::math::vec3f m_right;

    /* Camera projection parameters. */
    float m_fovy;
    float m_aspect;
    float m_znear;
    float m_zfar;

    /* Camera view and projection matrices. */
    atto::math::mat4f m_view;
    atto::math::mat4f m_persp;

    /* Camera accessors. */
    const atto::math::vec3f &eye(void) const { return m_eye; }
    const atto::math::vec3f &up(void) const { return m_up; }
    const atto::math::vec3f &front(void) const { return m_front; }
    const atto::math::vec3f &right(void) const { return m_right; }

    const float &fovy(void) const { return m_fovy; }
    const float &aspect(void) const { return m_aspect; }
    const float &znear(void) const { return m_znear; }
    const float &zfar(void) const { return m_zfar; }

    const atto::math::mat4f &view(void) const { return m_view; }
    const atto::math::mat4f &persp(void) const { return m_persp; }

    /* Update the camera state. */
    void update(void);

    /* Reset the camera state. */
    void reset(void);

    /* Move the camera along the view direction w. */
    void move(const atto::math::vec3f &dir);

    /* Rotate the camera around the up direction. */
    void rotate_yaw(const float angle);

    /* Rotate the camera around the right direction u. */
    void rotate_pitch(const float angle);

    /* Zoom the camera field of view. */
    void zoom(const float fovy);

    /* Set the camera lookat state. */
    void lookat(
        const atto::math::vec3f &eye,
        const atto::math::vec3f &ctr,
        const atto::math::vec3f &up);

    /* Set the camera projection lens. */
    void perspective(
        const float fovy,
        const float aspect,
        const float znear,
        const float zfar);

    /* Constructor/destructor. */
    Camera() { reset(); }
    ~Camera() = default;

    /* Copy constructor/assignment */
    Camera(const Camera &other) {
        /* Camera view parameters. */
        m_yaw   = other.m_yaw;
        m_pitch = other.m_pitch;
        m_eye   = other.m_eye;
        m_up    = other.m_up;
        m_front = other.m_front;
        m_right = other.m_right;

        /* Camera lens parameters */
        m_fovy   = other.m_fovy;
        m_aspect = other.m_aspect;
        m_znear  = other.m_znear;
        m_zfar   = other.m_zfar;

        /* Camera view and projection matrices. */
        m_view = other.m_view;
        m_persp = other.m_persp;
    }
    Camera &operator=(const Camera &other) {
        if (this == &other) {
            return *this;
        }

        /* Camera view parameters. */
        m_yaw   = other.m_yaw;
        m_pitch = other.m_pitch;
        m_eye   = other.m_eye;
        m_up    = other.m_up;
        m_front = other.m_front;
        m_right = other.m_right;

        /* Camera lens parameters */
        m_fovy   = other.m_fovy;
        m_aspect = other.m_aspect;
        m_znear  = other.m_znear;
        m_zfar   = other.m_zfar;

        /* Camera view and projection matrices. */
        m_view = other.m_view;
        m_persp = other.m_persp;

        return *this;
    }
}; /* Camera */

/** ---------------------------------------------------------------------------
 * to_string
 * Serialize camera state.
 */
std::string to_string(void);

#endif /* CAMERA_H_ */
