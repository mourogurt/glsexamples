#include <backend/engInit.hpp>
#include <backend/engShader.hpp>
#include <backend/engValue.hpp>
#include <backend/engBuffer.hpp>
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#define VPX_CODEC_DISABLE_COMPAT 1
#include "vpx/vpx_encoder.h"
#include "vpx/vp8cx.h"
#include <libyuv.h>
#define interface (vpx_codec_vp8_cx())
#define fourcc    0x30385056

#define IVF_FILE_HDR_SZ  (32)
#define IVF_FRAME_HDR_SZ (12)

const int N = 1024;
const int M = 780;
const int FRAMES = 200;

static void mem_put_le16(char *mem, unsigned int val) {
    mem[0] = val;
    mem[1] = val>>8;
}

static void mem_put_le32(char *mem, unsigned int val) {
    mem[0] = val;
    mem[1] = val>>8;
    mem[2] = val>>16;
    mem[3] = val>>24;
}

using namespace std;

static void write_ivf_file_header(ofstream& outfile,
                                  const vpx_codec_enc_cfg_t *cfg,
                                  int frame_cnt)
{
    char header[32];

    if(cfg->g_pass != VPX_RC_ONE_PASS && cfg->g_pass != VPX_RC_LAST_PASS)
        return;
    header[0] = 'D';
    header[1] = 'K';
    header[2] = 'I';
    header[3] = 'F';
    mem_put_le16(header+4,  0);                   /* version */
    mem_put_le16(header+6,  32);                  /* headersize */
    mem_put_le32(header+8,  fourcc);              /* headersize */
    mem_put_le16(header+12, cfg->g_w);            /* width */
    mem_put_le16(header+14, cfg->g_h);            /* height */
    mem_put_le32(header+16, cfg->g_timebase.den); /* rate */
    mem_put_le32(header+20, cfg->g_timebase.num); /* scale */
    mem_put_le32(header+24, frame_cnt);           /* length */
    mem_put_le32(header+28, 0);                   /* unused */
    outfile.write(header,32);
}

static void write_ivf_frame_header(ofstream& outfile,
                                   const vpx_codec_cx_pkt_t *pkt)
{
    char             header[12];
    vpx_codec_pts_t  pts;

    if(pkt->kind != VPX_CODEC_CX_FRAME_PKT)
        return;

    pts = pkt->data.frame.pts;
    mem_put_le32(header, pkt->data.frame.sz);
    mem_put_le32(header+4, pts&0xFFFFFFFF);
    mem_put_le32(header+8, pts >> 32);
    outfile.write(header,12);
}

