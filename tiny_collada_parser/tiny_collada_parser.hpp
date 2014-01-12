//  mcp_parser.hpp

#ifndef TC_PARSER_HPP_INCLUDED
#define TC_PARSER_HPP_INCLUDED

// Include files.
#include <vector>
#include <cstdint>
#include <memory>



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

class ColladaMesh;
using Indices = ::std::vector<uint32_t>;
using Vertices = ::std::vector<float>;
using Meshes =  ::std::vector<::std::shared_ptr<ColladaMesh>>;

//  ColladaMaterial
class ColladaMaterial
{
public:
    void dump() {
        printf("ColladaMaterial:shading = %s\n", shading_name_);
        
        printf("  diffuse");
        for (int i = 0; i < diffuse_.size(); ++i) {
            printf(" %f", diffuse_[i]);
        }
        printf("\n");

        printf("  ambient");
        for (int i = 0; i < ambient_.size(); ++i) {
            printf(" %f", ambient_[i]);
        }
        printf("\n");


        printf("  emission");
        for (int i = 0; i < emission_.size(); ++i) {
            printf(" %f", emission_[i]);
        }
        printf("\n");


        printf("  specular");
        for (int i = 0; i < specular_.size(); ++i) {
            printf(" %f", specular_[i]);
        }
        printf("\n");

        printf("  reflective");
        for (int i = 0; i < reflective_.size(); ++i) {
            printf(" %f", reflective_[i]);
        }
        printf("\n");
        
        printf("  reflectivity %f\n", reflectivity_);
        printf("  shininess %f\n", shininess_);
        printf("  transparency %f\n", transparency_);
        printf("\n");
    }

    ColladaMaterial()
        : shading_name_(nullptr)
        , texture_name_(nullptr)
        , diffuse_()
        , ambient_()
        , emission_()
        , specular_()
        , reflective_()
        , shininess_(0.0f)
        , transparency_(0.0f)
        , reflectivity_(0.0f)
    {}


    const char* shading_name_;
    const char* texture_name_;
    std::vector<float> diffuse_;
    std::vector<float> ambient_;
    std::vector<float> emission_;
    std::vector<float> specular_;
    std::vector<float> reflective_;
    float shininess_;
    float transparency_;
    float reflectivity_;
};


//  Colladaメッシュデータ
class ColladaMesh final
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
        PRIMITIVE_TRIANGLES,        
        UNKNOWN_TYPE
    };
    
public:
    ColladaMesh()
        : vertex_()
        , normal_()
        , uv_()
        , primitive_type_(UNKNOWN_TYPE)
    {}
    ~ColladaMesh(){}
    ColladaMesh& operator=(const ColladaMesh&) = delete;	// コピーの禁止
    ColladaMesh(const ColladaMesh&) = delete;

    
public:
    //  法線を持っているか判定
    bool hasNormal() const {
        return normal_.isValidate();
    }
    
    //  頂点を持っているか判定
    bool hasVertex() const {
        return vertex_.isValidate();
    }

    //  テクスチャ座標を持っているか判定
    bool hasTexCoord() const {
        return uv_.isValidate();
    }

    void setPrimitiveType(
        const PrimitiveType type
    ) {
        primitive_type_ = type;
    }
    
    PrimitiveType getPrimitiveType() const {
        return primitive_type_;
    }
    
    const ArrayData* getVertex() const{
        return &vertex_;
    }
    
    const ArrayData* getNormals() const {
        return &normal_;
    }

    const ArrayData* getTexCoord() const {
        return &uv_;
    }

    void dump();


public:
    ArrayData vertex_;
    ArrayData normal_;
    ArrayData uv_;
    PrimitiveType primitive_type_;
    std::shared_ptr<ColladaMaterial> material_;
};
using ColladaMeshes = std::vector<std::shared_ptr<ColladaMesh>>;


//  シーン情報
class ColladaScene final
{
public:
    void dump(){
        if (material_) {
            material_->dump();
        }
    }


public:
    std::vector<float> matrix_;
    std::vector<std::shared_ptr<ColladaMesh>> meshes_;
    std::shared_ptr<ColladaMaterial> material_;
};
using ColladaScenes = std::vector<std::shared_ptr<ColladaScene>>;


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
    
//    const Meshes* meshes() const;
    const ColladaScenes* scenes() const;
private:
    class Impl;
    ::std::unique_ptr<Impl> impl_;

};






}   // namespace tc

#endif // MCP_PARSER_HPP_INCLUDED
