//  mcp_parser.hpp

#ifndef TC_PARSER_HPP_INCLUDED
#define TC_PARSER_HPP_INCLUDED

// Include files.
#include <vector>
#include <cstdint>

//  無効なストライド幅
#define TINY_COLLADA_MESH_INVARIDATE_STRIDE  (-2)

namespace tc {

//  Colladaメッシュデータ
class Mesh
{
    //  法線を持っているか判定
    bool hasNormal() const {
        return stride_normal_ != TINY_COLLADA_MESH_INVARIDATE_STRIDE;
    }
    
    //  頂点を持っているか判定
    bool hasVertex() const {
        return stride_vertex_ != TINY_COLLADA_MESH_INVARIDATE_STRIDE;
    }


private:
    int8_t stride_vertex_;
    int8_t stride_normal_;
    
    std::vector<float> mesh_vertex_;
    std::vector<float> mesh_normal_;

    float transform_matrix_[4][4];
};

//  Simple Parser. it's use tinyxml2.
class Perser
{
public:
    Perser();
    ~Perser();

public:
    bool load(const char* const dae_path);

private:
    std::vector<Mesh*> mesh_;
};



}   // namespace mcp

#endif // MCP_PARSER_HPP_INCLUDED
