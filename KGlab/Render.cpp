#include "Render.h"
#include <Windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "GUItextRectangle.h"
#include "MyShaders.h"
#include "Texture.h"
#include "ObjLoader.h"
#include "debout.h"

//внутренняя логика "движка"
#include "MyOGL.h"
extern OpenGL gl;
#include "Light.h"
Light light;
#include "Camera.h"
Camera camera;

bool texturing = true;
bool lightning = true;
bool alpha = false;

void switchModes(OpenGL* sender, KeyEventArg arg)
{
    auto key = LOWORD(MapVirtualKeyA(arg.key, MAPVK_VK_TO_CHAR));
    switch (key)
    {
    case 'L': lightning = !lightning; break;
    case 'T': texturing = !texturing; break;
    case 'A': alpha = !alpha; break;
    }
}

template<typename T, int M1, int N1, int M2, int N2>
void MatrixMultiply(const T* a, const T* b, T* c)
{
    for (int i = 0; i < M1; ++i)
    {
        for (int j = 0; j < N2; ++j)
        {
            c[i * N2 + j] = T(0);
            for (int k = 0; k < N1; ++k)
            {
                c[i * N2 + j] += a[i * N1 + k] * b[k * N2 + j];
            }
        }
    }
}

GuiTextRectangle text;
GLuint texId;
ObjModel f;

Shader cassini_sh;
Shader phong_sh;
Shader vb_sh;
Shader simple_texture_sh;

// Только текстура аметиста
Texture amethyst_tex;

void initRender()
{
    cassini_sh.VshaderFileName = "shaders/v.vert";
    cassini_sh.FshaderFileName = "shaders/cassini.frag";
    cassini_sh.LoadShaderFromFile();
    cassini_sh.Compile();

    phong_sh.VshaderFileName = "shaders/v.vert";
    phong_sh.FshaderFileName = "shaders/light.frag";
    phong_sh.LoadShaderFromFile();
    phong_sh.Compile();

    vb_sh.VshaderFileName = "shaders/v.vert";
    vb_sh.FshaderFileName = "shaders/vb.frag";
    vb_sh.LoadShaderFromFile();
    vb_sh.Compile();

    simple_texture_sh.VshaderFileName = "shaders/v.vert";
    simple_texture_sh.FshaderFileName = "shaders/textureShader.frag";
    simple_texture_sh.LoadShaderFromFile();
    simple_texture_sh.Compile();

    // Загружаем только текстуру аметиста
    amethyst_tex.LoadTexture("textures/amethyst.png");

    f.LoadModel("models//monkey.obj_m");
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    camera.caclulateCameraPos();
    gl.WheelEvent.reaction(&camera, &Camera::Zoom);
    gl.MouseMovieEvent.reaction(&camera, &Camera::MouseMovie);
    gl.MouseLeaveEvent.reaction(&camera, &Camera::MouseLeave);
    gl.MouseLdownEvent.reaction(&camera, &Camera::MouseStartDrag);
    gl.MouseLupEvent.reaction(&camera, &Camera::MouseStopDrag);

    gl.MouseMovieEvent.reaction(&light, &Light::MoveLight);
    gl.KeyDownEvent.reaction(&light, &Light::StartDrug);
    gl.KeyUpEvent.reaction(&light, &Light::StopDrug);

    gl.KeyDownEvent.reaction(switchModes);
    text.setSize(512, 180);

    camera.setPosition(2, 1.5, 1.5);
}

float view_matrix[16];
double full_time = 0;
int location = 0;

