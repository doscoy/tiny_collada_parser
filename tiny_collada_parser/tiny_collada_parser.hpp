//  mcp_parser.hpp

#ifndef TC_PARSER_HPP_INCLUDED
#define TC_PARSER_HPP_INCLUDED

// Include files.
#include <vector>
#include <cstdint>




namespace tc {


//  リザルトコード
class Result
{
    friend class Parser;
public:
    enum class Code {
        SUCCESS,
        READ_ERROR,
        PERSE_ERROR,
    };


public:
    Result()
        : code_(Code::SUCCESS)
    {}
    
    Result(Code code)
        : code_(code)
    {}

public:
    bool isSucceed() const {
        return code_ == Code::SUCCESS;
    }
    
    bool isFailed() const {
        return code_ != Code::SUCCESS;
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

class Mesh;
typedef ::std::vector<uint16_t> Indices;
typedef ::std::vector<float> Vertices;
typedef ::std::vector<::std::shared_ptr<Mesh>> Meshes;

//  Colladaメッシュデータ
class Mesh final
{
public:
    class ArrayData
    {
    public:
        ArrayData()
            : stride_(0)
            , data_()
            , indices_()
        {}
    
        ~ArrayData()
        {}
    
    public:
        bool isValidate() const {
            return stride_ != 0;
        }
    
        void setStride(int8_t stride) {
            stride_ = stride;
        }
    
        void dump();

    public:
        int8_t stride_;
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
    Mesh& operator=(const Mesh&) = delete;	// コピーの禁止
    Mesh(const Mesh&) = delete;

    
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

    void dump();


public:
    ArrayData vertex_;
    ArrayData normal_;
    PrimitiveType primitive_type_;
};



//  パーサー
class Parser final
{
public:
    Parser();
    ~Parser();
    Parser& operator=(const Parser&) = delete;	// コピーの禁止
    Parser(const Parser&) = delete;
public:
    Result parse(const char* const dae_file_path);
    
    const Meshes* meshes() const;
    
private:
    class Impl;
    ::std::unique_ptr<Impl> impl_;

};






}   // namespace mcp

#endif // MCP_PARSER_HPP_INCLUDED
