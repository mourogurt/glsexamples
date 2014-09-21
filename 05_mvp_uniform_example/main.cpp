#include <iostream>
#include <fstream>
#include <engInit.hpp>
#include <engBuffer.hpp>
#include <engValue.hpp>
#include <engShader.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

int main()
{
    EngInit init;
    unsigned win = init.createGLWindow("Example 5",800,600);
    EngGLPlatform* platform = init.getEngGLPlatform(win);
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
    EngGLUniform uniform(&shader,"mvp");
    glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    glm::mat4 View = glm::lookAt(glm::vec3(4,3,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 Model = glm::mat4(1.0f);
    glm::mat4 mvp = Projection * View * Model;
    uniform.write(&mvp[0][0],4,4,1,GL_FALSE);
    //EngGLAttribute attrib1(&shader,"vcolor");
    GLfloat color[2] = {0.5f,0.0f};
    //attrib1.write(color,4);
    EngGLUniform unif1(&shader,"vcolor");
    unif1.write(color,2,1);
    EngGLVBO buf1(&shader,"pos");
    GLfloat pos[] = {
            -1.0f, -1.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f,
             0.0f,  1.0f, 0.0f, 1.0f,
    };
    buf1.allocate(sizeof(pos),GL_STATIC_DRAW,pos);
    glClearColor(0.3f,0.3f,0.3f,1.0f);
    buf1.bind();
    buf1.enable();
    do
    {
        glClear(GL_COLOR_BUFFER_BIT);
        buf1.render(3);
        glfwSwapBuffers(platform->controll_window);
        glfwPollEvents();
    } while (glfwGetKey(platform->controll_window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
             glfwWindowShouldClose(platform->controll_window) == 0);
    buf1.disable();
    buf1.clear();
    glDeleteVertexArrays(1,&VAO);
    return 0;
}