void Render(double delta_time)
{
    full_time += delta_time;

    if (gl.isKeyPressed('F'))
    {
        light.SetPosition(camera.x(), camera.y(), camera.z());
    }
    camera.SetUpCamera();
    glGetFloatv(GL_MODELVIEW_MATRIX, view_matrix);
    light.SetUpLight();
    gl.DrawAxes();
    glBindTexture(GL_TEXTURE_2D, 0);

    glEnable(GL_NORMALIZE);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    if (lightning) glEnable(GL_LIGHTING);
    if (texturing)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    if (alpha)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    float  amb[] = { 0.2, 0.2, 0.1, 1. };
    float dif[] = { 0.4, 0.65, 0.5, 1. };
    float spec[] = { 0.9, 0.8, 0.3, 1. };
    float sh = 0.2f * 256;

    glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
    glMaterialf(GL_FRONT, GL_SHININESS, sh);
    glShadeModel(GL_SMOOTH);

    // Отрисовка призмы с текстурой аметиста
    double vysota = 2;
    double A[]{ 0,0,0 }, B[]{ 0,3,0 }, C[]{ 7,6,0 }, D[]{ 1,0,0 };
    double A_v[]{ 0,0,vysota }, B_v[]{ 0,3,vysota }, C_v[]{ 7,6,vysota }, D_v[]{ 1,0,vysota };
    double E[]{ 5,-7,0 }, F[]{ -2,-9,0 };
    double E_v[]{ 5,-7,vysota }, F_v[]{ -2,-9,vysota };
    double G[]{ -7,-4,0 }, H[]{ -2,0,0 };
    double G_v[]{ -7,-4,vysota }, H_v[]{ -2,0,vysota };
    double I[]{ -4,6,0 }, I_v[]{ -4,6,vysota };

    // Включаем текстуру аметиста перед отрисовкой призмы
    if (texturing) {
        glEnable(GL_TEXTURE_2D);
        amethyst_tex.Bind();
    }

    glBegin(GL_QUADS);
    // Нижняя грань
    glNormal3d(0, 0, -1);
    glTexCoord2d(0, 0); glVertex3dv(A);
    glTexCoord2d(1, 0); glVertex3dv(B);
    glTexCoord2d(1, 1); glVertex3dv(C);
    glTexCoord2d(0, 1); glVertex3dv(D);

    // Верхняя грань
    glNormal3d(0, 0, 1);
    glTexCoord2d(0, 0); glVertex3dv(A_v);
    glTexCoord2d(1, 0); glVertex3dv(B_v);
    glTexCoord2d(1, 1); glVertex3dv(C_v);
    glTexCoord2d(0, 1); glVertex3dv(D_v);

    // Боковые грани с текстурными координатами
    // Часть 1: боковая грань (B, B_v, C_v, C)
    glNormal3d(-0.394, 0.919, 0);
    glTexCoord2d(0, 0); glVertex3dv(B);
    glTexCoord2d(1, 0); glVertex3dv(B_v);
    glTexCoord2d(1, 1); glVertex3dv(C_v);
    glTexCoord2d(0, 1); glVertex3dv(C);

    // Часть 1: боковая грань (C, C_v, D_v, D)
    glNormal3d(0.624, -0.781, 0);
    glTexCoord2d(0, 0); glVertex3dv(C);
    glTexCoord2d(1, 0); glVertex3dv(C_v);
    glTexCoord2d(1, 1); glVertex3dv(D_v);
    glTexCoord2d(0, 1); glVertex3dv(D);

    // Часть 2: нижняя грань (A, D, E, F)
    glNormal3d(0, 0, -1);
    glTexCoord2d(0, 0); glVertex3dv(A);
    glTexCoord2d(1, 0); glVertex3dv(D);
    glTexCoord2d(1, 1); glVertex3dv(E);
    glTexCoord2d(0, 1); glVertex3dv(F);

    // Часть 2: верхняя грань (A_v, D_v, E_v, F_v)
    glNormal3d(0, 0, 1);
    glTexCoord2d(0, 0); glVertex3dv(A_v);
    glTexCoord2d(1, 0); glVertex3dv(D_v);
    glTexCoord2d(1, 1); glVertex3dv(E_v);
    glTexCoord2d(0, 1); glVertex3dv(F_v);

    // Часть 2: боковая грань (D, D_v, E_v, E)
    glNormal3d(0.961, 0.274, 0);
    glTexCoord2d(0, 0); glVertex3dv(D);
    glTexCoord2d(1, 0); glVertex3dv(D_v);
    glTexCoord2d(1, 1); glVertex3dv(E_v);
    glTexCoord2d(0, 1); glVertex3dv(E);

    // Часть 2: боковая грань (E, E_v, F_v, F)
    glNormal3d(-0.781, -0.624, 0);
    glTexCoord2d(0, 0); glVertex3dv(E);
    glTexCoord2d(1, 0); glVertex3dv(E_v);
    glTexCoord2d(1, 1); glVertex3dv(F_v);
    glTexCoord2d(0, 1); glVertex3dv(F);

    // Часть 3: нижняя грань (A, F, G, H)
    glNormal3d(0, 0, -1);
    glTexCoord2d(0, 0); glVertex3dv(A);
    glTexCoord2d(1, 0); glVertex3dv(F);
    glTexCoord2d(1, 1); glVertex3dv(G);
    glTexCoord2d(0, 1); glVertex3dv(H);

    // Часть 3: верхняя грань (A_v, F_v, G_v, H_v)
    glNormal3d(0, 0, 1);
    glTexCoord2d(0, 0); glVertex3dv(A_v);
    glTexCoord2d(1, 0); glVertex3dv(F_v);
    glTexCoord2d(1, 1); glVertex3dv(G_v);
    glTexCoord2d(0, 1); glVertex3dv(H_v);

    // Часть 3: боковая грань (F, F_v, G_v, G)
    glNormal3d(0.196, -0.980, 0);
    glTexCoord2d(0, 0); glVertex3dv(F);
    glTexCoord2d(1, 0); glVertex3dv(F_v);
    glTexCoord2d(1, 1); glVertex3dv(G_v);
    glTexCoord2d(0, 1); glVertex3dv(G);

    // Часть 3: боковая грань (G, G_v, H_v, H)
    glNormal3d(-0.857, -0.514, 0);
    glTexCoord2d(0, 0); glVertex3dv(G);
    glTexCoord2d(1, 0); glVertex3dv(G_v);
    glTexCoord2d(1, 1); glVertex3dv(H_v);
    glTexCoord2d(0, 1); glVertex3dv(H);

    // Часть 4: нижняя грань (A, H, I, B)
    glNormal3d(0, 0, -1);
    glTexCoord2d(0, 0); glVertex3dv(A);
    glTexCoord2d(1, 0); glVertex3dv(H);
    glTexCoord2d(1, 1); glVertex3dv(I);
    glTexCoord2d(0, 1); glVertex3dv(B);

    // Часть 4: верхняя грань (A_v, H_v, I_v, B_v)
    glNormal3d(0, 0, 1);
    glTexCoord2d(0, 0); glVertex3dv(A_v);
    glTexCoord2d(1, 0); glVertex3dv(H_v);
    glTexCoord2d(1, 1); glVertex3dv(I_v);
    glTexCoord2d(0, 1); glVertex3dv(B_v);

    // Часть 4: боковая грань (H, H_v, I_v, I)
    glNormal3d(-0.624, 0.781, 0);
    glTexCoord2d(0, 0); glVertex3dv(H);
    glTexCoord2d(1, 0); glVertex3dv(H_v);
    glTexCoord2d(1, 1); glVertex3dv(I_v);
    glTexCoord2d(0, 1); glVertex3dv(I);

    // Часть 4: боковая грань (I, I_v, B_v, B)
    glNormal3d(0.857, 0.514, 0);
    glTexCoord2d(0, 0); glVertex3dv(I);
    glTexCoord2d(1, 0); glVertex3dv(I_v);
    glTexCoord2d(1, 1); glVertex3dv(B_v);
    glTexCoord2d(0, 1); glVertex3dv(B);
    glEnd();

    // Отключаем текстуру после отрисовки призмы
    glBindTexture(GL_TEXTURE_2D, 0);

    // Остальной код без изменений
    glLoadIdentity();
    camera.SetUpCamera();
    Shader::DontUseShaders();
    light.DrawLightGizmo();

    glActiveTexture(GL_TEXTURE0);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, gl.getWidth() - 1, 0, gl.getHeight() - 1, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    std::wstringstream ss;
    ss << std::fixed << std::setprecision(3);
    ss << "T - " << (texturing ? L"[вкл]выкл  " : L" вкл[выкл] ") << L"текстур" << std::endl;
    ss << "L - " << (lightning ? L"[вкл]выкл  " : L" вкл[выкл] ") << L"освещение" << std::endl;
    ss << "A - " << (alpha ? L"[вкл]выкл  " : L" вкл[выкл] ") << L"альфа-наложение" << std::endl;
    ss << L"F - Свет из камеры" << std::endl;
    ss << L"G - двигать свет по горизонтали" << std::endl;
    ss << L"G+ЛКМ двигать свет по вертекали" << std::endl;
    ss << L"Коорд. света: (" << std::setw(7) << light.x() << "," << std::setw(7) << light.y() << "," << std::setw(7) << light.z() << ")" << std::endl;
    ss << L"Коорд. камеры: (" << std::setw(7) << camera.x() << "," << std::setw(7) << camera.y() << "," << std::setw(7) << camera.z() << ")" << std::endl;
    ss << L"Параметры камеры: R=" << std::setw(7) << camera.distance() << ",fi1=" << std::setw(7) << camera.fi1() << ",fi2=" << std::setw(7) << camera.fi2() << std::endl;
    ss << L"delta_time: " << std::setprecision(5) << delta_time << std::endl;
    ss << L"full_time: " << std::setprecision(2) << full_time << std::endl;

    text.setPosition(10, gl.getHeight() - 10 - 180);
    text.setText(ss.str().c_str());
    text.Draw();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}