//  mcp_parser.hpp

#ifndef MCP_PARSER_HPP_INCLUDED
#define MCP_PARSER_HPP_INCLUDED

// Include files.
#include <vector>
#include <cstdint>

namespace mcp {

//  Mesh data manager.
class Mesh
{
private:
    int32_t
    
    std::vector<float> mesh_vertex_;
    std::vector<float>

    float transform_matrix_[4][4];
};

//  Simple Parser. it's use tinyxml2.
class Perser
{
public:
    Perser();
    ~Perser();

public:
    bool load

private:
    std::vector<Mesh*> mesh_;
};



}   // namespace mcp

#endif // MCP_PARSER_HPP_INCLUDED