int main()
{
    vpx_image_t          image;
    vpx_codec_ctx_t      codec;
    vpx_codec_enc_cfg_t  cfg;
    int                  frame_cnt = 0;
    int                  flags = 0;
    vpx_img_alloc(&image, VPX_IMG_FMT_I420, N, M, 1);
    ofstream out("video.ivf");
    cout << "Using: " << vpx_codec_iface_name(interface) << endl;
    vpx_codec_enc_config_default(interface, &cfg, 0);
    cfg.rc_target_bitrate = N * M * cfg.rc_target_bitrate
                                / cfg.g_w / cfg.g_h;
    cfg.g_w = N;
    cfg.g_h = M;
    write_ivf_file_header(out, &cfg, FRAMES);
    vpx_codec_enc_init(&codec, interface, &cfg, 0);
    EngInit init;
    init.setHint({GLFW_VISIBLE,GL_FALSE});
    unsigned win1 = init.createGLWindow("IVF Video",N,M);
    EngGLPlatform* platform = init.getEngGLPlatform(win1);
    EngGLShader shader;
    //EngGLShader shader2;
    string src;
    ifstream in;
    in.open("vert.glsl");
    getline(in,src,'\0');
    in.close();
    shader.compileShaderStage(gl::GLenum(GL_VERTEX_SHADER),src);
    in.open("frag.glsl");
    getline(in,src,'\0');
    in.close();
    shader.compileShaderStage(gl::GLenum(GL_FRAGMENT_SHADER),src);
    shader.linkShader();
    auto log = shader.getErrLog();
    for (unsigned i = 0; i < log.size(); i++)
        cout << log[i]<<endl;
    if (log.size() > 0) return 1;
    /*in.open("passvert.glsl");
    getline(in,src,'\0');
    in.close();
    shader2.compileShaderStage(gl::GLenum(GL_VERTEX_SHADER),src);
    in.open("passfrag.glsl");
    getline(in,src,'\0');
    in.close();
    shader2.compileShaderStage(gl::GLenum(GL_FRAGMENT_SHADER),src);
    shader2.linkShader();
    log = shader2.getErrLog();
    for (unsigned i = 0; i < log.size(); i++)
        cout << log[i]<<endl;
    if (log.size() > 0) return 1;*/
    GLuint VAO;
    gl::glGenVertexArrays(1,&VAO);
    gl::glBindVertexArray(VAO);
    /*GLfloat rect_pos[] = {
            -1.0f, -1.0f, 0.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f,
             1.0f,  1.0f, 0.0f, 1.0f
    };
    GLfloat uv [] = {
             0.0f,0.0f,
             0.0f,1.0f,
             1.0f,0.0f,
             1.0f,1.0f
    };*/
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
    //EngGLVBO vbo3(&shader2,"pos"), vbo4(&shader2,"uv");
    vbo.allocate(sizeof(vertex_positions),gl::GLenum(GL_STATIC_DRAW),vertex_positions);
    vbo2.allocate(sizeof(g_color_buffer_data),gl::GLenum(GL_STATIC_DRAW),g_color_buffer_data);
    //vbo3.allocate(sizeof(rect_pos),gl::GLenum(GL_STATIC_DRAW),rect_pos);
    //vbo4.allocate(sizeof(uv),gl::GLenum(GL_STATIC_DRAW),uv);
    EngGLUniform unif1(&shader,"proj"), unif2(&shader,"view");
    glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    glm::vec3 cam_position = glm::vec3(15,2,2);
    glm::mat4 View = glm::lookAt(cam_position, glm::vec3(0,0,0), glm::vec3(0,1,0));
    unif1.write(&Projection[0][0],4,4,1);
    unif2.write(&View[0][0],4,4,1);
    auto start = std::chrono::system_clock::now();
    auto end = start;
    float time = 0;
    auto dur = end - start;
    gl::glClearColor(0.3f,0.3f,0.3f,1.0f);
    gl::glEnable(gl::GLenum(GL_DEPTH_TEST));
    gl::glDepthFunc(gl::GLenum(GL_LESS));
    GLuint FramebufferName = 0;
    gl::glGenFramebuffers(1, &FramebufferName);
    gl::glBindFramebuffer(gl::GLenum(GL_FRAMEBUFFER), FramebufferName);
    GLuint renderedTexture;
    //GLuint outTexture;
    gl::glGenTextures(1, &renderedTexture);
    //gl::glGenTextures(1, &outTexture);
    gl::glBindTexture(gl::GLenum(GL_TEXTURE_2D), renderedTexture);
    gl::glTexImage2D(gl::GLenum(GL_TEXTURE_2D), 0,GL_RGB, N, M, 0,gl::GLenum(GL_RGB), gl::GLenum(GL_UNSIGNED_BYTE), 0);
    gl::glTexParameteri(gl::GLenum(GL_TEXTURE_2D),gl::GLenum(GL_TEXTURE_MAG_FILTER), GL_NEAREST);
    gl::glTexParameteri(gl::GLenum(GL_TEXTURE_2D), gl::GLenum(GL_TEXTURE_MIN_FILTER), GL_NEAREST);
    gl::glTexParameteri(gl::GLenum(GL_TEXTURE_2D), gl::GLenum(GL_TEXTURE_WRAP_S), GL_CLAMP_TO_EDGE);
    gl::glTexParameteri(gl::GLenum(GL_TEXTURE_2D), gl::GLenum(GL_TEXTURE_WRAP_T), GL_CLAMP_TO_EDGE);
    GLuint depthrenderbuffer;
    gl::glGenRenderbuffers(1, &depthrenderbuffer);
    gl::glBindRenderbuffer(gl::GLenum(GL_RENDERBUFFER), depthrenderbuffer);
    gl::glRenderbufferStorage(gl::GLenum(GL_RENDERBUFFER),gl::GLenum(GL_DEPTH_COMPONENT), N, M);
    gl::glFramebufferRenderbuffer(gl::GLenum(GL_FRAMEBUFFER),gl::GLenum(GL_DEPTH_ATTACHMENT),gl::GLenum(GL_RENDERBUFFER), depthrenderbuffer);
    gl::glFramebufferTexture(gl::GLenum(GL_FRAMEBUFFER),gl::GLenum(GL_COLOR_ATTACHMENT0), renderedTexture, 0);
    gl::GLenum DrawBuffers[1] = {gl::GLenum(GL_COLOR_ATTACHMENT0)};
    gl::glDrawBuffers(1, DrawBuffers);
    if(gl::glCheckFramebufferStatus(gl::GLenum(GL_FRAMEBUFFER)) != gl::GLenum(GL_FRAMEBUFFER_COMPLETE))
        return false;
    for (unsigned i = 0; i < FRAMES; i++)
    {
        gl::glBindFramebuffer(gl::GLenum(GL_FRAMEBUFFER), FramebufferName);
        glViewport(0,0,N,M);
        shader.bind_program();
        glBindTexture(gl::GLenum(GL_TEXTURE_2D), renderedTexture);
        vbo.enable();
        vbo.bind(3);
        vbo2.enable();
        vbo2.bind(3);
        end = std::chrono::system_clock::now();
        dur = end- start;
        time = dur.count();
        start = end;
        cam_position.x -= time / 100000000.0;
        View = glm::lookAt(cam_position, glm::vec3(0,0,0), glm::vec3(0,1,0));
        unif2.write(&View[0][0],4,4,1);
        gl::glClear(gl::ClearBufferMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        vbo.render(36);
        unsigned char* pixels = new uint8_t[N*M*3];
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        vpx_codec_iter_t iter = NULL;
        const vpx_codec_cx_pkt_t *pkt;
        libyuv::RAWToI420(pixels,N*3,image.planes[VPX_PLANE_Y],image.stride[VPX_PLANE_Y],
                                image.planes[VPX_PLANE_U],image.stride[VPX_PLANE_U],
                                image.planes[VPX_PLANE_V],image.stride[VPX_PLANE_V],
                                N,M);
        if (vpx_codec_encode(&codec, &image, frame_cnt,1, flags, VPX_DL_REALTIME))
            cerr << "Error while encoding data";
        while( (pkt = vpx_codec_get_cx_data(&codec, &iter)) ) {
            switch(pkt->kind) {
            case VPX_CODEC_CX_FRAME_PKT:
                write_ivf_frame_header(out, pkt);
                out.write((const char*)pkt->data.frame.buf,pkt->data.frame.sz);
                break;
            default:
                break;
            }
        }
        frame_cnt++;
        cout << "Frame: " << frame_cnt << endl;
        vbo.disable();
        vbo2.disable();
        /*libyuv::I420ToRAW(image.planes[VPX_PLANE_Y],image.stride[VPX_PLANE_Y],
                                image.planes[VPX_PLANE_U],image.stride[VPX_PLANE_U],
                                image.planes[VPX_PLANE_V],image.stride[VPX_PLANE_V],
                                pixels,image.d_w * 3,image.d_w,image.d_h);
        glActiveTexture(GL_TEXTURE0);
        gl::glBindFramebuffer(gl::GLenum(GL_FRAMEBUFFER), 0);
        gl::glBindTexture(gl::GLenum(GL_TEXTURE_2D), outTexture);
        gl::glTexImage2D(gl::GLenum(GL_TEXTURE_2D), 0,GL_RGB, N, M, 0,gl::GLenum(GL_RGB), gl::GLenum(GL_UNSIGNED_BYTE), pixels);
        gl::glTexParameteri(gl::GLenum(GL_TEXTURE_2D),gl::GLenum(GL_TEXTURE_MAG_FILTER), GL_NEAREST);
        gl::glTexParameteri(gl::GLenum(GL_TEXTURE_2D), gl::GLenum(GL_TEXTURE_MIN_FILTER), GL_NEAREST);
        gl::glTexParameteri(gl::GLenum(GL_TEXTURE_2D), gl::GLenum(GL_TEXTURE_WRAP_S), GL_CLAMP_TO_EDGE);
        gl::glTexParameteri(gl::GLenum(GL_TEXTURE_2D), gl::GLenum(GL_TEXTURE_WRAP_T), GL_CLAMP_TO_EDGE);
        glViewport(0,0,N,M);
        shader2.bind_program();
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        vbo3.bind();
        vbo3.enable();
        vbo4.bind(2);
        vbo4.enable();
        vbo3.render(4,0,gl::GLenum(GL_TRIANGLE_STRIP));
        vbo3.disable();
        vbo4.disable();
        delete [] pixels;*/
        glfwSwapBuffers(platform->controll_window);
        glfwPollEvents();
    }
    vpx_img_free(&image);
    vpx_codec_destroy(&codec);
    out.close();
    gl::glDeleteFramebuffers(1, &FramebufferName);
    gl::glDeleteTextures(1, &renderedTexture);
    //gl::glDeleteTextures(1, &outTexture);
    gl::glDeleteRenderbuffers(1, &depthrenderbuffer);
    gl::glDeleteVertexArrays(1,&VAO);
    return 0;
}
