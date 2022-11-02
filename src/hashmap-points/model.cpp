/*
 * model.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "model.hpp"
using namespace atto;

/** ---------------------------------------------------------------------------
 * Model::Model
 * @brief Create OpenCL context and associated objects.
 */
Model::Model()
{
    /*
     * Setup Model data.
     */
    {
        /* Generate n_points randomly distributed inside the domain */
        math::rng::Kiss kiss(true);             /* rng engine */
        math::rng::uniform<cl_float> rand;      /* rng sampler */

        m_points.clear();
        for (size_t i = 0; i < Params::n_points; ++i) {
            m_points.push_back({
                /* point coordinates */
                rand(kiss, Params::domain_lo.s[0], Params::domain_hi.s[0]),
                rand(kiss, Params::domain_lo.s[1], Params::domain_hi.s[1]),
                rand(kiss, Params::domain_lo.s[2], Params::domain_hi.s[2]),
                /* point color */
                0.0f,
                0.0f,
                0.0f});
        }

        /* Initialize prope */
        m_probe = {};
    }

    /*
     * Setup OpenGL data.
     */
    {
        /* Setup camera view matrix. */
        m_gl.camera.lookat(
            math::vec3f{0.0f, 0.0f, 2.0f},
            math::vec3f{0.0f, 0.0f, 0.0f},
            math::vec3f{0.0f, 1.0f, 0.0f});

        /*
         * Create buffer storage for vertex data with layout:
         * {(xyzrgb)_1, (xyzrgb)_2, ...}
         */
        m_gl.point_vbo = gl::create_buffer(
            GL_ARRAY_BUFFER,
            6 * Params::n_points * sizeof(GLfloat),
            GL_STREAM_DRAW);

        /*
         * Create buffer storage for sprite vertex data with layout:
         * {(uv)_1, (uv)_2, ...}
         */
        m_gl.sprite_vertex = std::vector<GLfloat>{
            0.0f,  0.0f,                        /* bottom left */
            1.0f,  0.0f,                        /* bottom right */
            0.0f,  1.0f,                        /* top left */
            1.0f,  1.0f};                       /* top right */

        m_gl.sprite_index = std::vector<GLuint>{
            0, 1, 2,                            /* first triangle */
            3, 2, 1};                           /* second triangle */

        m_gl.sprite_vbo = gl::create_buffer(
            GL_ARRAY_BUFFER,
            m_gl.sprite_vertex.size() * sizeof(GLfloat),
            GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, m_gl.sprite_vbo);
        glBufferSubData(
            GL_ARRAY_BUFFER,                            /* target binding point */
            0,                                          /* offset in data store */
            m_gl.sprite_vertex.size() * sizeof(GLfloat),/* data store size in bytes */
            m_gl.sprite_vertex.data());                 /* pointer to data source */
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_gl.sprite_ebo = gl::create_buffer(
            GL_ELEMENT_ARRAY_BUFFER,
            m_gl.sprite_index.size() * sizeof(GLuint),
            GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl.sprite_ebo);
        glBufferSubData(
            GL_ELEMENT_ARRAY_BUFFER,                    /* target binding point */
            0,                                          /* offset in data store */
            m_gl.sprite_index.size() * sizeof(GLuint),  /* data store size in bytes */
            m_gl.sprite_index.data());                  /* pointer to data source */
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        /*
         * Create shader program object.
         */
        std::vector<GLuint> shaders{
            gl::create_shader(GL_VERTEX_SHADER, "data/hashmap-points.vert"),
            gl::create_shader(GL_FRAGMENT_SHADER, "data/hashmap-points.frag")};
        m_gl.program = gl::create_program(shaders);
        std::cout << gl::get_program_info(m_gl.program) << "\n";

        /*
         * Create vertex array object.
         */
        m_gl.vao = gl::create_vertex_array();
        glBindVertexArray(m_gl.vao);

        /* Set point sprite vertex data format. */
        glBindBuffer(GL_ARRAY_BUFFER, m_gl.sprite_vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl.sprite_ebo);
        gl::enable_attribute(m_gl.program, "a_sprite_coord");
        gl::attribute_pointer(
            m_gl.program,
            "a_sprite_coord",
            GL_FLOAT_VEC2,
            2*sizeof(GLfloat),  /* byte offset between consecutive attributes */
            0,                  /* byte offset of first element in the buffer */
            false);             /* normalized flag */

        /* Set point vertex data format. */
        glBindBuffer(GL_ARRAY_BUFFER, m_gl.point_vbo);
        gl::enable_attribute(m_gl.program, "a_point_pos");
        gl::attribute_pointer(
            m_gl.program,
            "a_point_pos",
            GL_FLOAT_VEC3,
            6*sizeof(GLfloat),  /* byte offset between consecutive attributes */
            0,                  /* byte offset of first element in the buffer */
            false);             /* normalized flag */
        gl::attribute_divisor(m_gl.program, "a_point_pos", 1);

        gl::enable_attribute(m_gl.program, "a_point_col");
        gl::attribute_pointer(
            m_gl.program,
            "a_point_col",
            GL_FLOAT_VEC3,
            6*sizeof(GLfloat),  /* byte offset between consecutive attributes */
            3*sizeof(GLfloat),  /* byte offset of first element in the buffer */
            false);             /* normalized flag */
        gl::attribute_divisor(m_gl.program, "a_point_col", 1);

        /* Unbind vertex array object. */
        glBindVertexArray(0);
    }

    /*
     * Setup OpenCL data.
     */
    {
        /*
         * Setup OpenCL context with a command queue on the specified device.
         */
        // m_context = cl::Context::create(CL_DEVICE_TYPE_GPU);
        // m_device = cl::Context::get_device(m_context, Params::device_index);
        // m_queue = cl::Queue::create(m_context, m_device);

        /*
         * Setup OpenCL context based on the OpenGL context in the device.
         */
        std::vector<cl_device_id> devices = cl::Device::get_device_ids(CL_DEVICE_TYPE_GPU);
        core_assert(Params::device_index < devices.size(), "device index overflow");
        m_device = devices[Params::device_index];
        m_context = cl::Context::create_cl_gl_shared(m_device);
        m_queue = cl::Queue::create(m_context, m_device);
        std::cout << cl::Device::get_info_string(m_device) << "\n";

        /*
         * Create the program object.
         */
        m_program = cl::Program::create_from_file(m_context, "data/hashmap-points.cl");
        cl::Program::build(m_program, m_device, "");
        std::cout << cl::Program::get_source(m_program) << "\n";

        /*
         * Create the program kernels.
         */
        m_kernels.resize(NumKernels, NULL);
        m_kernels[KernelHashmapClear] = cl::Kernel::create(m_program, "hashmap_clear");
        m_kernels[KernelHashmapBuild] = cl::Kernel::create(m_program, "hashmap_build");
        m_kernels[KernelHashmapQuery] = cl::Kernel::create(m_program, "hashmap_query");
        m_kernels[KerkelUpdatePoints] = cl::Kernel::create(m_program, "update_points");
        m_kernels[KerkelUpdateVertex] = cl::Kernel::create(m_program, "update_vertex");

        /*
         * Create memory buffers.
         */
        m_buffers.resize(NumBuffers, NULL);
        m_buffers[BufferHashmap] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            Params::capacity * sizeof(KeyValue),
            (void *) NULL);
        m_buffers[BufferPoints] = cl::Memory::create_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            Params::n_points * sizeof(Point),
            (void *) NULL);
        m_buffers[BufferVertex] = cl::gl::create_from_gl_buffer(
            m_context,
            CL_MEM_READ_WRITE,
            m_gl.point_vbo);

        /*
         * Copy point data to the device.
        */
        cl::Queue::enqueue_write_buffer(
            m_queue,
            m_buffers[BufferPoints],
            CL_TRUE,
            0,
            Params::n_points * sizeof(Point),
            (void *) &m_points[0]);
    }
}

