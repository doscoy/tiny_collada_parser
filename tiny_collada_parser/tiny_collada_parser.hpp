//  mcp_parser.hpp

#ifndef TC_PARSER_HPP_INCLUDED
#define TC_PARSER_HPP_INCLUDED

// Include files.
#include <vector>
#include <cstdint>

//  無効なストライド幅
#define TINY_COLLADA_MESH_INVARIDATE_STRIDE  (-2)


namespace tinyxml2 {
class XMLDocument;
}   // namespace tinyxml2

namespace tc {

//  リザルトコード
class Result
{
    friend class Perser;
public:
    enum Code {
        SUCCESS = 0,
        READ_ERROR,
        PERSE_ERROR,
    };


public:
    Result()
        : code_(SUCCESS)
    {}
    
    Result(Code code)
        : code_(code)
    {}

public:
    bool isSucceed() const {
        return code_ == SUCCESS;
    }
    
    bool isFailed() const {
        return code_ != SUCCESS;
    }
    
    //  現在のエラーコードを取得
    Code getErrorCode() const {
        return code_;
    }
    
private:
    //  エラーコードを設定
    void setErrorCode(
        Code code
    ) {
        code_ = code;
    }
    
    
private:
    Code code_;
};

//  Colladaメッシュデータ
class Mesh
{
public:
    Mesh();
    ~Mesh();
    
public:
    //  法線を持っているか判定
    bool hasNormal() const {
        return stride_normal_ != TINY_COLLADA_MESH_INVARIDATE_STRIDE;
    }
    
    //  頂点を持っているか判定
    bool hasVertex() const {
        return stride_vertex_ != TINY_COLLADA_MESH_INVARIDATE_STRIDE;
    }

    //  


private:
    int8_t stride_vertex_;
    int8_t stride_normal_;
    
    std::vector<float> mesh_vertex_;
    std::vector<float> mesh_normal_;

    float transform_matrix_[4][4];
};



//  パーサー
class Perser
{
public:
    Perser();
    ~Perser();

public:
    Result perse(const char* const dae_file_path);

private:
    Result loadDae(
        tinyxml2::XMLDocument* const doc,
        const char* const dae_file_path
    );
    
    Result perseDae(
        const tinyxml2::XMLDocument* const doc
    );

private:
    std::vector<Mesh*> mesh_;
};






}   // namespace mcp

#endif // MCP_PARSER_HPP_INCLUDED
