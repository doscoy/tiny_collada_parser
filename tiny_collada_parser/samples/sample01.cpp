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
        
        //  インデックス値ダンプ
        const tc::Indices* indices = &perser.raw_indices_;
        printf("idx count = %d\n", indices->size());
        for (int index_idx = 0; index_idx < indices->size(); ++index_idx) {
            printf("%d ", indices->at(index_idx));
        }
        
        printf("\n\n");

        printf("primitive = %d\n\n", mesh.getPrimitiveType());


        //  頂点
        if (mesh.hasVertex()) {
            
            printf("vertex\n\n");
            tc::Mesh::ArrayData* vad = mesh.getVertex();
            std::vector<float>& ary = vad->data_;
            for (int adi = 0; adi < ary.size(); ++adi) {
                printf("%f ", ary[adi]);
            }
            printf("\n");
            tc::Indices& ind = vad->indices_;
            for (int adi = 0; adi < ind.size(); ++adi) {
                printf("%d ", ind[adi]);
            }
        }
        else {
            printf("has not vertex...\n\n");
            
        }

        //  法線
        if (mesh.hasNormal()) {
            
            printf("normal\n\n");
            tc::Mesh::ArrayData* nad = mesh.getNormals();
            std::vector<float>& ary = nad->data_;
            for (int adi = 0; adi < ary.size(); ++adi) {
                printf("%f ", ary[adi]);
            }
            printf("\n");
            tc::Indices& ind = nad->indices_;
            for (int adi = 0; adi < ind.size(); ++adi) {
                printf("%d ", ind[adi]);
            }
            printf("\n");

        }
        else {
            printf("has not normal...\n\n");
            
        }
    }
}