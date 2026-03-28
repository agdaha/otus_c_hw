#include <GL/freeglut.h>
#include <GL/gl.h>
#include <stdio.h>

static GLfloat rotation_angle_x = 0.0f;
static GLfloat rotation_angle_y = 0.0f;
static GLfloat rotation_angle_z = 0.0f;

GLuint textures[6] = {0}; // 6 текстур для каждой грани

// Цвета граней кубика
static const GLubyte side_colors[6][3] = {
    {255, 255, 255}, // белый
    {255, 255, 0},   // желтый
    {255, 0, 0},     // краный
    {255, 165, 0},   // оранжевый
    {0, 0, 255},     // синий
    {0, 255, 0}};    // зеленый

// Создание текстур для граней
void create_textures(void)
{
    const int texture_size = 24;
    const int cell_size = 8;
    const int line_width = 1;

    glGenTextures(6, textures);

    for (int f = 0; f < 6; f++)
    {
        GLubyte image[texture_size][texture_size][3];

        for (int i = 0; i < texture_size; i++)
        {
            for (int j = 0; j < texture_size; j++)
            {
                int is_line = (i % cell_size < line_width) || (i % cell_size >= cell_size - line_width) ||
                              (j % cell_size < line_width) || (j % cell_size >= cell_size - line_width);

                if (is_line)
                {
                    // Серые линии контуров между клетками
                    image[i][j][0] = 50;
                    image[i][j][1] = 50;
                    image[i][j][2] = 50;
                }
                else
                {
                    // Цвет грани
                    image[i][j][0] = side_colors[f][0];
                    image[i][j][1] = side_colors[f][1];
                    image[i][j][2] = side_colors[f][2];
                }
            }
        }

        glBindTexture(GL_TEXTURE_2D, textures[f]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_size, texture_size, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    }
}

// Обработка нажатия на клавишу Esc
void handle_keypress(unsigned char key, __attribute__((unused)) int x, __attribute__((unused)) int y)
{
    //(void)x;
    //(void)y;
    if (key == 27)
    {
        glutLeaveMainLoop(); // Завершение жизненного цикла
    }
}

// Функция отрисовки одной грани куба с текстурными координатами
void draw_side(float x1, float y1, float z1,
               float x2, float y2, float z2,
               float x3, float y3, float z3,
               float x4, float y4, float z4)
{
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(x1, y1, z1);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(x2, y2, z2);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(x3, y3, z3);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(x4, y4, z4);
    glEnd();
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -6.0f);

    // Вращаем по всем осям
    glRotatef(rotation_angle_x, 1.0f, 0.0f, 0.0f);
    glRotatef(rotation_angle_y, 0.0f, 1.0f, 0.0f);
    glRotatef(rotation_angle_z, 0.0f, 0.0f, 1.0f);

    glEnable(GL_TEXTURE_2D);

    // Отрисовка куба
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    draw_side(-1, -1, 1, 1, -1, 1, 1, 1, 1, -1, 1, 1); // перед
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    draw_side(-1, -1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1);
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    draw_side(-1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1);
    glBindTexture(GL_TEXTURE_2D, textures[3]);
    draw_side(-1, -1, -1, 1, -1, -1, 1, -1, 1, -1, -1, 1);
    glBindTexture(GL_TEXTURE_2D, textures[4]);
    draw_side(1, -1, -1, 1, 1, -1, 1, 1, 1, 1, -1, 1);
    glBindTexture(GL_TEXTURE_2D, textures[5]);
    draw_side(-1, -1, -1, -1, -1, 1, -1, 1, 1, -1, 1, -1);

    glDisable(GL_TEXTURE_2D);
    glutSwapBuffers();
}

// Обновление угла вращения
void timer(__attribute__((unused)) int value)
{
    //(void)value;
    rotation_angle_x += 0.7f;
    rotation_angle_y += 1.3f;
    rotation_angle_z += 0.5f;
    if (rotation_angle_x > 360)
        rotation_angle_x -= 360;
    if (rotation_angle_x < 0)
        rotation_angle_x += 360;
    if (rotation_angle_y > 360)
        rotation_angle_y -= 360;
    if (rotation_angle_y < 0)
        rotation_angle_y += 360;
    if (rotation_angle_z > 360)
        rotation_angle_z -= 360;
    if (rotation_angle_z < 0)
        rotation_angle_z += 360;

    glutPostRedisplay();
    glutTimerFunc(8, timer, 0);
}

// Настройка проекции при изменении размера окна
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Rotating Rubik's Cube");

    glEnable(GL_DEPTH_TEST);
    create_textures();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handle_keypress);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    exit(EXIT_SUCCESS);
}
