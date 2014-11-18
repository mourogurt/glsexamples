#include "webm_header.h"

void webm_start(char const* name) {
  int r = 0;
  nestegg* ne;
  ifstream infile(name);
  nestegg_io ne_io;
  ne_io.read = ifstream_read; //Ставим callback'и по которым либа(nestegg) будет проходиться по файлу
  ne_io.seek = ifstream_seek;
  ne_io.tell = ifstream_tell;
  ne_io.userdata = (void*)&infile;

  r = nestegg_init(&ne, ne_io, NULL, -1); //Инициализируем
  assert(r == 0);

  uint64_t duration = 0;
  r = nestegg_duration(ne, &duration);
  assert(r == 0);
  cout << "Duration: " << duration << endl;

  unsigned int ntracks = 0;
  r = nestegg_track_count(ne, &ntracks); //Число дорожек в потоке
  assert(r == 0);
  cout << "Tracks: " << ntracks << endl;

  nestegg_video_params vparams;
  vparams.width = 0;
  vparams.height = 0;

  vpx_codec_iface_t* interface;
  for (unsigned i=0; i < ntracks; ++i) {
    int id = nestegg_track_codec_id(ne, i); //Получаем ID codec'а
    assert(id >= 0);
    int type = nestegg_track_type(ne, i); //тип дорожки (видео или звук)
    cout << "Track " << i << " codec id: " << id << " type " << type << " ";
    interface = id == NESTEGG_CODEC_VP9 ? &vpx_codec_vp9_dx_algo : &vpx_codec_vp8_dx_algo; //VP9 или VP8
    if (type == NESTEGG_TRACK_VIDEO) {
      r = nestegg_track_video_params(ne, i, &vparams); //Параметры видео
      assert(r == 0);
      cout << vparams.width << "x" << vparams.height
           << " (d: " << vparams.display_width << "x" << vparams.display_height << ")";
    }
    if (type == NESTEGG_TRACK_AUDIO) { //Частота, каналы, глубина аудио
      nestegg_audio_params params;
      r = nestegg_track_audio_params(ne, i, &params);
      assert(r == 0);
      cout << params.rate << " " << params.channels << " channels " << " depth " << params.depth;
    }
    cout << endl;
  }

  EngInit init;
  unsigned win = init.createGLWindow("WebM player",vparams.width,vparams.height); //Создаем окно
  EngGLPlatform* platform = init.getEngGLPlatform(win);  //Получаем параметры окна
  EngGLShader shader; //Компилим шейдры
  string src;
  ifstream in("vert.glsl");
  getline(in,src,'\0');
  in.close();
  shader.compileShaderStage(gl::GLenum(GL_VERTEX_SHADER),src);
  in.open("frag.glsl");
  getline(in,src,'\0');
  in.close();
  shader.compileShaderStage(gl::GLenum(GL_FRAGMENT_SHADER),src);
  shader.linkShader();
  auto log = shader.getErrLog(); //Получаем лог ошибок
  for (unsigned i = 0; i < log.size(); i++)
      cout << log[i]<<endl;
  if (log.size() > 0) return;
  shader.bind_program();
  GLuint VAO;
  gl::glGenVertexArrays(1,&VAO); //Организуем, VAO, две юниформы(проекция и вид), буффер с координатами
  gl::glBindVertexArray(VAO);    //Вершин и текстур
  EngGLUniform unif1(&shader,"proj"), unif2(&shader,"view");
  glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
  glm::vec3 cam_position = glm::vec3(2,2,2);
  glm::mat4 View = glm::lookAt(cam_position, glm::vec3(0,0,0), glm::vec3(0,1,0));
  unif1.write(&Projection[0][0],4,4,1);
  unif2.write(&View[0][0],4,4,1);
  EngGLVBO buf1(&shader,"pos"), buf2(&shader,"uv");
  GLfloat pos[] = {
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
  };
  buf1.allocate(sizeof(pos),gl::GLenum(GL_STATIC_DRAW),pos);
  buf2.allocate(sizeof(uv),gl::GLenum(GL_STATIC_DRAW),uv);
  gl::glClearColor(0.3f,0.3f,0.3f,1.0f);
  buf1.bind();
  buf1.enable();
  buf2.bind(2);
  buf2.enable();
  gl::glEnable(gl::GLenum(GL_DEPTH_TEST));
  gl::glDepthFunc(gl::GLenum(GL_LESS));
  gl::glClearColor(0.3,0.3,0.3,1.0);
  glfwSwapInterval(2);

  vpx_codec_ctx_t  codec;
  int flags = 0;

  cout << "Using " << vpx_codec_iface_name(interface) << endl;
  if(vpx_codec_dec_init(&codec, interface, NULL, flags)) {
    cerr << "Failed to initialize decoder" << endl;
    return;
  }
  int video_count = 0;
  int audio_count = 0;
  nestegg_packet* packet = 0;
  // 1 = keep calling
  // 0 = eof
  // -1 = error
  GLuint texture;
  while (glfwGetKey(platform->controll_window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
               glfwWindowShouldClose(platform->controll_window) == 0)
  {
    r = nestegg_read_packet(ne, &packet); //Читаем пакет
    if (r == 1 && packet == 0) //Если прочитался нормально, но пустой то continue
      continue;
    if (r <=0)
      break;

    unsigned int track = 0;
    r = nestegg_packet_track(packet, &track);
    assert(r == 0);
    if (nestegg_track_type(ne, track) == NESTEGG_TRACK_VIDEO) {
      cout << "video frame: " << ++video_count << " "; // Далее идем как с обычным ivf видео, если тип пакета - видео
      unsigned int count = 0;
      r = nestegg_packet_count(packet, &count);
      assert(r == 0);
      cout << "Count: " << count << " ";
      int nframes = 0;

      for (unsigned j=0; j < count; ++j) {
        unsigned char* data;
        size_t length;
        r = nestegg_packet_data(packet, j, &data, &length);
        assert(r == 0);

        vpx_codec_stream_info_t si;
        memset(&si, 0, sizeof(si));
        si.sz = sizeof(si);
        vpx_codec_peek_stream_info(interface, data, length, &si);
        cout << "keyframe: " << (si.is_kf ? "yes" : "no") << " ";

        cout << "length: " << length << " ";
        vpx_codec_err_t e = vpx_codec_decode(&codec, data, length, NULL, 0);
        if (e) {
          cerr << "Failed to decode frame. error: " << e << endl;
          return;
        }
       vpx_codec_iter_t  iter = NULL;
       vpx_image_t      *img;
        while((img = vpx_codec_get_frame(&codec, &iter))) {
          cout << "h: " << img->d_h << " w: " << img->d_w << endl;
          nframes++;

          if (glfwGetKey(platform->controll_window, GLFW_KEY_D ) == GLFW_PRESS) // Движение камеры по осям при
          {                                                                     // При нажатии клавиш
              cam_position.x += 0.1f;                                           //При желании юниформы можно удалить
              View = glm::lookAt(cam_position, glm::vec3(0,0,0), glm::vec3(0,1,0)); //Тогда будет обычный WebM плеер
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
          gl::glClear(gl::ClearBufferMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
          texture = gen_texture(img); //Генерируем текстуру из img
          buf1.render(4,0,gl::GLenum(GL_TRIANGLE_STRIP)); //Рисуем на прямоугольнике текстуру
          glfwSwapBuffers(platform->controll_window);
          glfwPollEvents();
          gl::glDeleteTextures(1,&texture); //Удаляем, что бы в gpu память не текла
        }

        cout << "nframes: " << nframes;
      }

      cout << endl;
    }

    if (nestegg_track_type(ne, track) == NESTEGG_TRACK_AUDIO) { //Аудио пакет не стал обрабатывать
      cout << "audio frame: " << ++audio_count << endl;
    }
    nestegg_free_packet(packet); //Чистим
  }
 if(vpx_codec_destroy(&codec)) {
    cerr << "Failed to destroy codec" << endl;
    return;
  }

  nestegg_destroy(ne);
  infile.close();
  gl::glDeleteVertexArrays(1,&VAO);
}

GLuint gen_texture(vpx_image_t *image) {
    uint8* pixels = (uint8 *)malloc(image->d_w*image->d_h * 3); //Организуем 3-х канальный буффер
    libyuv::I420ToRAW(image->planes[VPX_PLANE_Y],image->stride[VPX_PLANE_Y],
                            image->planes[VPX_PLANE_U],image->stride[VPX_PLANE_U],
                            image->planes[VPX_PLANE_V],image->stride[VPX_PLANE_V],
                            pixels,image->d_w * 3,image->d_w,image->d_h); //Переводим из yuv в RGB
    GLuint texture;
    gl::glGenTextures(1, &texture);
    gl::glBindTexture(gl::GLenum(GL_TEXTURE_2D), texture);
    gl::glTexParameteri(gl::GLenum(GL_TEXTURE_2D), gl::GLenum(GL_TEXTURE_MIN_FILTER), GL_LINEAR);  //Устанавливаем параметры текстур
    gl::glTexParameteri(gl::GLenum(GL_TEXTURE_2D), gl::GLenum(GL_TEXTURE_MAG_FILTER), GL_LINEAR);
    gl::glTexParameteri(gl::GLenum(GL_TEXTURE_2D), gl::GLenum(GL_TEXTURE_WRAP_S), GL_CLAMP_TO_EDGE);
    gl::glTexParameteri(gl::GLenum(GL_TEXTURE_2D), gl::GLenum(GL_TEXTURE_WRAP_T), GL_CLAMP_TO_EDGE);
    gl::glBindTexture(gl::GLenum(GL_TEXTURE_2D), texture); //Биндим её
    gl::glTexImage2D(gl::GLenum(GL_TEXTURE_2D), 0, GL_RGB, image->d_w,image->d_h,0,gl::GLenum(GL_RGB), gl::GLenum(GL_UNSIGNED_BYTE),pixels);
    free (pixels);
    return texture;
}

int main() {
    webm_start("big-buck-bunny_trailer.webm");
    return 0;
}
