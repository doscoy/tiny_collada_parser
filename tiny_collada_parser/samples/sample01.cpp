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
        const tc::Indices& indices = mesh.getIndices();
        int index_size = indices.size();
        printf("idx count = %d\n", index_size);
        printf("primitive = %d\n", mesh.getPrimitiveType());
        
        for (int index_idx = 0; index_idx < index_size; ++index_idx) {
            printf("%d ", indices[index_idx]);
        }
        printf("\n");
        
        
        const tc::SourceMap* map = mesh.getSourceMap();
        tc::SourceMap::const_iterator end = map->end();
        
        for (tc::SourceMap::const_iterator it = map->begin(); it != end; ++it) {
            std::cout << it->first << std::endl;
            printf("  size %d\n", map->at("POSITION").size_);
            printf("  count %d\n", map->at("POSITION").count_);
        }
    }
}