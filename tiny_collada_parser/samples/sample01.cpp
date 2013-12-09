//  mcp_parser.cpp


#include "../tiny_collada_parser.hpp"
#include <iostream>


void sample01(
    const char* const dae_path
) {
    tc::Perser perser;
    perser.perse(dae_path);

    const std::vector<tc::Mesh>* meshes = perser.getMeshList();
    printf("mesh list = %d\n", meshes->size());
    
    
    for (int i = 0; i < meshes->size(); ++i) {
        tc::Mesh mesh = meshes->at(i);
        
        printf("name = %s\n", mesh.getName().c_str());
        printf("id = %s\n", mesh.getID().c_str());
        const tc::SourceMap* map = mesh.getSourceMap();
        tc::SourceMap::const_iterator end = map->end();
        
        for (tc::SourceMap::const_iterator it = map->begin(); it != end; ++it) {
            std::cout << it->first << std::endl;
            printf("  size %d\n", map->at("POSITION").size_);
            printf("  count %d\n", map->at("POSITION").count_);
        }
    }
}