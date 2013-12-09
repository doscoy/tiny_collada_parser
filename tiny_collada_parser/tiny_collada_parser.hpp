//  mcp_parser.hpp

#ifndef TC_PARSER_HPP_INCLUDED
#define TC_PARSER_HPP_INCLUDED

// Include files.
#include <vector>
#include <cstdint>
#include <string>
#include <map>



//  無効なストライド幅
#define TINY_COLLADA_MESH_INVARIDATE_STRIDE  (-2)


namespace tinyxml2 {
class XMLDocument;
class XMLElement;
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


struct SourceData {
  int32_t stride_;
  int32_t size_;
  int32_t count_;
  void* data_;
};

typedef std::map<std::string, SourceData> SourceMap;





//  Colladaメッシュデータ
class Mesh
{
    friend class Perser;
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

    //  IDを取得
    const std::string& getID() const {
        return id_;
    }
    
    const std::string& getName() const {
        return name_;
    }

    const SourceMap* getSourceMap() const {
        return &map_;
    }


private:
    //  IDを設定
    void setID(const char* const id) {
        id_ = id;
    }
    
    //  名前を設定
    void setName(const char* const name) {
        name_ = name;
    }

    
    


private:
    int8_t stride_vertex_;
    int8_t stride_normal_;
    SourceMap map_;
    std::vector<float> mesh_vertex_;
    std::vector<float> mesh_normal_;
    std::string name_;
    std::string id_;
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
    
    const std::vector<Mesh>* getMeshList() const {
        return &meshes_;
    }
private:
    Result loadDae(
        tinyxml2::XMLDocument* const doc,
        const char* const dae_file_path
    );
    
    Result perseDae(
        const tinyxml2::XMLDocument* const doc
    );
    
    
    SourceData readSource(
        const tinyxml2::XMLElement* const source
    );
private:
    std::vector<Mesh> meshes_;
};






}   // namespace mcp

#endif // MCP_PARSER_HPP_INCLUDED