/** ---------------------------------------------------------------------------
 * Model::~Model
 * @brief Destroy the OpenCL context and associated objects.
 */
Model::~Model()
{
    /* Teardown OpenCL data. */
    {
        for (auto &it : m_images) {
            cl::Memory::release(it);
        }
        for (auto &it : m_buffers) {
            cl::Memory::release(it);
        }
        for (auto &it : m_kernels) {
            cl::Kernel::release(it);
        }
        cl::Program::release(m_program);
        cl::Queue::release(m_queue);
        cl::Device::release(m_device);
        cl::Context::release(m_context);
    }
}

/** ---------------------------------------------------------------------------
 * Model::handle
 * @brief Handle the event.
 */
void Model::handle(const gl::Event &event)
{
    const float move_scale = 0.02f;
    const float rotate_scale = 0.02f;
    const float size_scale = 1.01f;

    /*
     * Handle camera movement and rotation.
     */
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_W) {
        m_gl.camera.move(-m_gl.camera.eye() * move_scale);
    }
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_S) {
        m_gl.camera.move( m_gl.camera.eye() * move_scale);
    }
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_UP) {
        m_gl.camera.rotate_pitch( rotate_scale * M_PI);
    }
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_DOWN) {
        m_gl.camera.rotate_pitch(-rotate_scale * M_PI);
    }
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_LEFT) {
        m_gl.camera.rotate_yaw( rotate_scale * M_PI);
    }
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_RIGHT) {
        m_gl.camera.rotate_yaw(-rotate_scale * M_PI);
    }

    /*
     * Handle point size changes.
     */
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_MINUS) {
        m_gl.point_scale /= size_scale;
    }
    if (event.type == gl::Event::Key && event.key.code == GLFW_KEY_EQUAL) {
        m_gl.point_scale *= size_scale;
    }
}

