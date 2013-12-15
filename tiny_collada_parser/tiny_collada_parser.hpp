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


typedef std::vector<uint16_t> Indices;

//  Colladaメッシュデータ
class Mesh
{
    friend class Perser;

public:
    class ArrayData
    {
    public:
        ArrayData()
            : stride_(TINY_COLLADA_MESH_INVARIDATE_STRIDE)
            , data_()
        {}
    
        ~ArrayData()
        {}
    
    public:
        bool isValidate() const {
            return stride_ != TINY_COLLADA_MESH_INVARIDATE_STRIDE;
        }
    
        void setStride(int8_t stride) {
            stride_ = stride;
        }
    
    
    private:
        int8_t stride_;
    public:
        std::vector<float> data_;
    };


public:
    enum PrimitiveType {
        LINE,
        LINE_STRIP,
        POLYGONS,
        POLYLIST,
        TRIANGLES,
        TRIANGLE_FAN,
        TRIANGLE_STRIP,
        
        PRIMITIVE_TYPE_NUM,
        
        UNKNOWN_TYPE
    };
    
public:
    Mesh()
        : vertex_()
        , normal_()
        , name_()
        , id_()
        , indices_()
        , primitive_type_(UNKNOWN_TYPE)
        ,transform_matrix_()
    {}
    ~Mesh(){}
    
public:
    //  法線を持っているか判定
    bool hasNormal() const {
        return normal_.isValidate();
    }
    
    //  頂点を持っているか判定
    bool hasVertex() const {
        return vertex_.isValidate();
    }

    //  IDを取得
    const std::string& getID() const {
        return id_;
    }
    
    const std::string& getName() const {
        return name_;
    }

    Indices* getIndices() {
        return &indices_;
    }
    
    const Indices* getIndices() const {
        return &indices_;
    }

    void setPrimitiveType(
        const PrimitiveType type
    ) {
        primitive_type_ = type;
    }
    
    PrimitiveType getPrimitiveType() const {
        return primitive_type_;
    }
    
    ArrayData* getVertex() {
        return &vertex_;
    }
    
    ArrayData* getNormals() {
        return &normal_;
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
    ArrayData vertex_;
    ArrayData normal_;
    std::string name_;
    std::string id_;
    Indices indices_;
    PrimitiveType primitive_type_;
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
    
    void perseMeshNode(
        const tinyxml2::XMLElement* mesh_node,
        Mesh* data
    );


    
private:
    std::vector<Mesh> meshes_;
};






}   // namespace mcp

#endif // MCP_PARSER_HPP_INCLUDED
