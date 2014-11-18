	TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

INCLUDEPATH += /usr/local/include/backend/ \
              /usr/local/include/system/

LIBS += -lbackend -lsystem -lglbinding -lyuv
QMAKE_CXXFLAGS += -std=c++11
win32: LIBS += -lopengl32 -lglfw3 -lvpx -lyuv -lglbinding
unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += gl
    PKGCONFIG += glfw3
    PKGCONFIG += nestegg
    PKGCONFIG += vpx
}
include(deployment.pri)
qtcAddDeployment()

shaders_copy.commands += $(COPY_DIR) $$PWD/vert.glsl $$OUT_PWD/; \
                         $(COPY_DIR) $$PWD/frag.glsl $$OUT_PWD/; \
                         $(COPY_DIR) $$PWD/big-buck-bunny_trailer.webm $$OUT_PWD/;
first.depends = $(first) shaders_copy
export(first.depends)
export(shaders_copy.commands)
QMAKE_EXTRA_TARGETS += first shaders_copy
HEADERS += \
    webm_header.h

OTHER_FILES += \
    vert.glsl \
    frag.glsl
