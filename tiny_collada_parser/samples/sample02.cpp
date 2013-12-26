//  glutで表示


#include "../tiny_collada_parser.hpp"
#include "multiplatform.hpp"

namespace  {
    
const tc::Meshes* meshes_ = nullptr;

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
    glDrawElements(GL_TRIANGLES, pos->indices_.size(), GL_UNSIGNED_INT, pos->indices_.data());

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

	std::shared_ptr<tc::Mesh> mesh = meshes_->at(0);

	//	カメラ設定
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 5.5, 5.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	//	メッシュ描画B
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
	gluPerspective(45.0, (double)width / (double)height, 0.1, 1000.0);
}

}   // unname namespace


//----------------------------------------------------------------------
//  sample
void sample02(
    const char* const dae_path
) {
    tc::Parser parser;
    //  daeを解析
    tc::Result result = parser.parse(dae_path);
	if (result.isFailed()) {
		printf("data parse failed.\n%s\n",dae_path);
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
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

	//	イベントループ
	glutMainLoop();
}