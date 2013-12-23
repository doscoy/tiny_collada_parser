//  解析した内容をコンソールに出力


#include "../tiny_collada_parser.hpp"


//----------------------------------------------------------------------
//  sample
void sample01(
    const char* const dae_path
) {
    tc::Parser parser;
    //  daeを解析
    parser.parse(dae_path);

    //  解析結果取得
    const tc::Meshes* meshes = parser.meshes();
    printf("mesh list = %lu\n", meshes->size());
    
    
    for (int mesh_idx = 0; mesh_idx < meshes->size(); ++mesh_idx) {
        std::shared_ptr<tc::Mesh> mesh = meshes->at(mesh_idx);
        mesh->dump();
    }
}