//  mcp_parser.cpp


#include "../tiny_collada_parser.hpp"
#include <iostream>

//----------------------------------------------------------------------
//  sample
void sample01(
    const char* const dae_path
) {
    tc::Perser perser;
    //  daeを解析
    perser.perse(dae_path);

    //  解析結果取得
    const std::vector<tc::Mesh>* meshes = perser.getMeshList();
    printf("mesh list = %lu\n", meshes->size());
    
    
    for (int i = 0; i < meshes->size(); ++i) {
        tc::Mesh mesh = meshes->at(i);
        
        printf("name = %s\n", mesh.getName().c_str());
        printf("id = %s\n", mesh.getID().c_str());
        
        //  インデックス値ダンプ
        const tc::Indices* indices = mesh.getIndices();
        int index_size = indices->size();
        printf("idx count = %d\n", index_size);
        for (int index_idx = 0; index_idx < index_size; ++index_idx) {
            printf("%d ", indices->at(index_idx));
        }
        
        printf("\n\n");

        printf("primitive = %d\n\n", mesh.getPrimitiveType());


        //  頂点
        if (mesh.hasVertex()) {
            
            printf("vertex\n\n");
            tc::Mesh::ArrayData* vad = mesh.getVertex();
        }
        else {
            printf("has not vertex...\n\n");
            
        }

        //  法線
        if (mesh.hasNormal()) {
            
            printf("normal\n\n");
            tc::Mesh::ArrayData* nad = mesh.getNormals();
            
        }
        else {
            printf("has not normal...\n\n");
            
        }
    }
}