/** ---------------------------------------------------------------------------
 * Model::draw
 * @brief Render the drawable.
 */
void Model::draw(void *data)
{
    GLFWwindow *window = gl::Renderer::window();
    if (window == nullptr) {
        return;
    }

    /*
     * Specify draw state modes.
     */
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Bind the shader program object and vertex array object. */
    glUseProgram(m_gl.program);
    glBindVertexArray(m_gl.vao);

    /* Set uniforms and draw. */
#if 0
    std::array<GLfloat,2> sizef = gl::Renderer::framebuffer_sizef();
    gl::set_uniform(m_gl.program, "u_width", GL_FLOAT, &sizef[0]);
    gl::set_uniform(m_gl.program, "u_height",GL_FLOAT, &sizef[1]);
#endif

    gl::set_uniform(m_gl.program, "u_scale", GL_FLOAT, &m_gl.point_scale);
    gl::set_uniform_matrix(m_gl.program, "u_view", GL_FLOAT_MAT4, true,
        m_gl.camera.view().data());
    gl::set_uniform_matrix(m_gl.program, "u_persp", GL_FLOAT_MAT4, true,
        m_gl.camera.persp().data());

    glDrawElementsInstanced(
        GL_TRIANGLES,               /* what kind of primitives? */
        m_gl.sprite_index.size(),   /* number of elements to render */
        GL_UNSIGNED_INT,            /* type of the values in indices */
        (GLvoid *) 0,               /* pointer to indices storage location */
        Params::n_points);          /* number of instances to be rendered */

    /* Unbind the vertex array object and shader program object. */
    glBindVertexArray(0);
    glUseProgram(0);
}

/** ---------------------------------------------------------------------------
 * Model::execute
 * @brief Execute the model.
 */
