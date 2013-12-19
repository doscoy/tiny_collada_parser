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

//  インプットデータ
struct InputData
{
    InputData()
        : semantic_(nullptr)
        , source_(nullptr)
        , offset_(0)
    {}

    const char* semantic_;
    const char* source_;
    int offset_;
};

//  ソースデータ
struct SourceData
{
    SourceData()
        : id_(nullptr)
        , stride_(0)
        , data_()
        , input_(nullptr)
    {}

    const char* id_;
    uint32_t stride_;
    std::vector<float> data_;
    InputData* input_;
};




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
        Indices indices_;

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
        , primitive_type_(UNKNOWN_TYPE)
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

    


public:
    ArrayData vertex_;
    ArrayData normal_;
    PrimitiveType primitive_type_;
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
    
    Result perseCollada(
        const tinyxml2::XMLDocument* const doc
    );
    
    void perseMeshNode(
        const tinyxml2::XMLElement* mesh_node,
        Mesh* data
    );

    void Perser::setupIndices(
        Indices& out,
        int start_offset,
        int stride
    );

    void readIndicesMaxOffset();
    

    void relateSourcesToInputs();
    InputData* searchInputBySource(const char* const id);
    SourceData* searchSourceBySemantic(const char* const semantic);
    void setupMesh(Mesh* mesh);
public:
    Indices raw_indices_;
    std::vector<Mesh> meshes_;
    std::vector<SourceData> sources_;
    std::vector<InputData> inputs_;
};






}   // namespace mcp

#endif // MCP_PARSER_HPP_INCLUDED
