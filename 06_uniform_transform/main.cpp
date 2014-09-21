#include <engInit.hpp>
#include <engShader.hpp>
#include <engValue.hpp>
#include <engBuffer.hpp>
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

int main()
{
    EngInit init;
    unsigned win1 = init.createGLWindow("Example 6",800,600);
    EngGLPlatform* platform = init.getEngGLPlatform(win1);
    EngGLShader shader;
    string src;
    ifstream in("vert.glsl");
    getline(in,src,'\0');
    in.close();
    shader.compileShaderStage(GL_VERTEX_SHADER,src);
    in.open("frag.glsl");
    getline(in,src,'\0');
    in.close();
    shader.compileShaderStage(GL_FRAGMENT_SHADER,src);
    shader.linkShader();
    auto log = shader.getErrLog();
    for (unsigned i = 0; i < log.size(); i++)
        cout << log[i]<<endl;
    if (log.size() > 0) return 1;
    shader.bind_program();
    GLuint VAO;
    glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);
    GLfloat vertex_positions[] =
    {
        -1.0f,-1.0f,-1.0f,
            -1.0f,-1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f,-1.0f,
            -1.0f,-1.0f,-1.0f,
            -1.0f, 1.0f,-1.0f,
            1.0f,-1.0f, 1.0f,
            -1.0f,-1.0f,-1.0f,
            1.0f,-1.0f,-1.0f,
            1.0f, 1.0f,-1.0f,
            1.0f,-1.0f,-1.0f,
            -1.0f,-1.0f,-1.0f,
            -1.0f,-1.0f,-1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f,-1.0f,
            1.0f,-1.0f, 1.0f,
            -1.0f,-1.0f, 1.0f,
            -1.0f,-1.0f,-1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f,-1.0f, 1.0f,
            1.0f,-1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f,-1.0f,-1.0f,
            1.0f, 1.0f,-1.0f,
            1.0f,-1.0f,-1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f,-1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f,-1.0f,
            -1.0f, 1.0f,-1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f,-1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f,-1.0f, 1.0f
    };
    GLfloat g_color_buffer_data[] = {
        0.583f,  0.771f,  0.014f,
        0.609f,  0.115f,  0.436f,
        0.327f,  0.483f,  0.844f,
        0.822f,  0.569f,  0.201f,
        0.435f,  0.602f,  0.223f,
        0.310f,  0.747f,  0.185f,
        0.597f,  0.770f,  0.761f,
        0.559f,  0.436f,  0.730f,
        0.359f,  0.583f,  0.152f,
        0.483f,  0.596f,  0.789f,
        0.559f,  0.861f,  0.639f,
        0.195f,  0.548f,  0.859f,
        0.014f,  0.184f,  0.576f,
        0.771f,  0.328f,  0.970f,
        0.406f,  0.615f,  0.116f,
        0.676f,  0.977f,  0.133f,
        0.971f,  0.572f,  0.833f,
        0.140f,  0.616f,  0.489f,
        0.997f,  0.513f,  0.064f,
        0.945f,  0.719f,  0.592f,
        0.543f,  0.021f,  0.978f,
        0.279f,  0.317f,  0.505f,
        0.167f,  0.620f,  0.077f,
        0.347f,  0.857f,  0.137f,
        0.055f,  0.953f,  0.042f,
        0.714f,  0.505f,  0.345f,
        0.783f,  0.290f,  0.734f,
        0.722f,  0.645f,  0.174f,
        0.302f,  0.455f,  0.848f,
        0.225f,  0.587f,  0.040f,
        0.517f,  0.713f,  0.338f,
        0.053f,  0.959f,  0.120f,
        0.393f,  0.621f,  0.362f,
        0.673f,  0.211f,  0.457f,
        0.820f,  0.883f,  0.371f,
        0.982f,  0.099f,  0.879f
    };
    EngGLVBO vbo(&shader,"pos"), vbo2(&shader,"color");
    vbo.allocate(sizeof(vertex_positions),GL_STATIC_DRAW,(GLvoid*)vertex_positions);
    vbo.bind(3);
    vbo.enable();
    vbo2.allocate(sizeof(g_color_buffer_data),GL_STATIC_DRAW,(GLvoid*)g_color_buffer_data);
    vbo2.bind(3);
    vbo2.enable();
    EngGLUniform unif1(&shader,"proj"), unif2(&shader,"view");
    glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    glm::vec3 cam_position = glm::vec3(2,2,2);
    glm::mat4 View = glm::lookAt(cam_position, glm::vec3(0,0,0), glm::vec3(0,1,0));
    unif1.write(&Projection[0][0],4,4,1);
    unif2.write(&View[0][0],4,4,1);
    glClearColor(0.3f,0.3f,0.3f,1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    do
    {
        if (glfwGetKey(platform->controll_window, GLFW_KEY_D ) == GLFW_PRESS)
        {
            cam_position.x += 0.1f;
            View = glm::lookAt(cam_position, glm::vec3(0,0,0), glm::vec3(0,1,0));
            unif2.write(&View[0][0],4,4,1);
        }
        if (glfwGetKey(platform->controll_window, GLFW_KEY_A ) == GLFW_PRESS)
        {
            cam_position.x -= 0.1f;
            View = glm::lookAt(cam_position, glm::vec3(0,0,0), glm::vec3(0,1,0));
            unif2.write(&View[0][0],4,4,1);
        }
        if (glfwGetKey(platform->controll_window, GLFW_KEY_W ) == GLFW_PRESS)
        {
            cam_position.y += 0.1f;
            View = glm::lookAt(cam_position, glm::vec3(0,0,0), glm::vec3(0,1,0));
            unif2.write(&View[0][0],4,4,1);
        }
        if (glfwGetKey(platform->controll_window, GLFW_KEY_S ) == GLFW_PRESS)
        {
            cam_position.y -= 0.1f;
            View = glm::lookAt(cam_position, glm::vec3(0,0,0), glm::vec3(0,1,0));
            unif2.write(&View[0][0],4,4,1);
        }
        if (glfwGetKey(platform->controll_window, GLFW_KEY_Q ) == GLFW_PRESS)
        {
            cam_position.z += 0.1f;
            View = glm::lookAt(cam_position, glm::vec3(0,0,0), glm::vec3(0,1,0));
            unif2.write(&View[0][0],4,4,1);
        }
        if (glfwGetKey(platform->controll_window, GLFW_KEY_E ) == GLFW_PRESS)
        {
            cam_position.z -= 0.1f;
            View = glm::lookAt(cam_position, glm::vec3(0,0,0), glm::vec3(0,1,0));
            unif2.write(&View[0][0],4,4,1);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        vbo.render(36);
        glfwSwapBuffers(platform->controll_window);
        glfwPollEvents();
    } while (glfwGetKey(platform->controll_window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
             glfwWindowShouldClose(platform->controll_window) == 0);
    vbo.disable();
    vbo.clear();
    vbo2.disable();
    vbo2.clear();
    glDeleteVertexArrays(1,&VAO);
    return 0;
}

