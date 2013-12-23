//  glutで表示


#include "../tiny_collada_parser.hpp"
#include <GLUT/GLUT.h>

namespace  {
    
const tc::Meshes* meshes_ = nullptr;

void display()
{
    printf("disp\n");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex3f(0, 0, 0.5);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex3f(0.5, 0, 0.5);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex3f(-0.5, 0, -0.5);
	glEnd();
    
    glFlush();
}


void reshape(int width, int height)
{
    printf("reshape\n");
    glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)width / (double)height, 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.5, 2.5, 2.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

}   // unname namespace


//----------------------------------------------------------------------
//  sample
void sample02(
    const char* const dae_path
) {
    tc::Parser parser;
    //  daeを解析
    parser.parse(dae_path);
    meshes_ = parser.meshes();
    int argc = 0;

    glutInit(&argc, nullptr);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow("sample02");
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glutMainLoop();
}