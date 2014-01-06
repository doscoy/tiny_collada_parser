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
    const tc::ColladaScenes* scenes = parser.scenes();
    printf("mesh list = %lu\n", scenes->size());
    
    
    for (int scene_idx = 0; scene_idx < scenes->size(); ++scene_idx) {
        std::shared_ptr<tc::ColladaScene> scene = scenes->at(scene_idx);
        scene->dump();
    }
}

