//  glutで表示


#include "../tiny_collada_parser.hpp"
#include "multiplatform.hpp"

namespace  {
    
const tc::Meshes* meshes_ = nullptr;
float zoom_ = 35;

//----------------------------------------------------------------------
//	
void keyboard(unsigned char key , int x , int y) {
    if (key == 'w') {
        zoom_ += 5;
    }
    else if (key == 's') {
        zoom_ -= 5;
    }
    printf("keyboard\n");
    if (zoom_ < 5) {
        zoom_ = 5;
    }
} 


//----------------------------------------------------------------------
//	メッシュの描画
void drawMesh(std::shared_ptr<tc::Mesh> mesh)
{
	//	頂点取得
    tc::Mesh::ArrayData* pos = mesh->getVertex();
	if (!pos) {
		return;
	}
	//	データ設定
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(pos->stride_, GL_FLOAT, 0, pos->data_.data());

	//	描画
    int draw_type = GL_TRIANGLES;
    glDrawElements(draw_type, pos->indices_.size(), GL_UNSIGNED_INT, pos->indices_.data());

	//	設定を戻す
	glDisableClientState(GL_VERTEX_ARRAY);
}

//----------------------------------------------------------------------
//	描画コールバック
void display()
{

	//	画面クリア
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

	std::shared_ptr<tc::Mesh> mesh = meshes_->at(0);

	//	カメラ設定
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, zoom_, zoom_, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	//	メッシュ描画
	drawMesh(mesh);

	glFlush();
}

//----------------------------------------------------------------------
//	スクリーンサイズ変更コールバック
void reshape(int width, int height)
{
	//	描画領域の設定変更
    glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)width / (double)height, 0.1, 10000.0);
}

}   // unname namespace

void timer(int value) 
{
	glutPostRedisplay();
	glutTimerFunc(50 , timer , 0);
}

//----------------------------------------------------------------------
//  sample
void dae_viewer(
    const char* const dae_path
) {
    tc::Parser parser;
    //  daeを解析
    tc::Result result = parser.parse(dae_path);
	if (result.isFailed()) {
		printf("data parse failed.\n%s\n",dae_path);
        return;
	}

    meshes_ = parser.meshes();

	//	gl設定
    int argc = 0;
    char* argv[] = {"\0","\0"};
//    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow("sample02");
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glutTimerFunc(100 , timer , 0);

	//	イベントループ
	glutMainLoop();
}