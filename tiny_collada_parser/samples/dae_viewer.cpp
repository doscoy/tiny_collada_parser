//  glutで表示


#include "../tiny_collada_parser.hpp"
#include "multiplatform.hpp"

namespace  {
    
const tc::ColladaScenes* scenes_ = nullptr;
float zoom_ = 4;
float height_ = 1;
const float ZOOM_VALUE = 2.0f;


struct vec3_t {
    float x_;
    float y_;
    float z_;
};


vec3_t getVertexData(
    const tc::ColladaMesh::ArrayData* m,
    int idx
) {

    int stride = m->stride_;
    int index = idx * stride;
    vec3_t v;
    v.x_ = m->data_[index +0];
    v.y_ = m->data_[index +1];
    v.z_ = m->data_[index +2];
    
    return v;
}


void drawLine(
    vec3_t start,
    vec3_t v
){
    
    glBegin(GL_TRIANGLES);
        glColor3ub(0xFF , 0 , 0);
        glVertex3f(
            start.x_,
            start.y_,
            start.z_
        );

        glColor3ub(0xFF , 0xFF , 0);
        glVertex3f(
            start.x_ + v.x_,
            start.y_ + v.y_,
            start.z_ + v.z_
        );

        glColor3ub(0xFF , 0xFF , 0);
        glVertex3f(
            start.x_+0.03f,
            start.y_+0.03f,
            start.z_
        );
	glEnd();
}



//----------------------------------------------------------------------
//	
void keyboard(unsigned char key , int x , int y) {
    if (key == 'w') {
        zoom_ += ZOOM_VALUE;
    }
    else if (key == 's') {
        zoom_ -= ZOOM_VALUE;
    }
    
    if (zoom_ < ZOOM_VALUE) {
        zoom_ = ZOOM_VALUE;
    }
    
    if (key == 'q') {
        height_ += ZOOM_VALUE;
    }
    else if (key == 'a') {
        height_ -= ZOOM_VALUE;
    }
    printf("zoom = %f height = %f\n", zoom_, height_);
    if (height_ < ZOOM_VALUE) {
        height_ = ZOOM_VALUE;
    }
    
} 


//----------------------------------------------------------------------
//	メッシュの描画
void drawMesh(std::shared_ptr<const tc::ColladaMesh> mesh)
{
    bool has_vertex = mesh->hasVertex();
    bool has_normal = mesh->hasNormal();
    
	//	頂点取得
    const tc::ColladaMesh::ArrayData* pos = mesh->getVertex();
    const tc::ColladaMesh::ArrayData* nor = mesh->getNormals();
	if (!pos) {
		return;
	}
	
    //	データ設定
    if (has_vertex) {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(pos->stride_, GL_FLOAT, 0, pos->data_.data());
    }
    if (has_normal) {
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, nor->data_.data());
    }
    

	//	描画
    int draw_type = GL_TRIANGLES;
    glDrawElements(draw_type, pos->indices_.size(), GL_UNSIGNED_INT, pos->indices_.data());

	//	設定を戻す
    if (has_vertex) {
        glDisableClientState(GL_VERTEX_ARRAY);
	}
    if (has_normal) {
        glDisableClientState(GL_NORMAL_ARRAY);
    }
}


//----------------------------------------------------------------------
//	描画コールバック
void display()
{

	//	画面クリア
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);
//    glShadeModel(GL_FLAT);

    static GLfloat lightPosition[4] = {0, 20, 0, 0.0f};
	static GLfloat lightDiffuse[3] = {1.0f, 1.0f, 1.0f};


	gluLookAt(0.0, zoom_ - height_, zoom_, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
    
	//	カメラ設定
	glMatrixMode(GL_MODELVIEW);


    //  ライト設定
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

    static float rot;
    rot += 0.8f;
        

	//	メッシュ描画
    int scene_count = scenes_->size();
    for (int i = 0; i < scene_count; ++i) {
        std::shared_ptr<const tc::ColladaScene> s = scenes_->at(i);
        std::shared_ptr<const tc::ColladaMaterial> material = s->material_;

        //  マテリアル設定
        if (!material->diffuse_.empty()){
            //  ディフューズ
            glMaterialfv(GL_FRONT, GL_DIFFUSE, material->diffuse_.data());
        }
        if (!material->ambient_.empty()) {
            //  アンビエント
            glMaterialfv(GL_FRONT, GL_AMBIENT, material->ambient_.data());
        }

        //  基本のSRT行列設定
        glLoadMatrixf(s->matrix_.data());

        //  メッシュ描画
        const tc::ColladaMeshes& ms = s->meshes_;
        for (int j = 0; j < ms.size(); ++j) {
            drawMesh(ms[j]);
        }

    }

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    
#if 0
    for (int i = 0; i < scene_count; ++i) {
        std::shared_ptr<const tc::ColladaScene> s = scenes_->at(i);
        const tc::ColladaMeshes& ms = s->meshes_;
        for (int j = 0; j < ms.size(); ++j) {        
            int vertex_count = ms[j]->getVertex()->data_.size() / 3;
            for (int k = 0; k < vertex_count; ++k) {
                vec3_t pos = getVertexData(ms[j]->getVertex() ,k);
                vec3_t nor = getVertexData(ms[j]->getNormals(), k);
                drawLine(pos, nor);
            }
        }
    
    }
#endif



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
	gluPerspective(45.0, (double)width / (double)height, 1, 20000.0);
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

    scenes_ = parser.scenes();

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