/*
 * Code adapted from:
 *   https://www.opengl.org/wiki/Tutorial:_OpenGL_3.0_Context_Creation_(GLX)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>

#include "common/error.h"

#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092

typedef GLXContext (*glXCreateContextAttribsARBProc)
    (Display*, GLXFBConfig, GLXContext, Bool, const int*);

const int visual_attribs[] = {
    GLX_X_RENDERABLE    , true,
    GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
    GLX_RENDER_TYPE     , GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
    GLX_RED_SIZE        , 8,
    GLX_GREEN_SIZE      , 8,
    GLX_BLUE_SIZE       , 8,
    GLX_ALPHA_SIZE      , 8,
    GLX_DEPTH_SIZE      , 24,
    GLX_STENCIL_SIZE    , 8,
    GLX_DOUBLEBUFFER    , true,
    //GLX_SAMPLE_BUFFERS  , 1,
    //GLX_SAMPLES         , 4,
    None
};

void checkGlxVersion(Display* display) {
    int glx_major, glx_minor;
    // FBConfigs were added in GLX version 1.3.
    if (!glXQueryVersion(display, &glx_major, &glx_minor) ||
            ((glx_major == 1) && (glx_minor < 3)) || (glx_major < 1))
        fail("Invalid GLX version");
}

GLXFBConfig chooseFBConfig(Display* display) {
    printf("Getting matching framebuffer configs\n");
    int fbcount;
    GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display),
                                         visual_attribs, &fbcount);
    if (!fbc)
        fail("Failed to retrieve a framebuffer config\n");
    printf("Found %d matching FB configs.\n", fbcount);

    // Pick the FB config/visual with the most samples per pixel
    printf("Getting XVisualInfos\n");
    int best_fbc = -1, worst_fbc = -1, best_num_samp = -1,
        worst_num_samp = 999;

    for (int i = 0; i < fbcount; ++i) {
        XVisualInfo *vi = glXGetVisualFromFBConfig(display, fbc[i]);
        if (vi) {
            int samp_buf, samples;
            glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLE_BUFFERS,
                                 &samp_buf);
            glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLES, &samples);
            printf("\tMatching fbconfig %d, visual ID 0x%2lx: "
                   "SAMPLE_BUFFERS = %d, SAMPLES = %d\n",
                    i, vi -> visualid, samp_buf, samples);

            if (best_fbc < 0 || (samp_buf && samples) > best_num_samp)
                best_fbc = i, best_num_samp = samples;
            if (worst_fbc < 0 || !samp_buf || samples < worst_num_samp)
                worst_fbc = i, worst_num_samp = samples;
        }
        XFree(vi);
    }
    XFree(fbc);
    return fbc[best_fbc];
}

GLXContext createContext(Display* display, GLXFBConfig bestFbc) {
    // NOTE: It is not necessary to create or make current to a context before
    // calling glXGetProcAddressARB
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
        glXGetProcAddressARB((const GLubyte *) "glXCreateContextAttribsARB");

    int context_attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 0,
        //GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        None
    };
    printf("Creating context\n");
    GLXContext context = 0;
    context = glXCreateContextAttribsARB(display, bestFbc, 0,
              true, context_attribs);
    // Sync to ensure any errors generated are processed.
    XSync(display, false);
    if (context) printf("Created GL 3.0 context\n");
    else         fail("Failed to create GL 3.0 context\n");
    // Sync to ensure any errors generated are processed.
    XSync(display, false);

    if (! glXIsDirect(display, context))
        printf("Indirect GLX rendering context obtained\n");
    else
        printf("Direct GLX rendering context obtained\n");
    return context;
}
void initGlew() {
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if(err != GLEW_OK)
        fail("glewInit failed: %s\n", glewGetErrorString(err));
    printGlErrors();
}

int main(int argc, char* argv[]) {
    Display *display = XOpenDisplay(NULL);
    if (!display)
        fail("Failed to open X display\n");
    checkGlxVersion(display);
    auto bestFbc = chooseFBConfig(display);

    // Get a visual
    XVisualInfo *vi = glXGetVisualFromFBConfig(display, bestFbc);
    printf("Chosen visual ID = 0x%lx\n", vi->visualid);

    printf("Creating colormap\n");
    XSetWindowAttributes swa;
    Colormap cmap;
    swa.colormap = cmap = XCreateColormap(display,
            RootWindow(display, vi->screen),
            vi->visual, AllocNone);
    swa.background_pixmap = None ;
    swa.border_pixel      = 0;
    swa.event_mask        = StructureNotifyMask;

    printf("Creating window\n");
    Window win = XCreateWindow(display, RootWindow(display, vi->screen),
            0, 0, 100, 100, 0, vi->depth, InputOutput,
            vi->visual,
            CWBorderPixel|CWColormap|CWEventMask, &swa);
    if (!win)
        fail("Failed to create window.\n");

    // Done with the visual info data
    XFree(vi);
    XStoreName(display, win, "GL 3.0 Window");
    printf("Mapping window\n");
    XMapWindow(display, win);

    auto context = createContext(display, bestFbc);
    printf("Making context current\n");
    glXMakeCurrent(display, win, context);
    initGlew();

    GLuint tex_;
    GLuint fbo_;
    glGenTextures(1, &tex_);
    glBindTexture(GL_TEXTURE_2D, tex_);

    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           tex_,
                           0);
    printGlErrors();

    glBindTexture(GL_TEXTURE_2D, tex_);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_RGBA8, 2, 2);
    char source_pixels[16] = {
        1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4
    };
    printGlErrors();
    printf("Crash here\n");
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0, 0,
                    2, 2,
                    GL_RGBA, GL_UNSIGNED_BYTE,
                    source_pixels);
    printf("Crashed :( \n");
    printGlErrors();

    glCheckFramebufferStatus(GL_FRAMEBUFFER);

    char pixels[16] = {0};
    glReadPixels(0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glCheckFramebufferStatus(GL_FRAMEBUFFER);
    printGlErrors();

    glXMakeCurrent(display, 0, 0);
    glXDestroyContext(display, context);
    XDestroyWindow(display, win);
    XFreeColormap(display, cmap);
    XCloseDisplay(display);
    printf("Test success\n");
    return 0;
}
