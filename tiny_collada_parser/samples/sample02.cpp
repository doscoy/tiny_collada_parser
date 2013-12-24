//  glutで表示


#include "../tiny_collada_parser.hpp"
#include "multiplatform.hpp"

namespace  {
    
const tc::Meshes* meshes_ = nullptr;


void drawMesh()
{
    std::shared_ptr<tc::Mesh> mesh = meshes_->at(0);
    tc::Mesh::ArrayData* pos = mesh->getVertex();

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(pos->stride_, GL_FLOAT, 0, pos->data_.data());
    glDrawElements(GL_TRIANGLES, pos->indices_.size(), GL_UNSIGNED_INT, pos->indices_.data());
    glDisableClientState(GL_VERTEX_ARRAY);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    drawMesh();
    glFlush();
}


void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)width / (double)height, 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 5.5, 5.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
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
    char* argv[] = {"\0","\0"};
//    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow("sample02");
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glutMainLoop();
}