void Model::execute(void)
{
    /* Update probe */
    {
        const math::vec3f domain_lo{
            Params::domain_lo.s[0],
            Params::domain_lo.s[1],
            Params::domain_lo.s[2]};

        const math::vec3f domain_hi{
            Params::domain_hi.s[0],
            Params::domain_hi.s[1],
            Params::domain_hi.s[2]};

        const cl_float dt = 0.02f;
        const cl_float theta = dt * glfwGetTime();
        const cl_float radius = std::cos(theta) * math::norm(domain_hi - domain_lo);

        cl_float x = m_probe.pos.s[0];
        cl_float y = m_probe.pos.s[1];

        /* Update probe position and apply periodic boundary conditions. */
        m_probe.pos.s[0] -= dt * radius * std::sin(theta);
        m_probe.pos.s[1] += dt * radius * std::sin(theta);
        m_probe.pos.s[2] += dt * radius * std::cos(theta);

        for (size_t i = 0; i < 3; ++i) {
            cl_float len = Params::domain_hi.s[i] - Params::domain_lo.s[i];
            if (m_probe.pos.s[i] < Params::domain_lo.s[i]) {
                m_probe.pos.s[i] += len;
            } else if (m_probe.pos.s[i] > Params::domain_hi.s[i]) {
                m_probe.pos.s[i] -= len;
            }
        }
    }

    /*
     * Clear the hashmap
     */
    {
        /* Set kernel arguments. */
        cl::Kernel::set_arg(m_kernels[KernelHashmapClear], 0, sizeof(cl_mem),  (void *) &m_buffers[BufferHashmap]);
        cl::Kernel::set_arg(m_kernels[KernelHashmapClear], 1, sizeof(cl_uint), (void *) &Params::capacity);

        /* Run the kernel */
        static cl::NDRange global_ws(cl::NDRange::Roundup(Params::n_points, Params::work_group_size));
        static cl::NDRange local_ws(Params::work_group_size);

        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelHashmapClear],
            cl::NDRange::Null,
            global_ws,
            local_ws);
    }

    /*
     * Build the hashmap
     */
    {
        /* Set kernel arguments. */
        cl::Kernel::set_arg(m_kernels[KernelHashmapBuild], 0, sizeof(cl_mem),    (void *) &m_buffers[BufferHashmap]);
        cl::Kernel::set_arg(m_kernels[KernelHashmapBuild], 1, sizeof(cl_uint),   (void *) &Params::capacity);
        cl::Kernel::set_arg(m_kernels[KernelHashmapBuild], 2, sizeof(cl_mem),    (void *) &m_buffers[BufferPoints]);
        cl::Kernel::set_arg(m_kernels[KernelHashmapBuild], 3, sizeof(cl_uint),   (void *) &Params::n_points);
        cl::Kernel::set_arg(m_kernels[KernelHashmapBuild], 4, sizeof(cl_uint),   (void *) &Params::n_cells);
        cl::Kernel::set_arg(m_kernels[KernelHashmapBuild], 5, sizeof(cl_float3), (void *) &Params::domain_lo);
        cl::Kernel::set_arg(m_kernels[KernelHashmapBuild], 6, sizeof(cl_float3), (void *) &Params::domain_hi);

        /* Run the kernel */
        static cl::NDRange global_ws(cl::NDRange::Roundup(Params::n_points, Params::work_group_size));
        static cl::NDRange local_ws(Params::work_group_size);

        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelHashmapBuild],
            cl::NDRange::Null,
            global_ws,
            local_ws);
    }

    /*
     * Query the hashmap
     */
    {
        /* Set kernel arguments. */
        cl::Kernel::set_arg(m_kernels[KernelHashmapQuery], 0, sizeof(cl_mem),    (void *) &m_buffers[BufferPoints]);
        cl::Kernel::set_arg(m_kernels[KernelHashmapQuery], 1, sizeof(cl_uint),   (void *) &Params::n_points);
        cl::Kernel::set_arg(m_kernels[KernelHashmapQuery], 2, sizeof(cl_uint),   (void *) &Params::n_cells);
        cl::Kernel::set_arg(m_kernels[KernelHashmapQuery], 3, sizeof(cl_float3), (void *) &Params::domain_lo);
        cl::Kernel::set_arg(m_kernels[KernelHashmapQuery], 4, sizeof(cl_float3), (void *) &Params::domain_hi);
        cl::Kernel::set_arg(m_kernels[KernelHashmapQuery], 5, sizeof(Point),     (void *) &m_probe);

        /* Run the kernel */
        static cl::NDRange global_ws(cl::NDRange::Roundup(Params::n_points, Params::work_group_size));
        static cl::NDRange local_ws(Params::work_group_size);

        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KernelHashmapQuery],
            cl::NDRange::Null,
            global_ws,
            local_ws);
    }

    /*
     * Update points
     */
    {
        /* Set kernel arguments. */
        cl::Kernel::set_arg(m_kernels[KerkelUpdatePoints], 0, sizeof(cl_mem),    (void *) &m_buffers[BufferPoints]);
        cl::Kernel::set_arg(m_kernels[KerkelUpdatePoints], 1, sizeof(cl_uint),   (void *) &Params::n_points);
        cl::Kernel::set_arg(m_kernels[KerkelUpdatePoints], 2, sizeof(cl_float3), (void *) &Params::domain_lo);
        cl::Kernel::set_arg(m_kernels[KerkelUpdatePoints], 3, sizeof(cl_float3), (void *) &Params::domain_hi);

        /* Run the kernel */
        static cl::NDRange global_ws(cl::NDRange::Roundup(Params::n_points, Params::work_group_size));
        static cl::NDRange local_ws(Params::work_group_size);

        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KerkelUpdatePoints],
            cl::NDRange::Null,
            global_ws,
            local_ws);
    }

    /*
     * Update vertex data from the points positions
     */
    {
        /* Wait for OpenGL to finish and acquire the gl objects. */
        cl::gl::enqueue_acquire_gl_objects(m_queue, 1, &m_buffers[BufferVertex]);

        /* Enqueue the OpenCL kernel for execution. */
        cl::Kernel::set_arg(m_kernels[KerkelUpdateVertex], 0, sizeof(cl_mem),  (void *) &m_buffers[BufferVertex]);
        cl::Kernel::set_arg(m_kernels[KerkelUpdateVertex], 1, sizeof(cl_mem),  (void *) &m_buffers[BufferPoints]);
        cl::Kernel::set_arg(m_kernels[KerkelUpdateVertex], 2, sizeof(cl_uint), (void *) &Params::n_points);

        static cl::NDRange global_ws(cl::NDRange::Roundup(Params::n_points, Params::work_group_size));
        static cl::NDRange local_ws(Params::work_group_size);

        cl::Queue::enqueue_nd_range_kernel(
            m_queue,
            m_kernels[KerkelUpdateVertex],
            cl::NDRange::Null,
            global_ws,
            local_ws);

        /* Wait for OpenCL to finish and release the gl objects. */
        cl::gl::enqueue_release_gl_objects(m_queue, 1, &m_buffers[BufferVertex]);
    }
}
