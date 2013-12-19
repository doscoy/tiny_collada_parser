//  mcp_parser.cpp

#include <cstring>
#include "tiny_collada_parser.hpp"
#include "third_party_libs/tinyxml2/tinyxml2.h"
#include <iostream>


namespace xml = tinyxml2;


namespace {


//----------------------------------------------------------------------
//  child elementを取得する
//  大文字小文字がバージョンによってバラバラな場合はここで吸収する
const xml::XMLElement* firstChildElement(
    const xml::XMLElement* parent,
    const char* const child_name
) {
    const xml::XMLElement* child = parent->FirstChildElement(child_name);
    return child;
}

//----------------------------------------------------------------------
//  attributeを取得する
//  大文字小文字がバージョンによってバラバラな場合はここで吸収する
const char* getElementAttribute(
    const xml::XMLElement* element,
    const char* const attri_name
) {
    return element->Attribute(attri_name);
}

//----------------------------------------------------------------------
//  ストライド幅を取得
uint32_t getStride(
    const xml::XMLElement* source_node
){
    const xml::XMLElement* technique_common = firstChildElement(
        source_node, 
        "technique_common"
    );
    const xml::XMLElement* accessor = firstChildElement(
        technique_common, 
        "accessor"
    );
    uint32_t stride;
    int check = accessor->QueryUnsignedAttribute("stride", &stride);
    if (check == xml::XML_NO_ATTRIBUTE) {
        stride = 1;
    }
        
    return stride;
}


//----------------------------------------------------------------------
//  配列データ読み込み
template <typename T>
void readArray(
    char* text,
    std::vector<T>* container
){
    char* value_str = std::strtok(text, " ");
    while (value_str) {
        T v = static_cast<T>(atof(value_str));
        container->push_back(v);
        value_str = std::strtok(nullptr, " ");
    }
}


//----------------------------------------------------------------------
//  ソース解析
void readSourceNode(
    const xml::XMLElement* const source_node,
    tc::SourceData* out
) {
        
    char array_types[2][15] = {
        "float_array",
        "int_array"
    };
        
    for (int i = 0; i < 2; ++i) {
        const xml::XMLElement* array_node = firstChildElement(
            source_node, 
            array_types[i]
        );
        if (!array_node) {
            continue;
        }
            
        out->stride_ = getStride(source_node);

        char* text = const_cast<char*>(array_node->GetText());
            
        readArray(text, &out->data_);
    }
}


//----------------------------------------------------------------------
//  インデックス読み込み
const xml::XMLElement* getIndexNode(
    const xml::XMLElement* mesh_node
) {
    //  どのデータ構造でインデックスをもっているか調査
    for (int prim_idx = 0; prim_idx < tc::Mesh::PRIMITIVE_TYPE_NUM; ++prim_idx) {
        const char primitive_types_name[tc::Mesh::PRIMITIVE_TYPE_NUM][15] = {
            "lines",
            "linestrips",
            "polygons",
            "polylist",
            "triangles",
            "trifans",
            "tristrips"
        };

        const xml::XMLElement* primitive_node = firstChildElement(
            mesh_node,
            primitive_types_name[prim_idx]
        );

        if (primitive_node) {
            return primitive_node;
        }
    }

    return nullptr;
}

//----------------------------------------------------------------------
//  インデックス読み込み
void collectIndices(
    const xml::XMLElement* mesh_node,
    tc::Indices& indices
) {        
    const xml::XMLElement* primitive_node = getIndexNode(mesh_node);

    //  インデックス値読み込み
    if (primitive_node) {
        char* text = const_cast<char*>(primitive_node->FirstChildElement("p")->GetText());
       readArray(text, &indices);
    }
}

//----------------------------------------------------------------------
tc::Mesh::PrimitiveType getMeshPrimitiveType(
    const xml::XMLElement* mesh_node
) {
    for (int prim_idx =0; prim_idx < tc::Mesh::PRIMITIVE_TYPE_NUM; ++prim_idx) {

        const char primitive_types_name[tc::Mesh::PRIMITIVE_TYPE_NUM][15] = {
            "lines",
            "linestrips",
            "polygons",
            "polylist",
            "triangles",
            "trifans",
            "tristrips"
        };
        
        const tc::Mesh::PrimitiveType primitive_types[tc::Mesh::PRIMITIVE_TYPE_NUM] = {
            tc::Mesh::LINE,
            tc::Mesh::LINE_STRIP,
            tc::Mesh::POLYGONS,
            tc::Mesh::POLYLIST,
            tc::Mesh::TRIANGLES,
            tc::Mesh::TRIANGLE_FAN,
            tc::Mesh::TRIANGLE_STRIP
        };
        
        const xml::XMLElement* primitive_node = firstChildElement(
            mesh_node,
            primitive_types_name[prim_idx]
        );
        
        if (!primitive_node) {
            //  知らないデータ構造なのでスキップ
            continue;
        }
        
        return primitive_types[prim_idx];
    }

    return tc::Mesh::UNKNOWN_TYPE;
}





//----------------------------------------------------------------------
//  メッシュノードのソース情報を取得
void collectMeshSources(
    std::vector<tc::SourceData>& out,
    const xml::XMLElement* mesh
){
    //  ソースノードを総なめして情報を保存
    const xml::XMLElement* target = firstChildElement(mesh, "source");
    while (target) {
        tc::SourceData data;
        //  ID保存
        data.id_ = getElementAttribute(target, "id");
        
        //  配列データ保存
        readSourceNode(target, &data);

        //  コンテナに追加
        out.push_back(data);
        
        //  次へ
        target = target->NextSiblingElement("source");
    }
}


//----------------------------------------------------------------------
//  インプット情報を取得
void collectMeshInputs(
    std::vector<tc::InputData>& out,
    const xml::XMLElement* mesh,
    const char* const target_name
){
    //  verticesノードのインプットとの２重連携の為に
    const xml::XMLElement* vertices =  firstChildElement(mesh, "vertices");

    //  inputノード
    const xml::XMLElement* target_node = firstChildElement(mesh, target_name);
    const xml::XMLElement* input_node = firstChildElement(target_node, "input");

    while (input_node) {
        tc::InputData input;
        //  sourceアトリビュートを取得
        const char* attr_source = getElementAttribute(input_node, "source");
        //  source_nameの先頭の#を取る
        if (attr_source[0] == '#') {
            attr_source = &attr_source[1];
        }

        //  verticesノードのinputで置き換えるべきものか判定
        if (vertices) {
            const char* vertices_id = getElementAttribute(vertices, "id");
            if (std::strncmp(attr_source, vertices_id, 64) == 0) {
                const xml::XMLElement* vert_input_node = firstChildElement(vertices, "input");
                //  verticesノードのinputで置き換える
                const char* vert_source = getElementAttribute(vert_input_node, "source");
                //  source_nameの先頭の#を取る
                if (vert_source[0] == '#') {
                    vert_source = &vert_source[1];
                }
                attr_source = vert_source;

                //  semanticも保存しておく
                input.semantic_ = getElementAttribute(vert_input_node, "semantic");
            }

        }
        input.source_ = attr_source;


        //  semanticアトリビュートを取得
        if (!input.semantic_) {
            //  verticesとのリレーションが既にはられてなければinputのsemanticを使用
            input.semantic_ = getElementAttribute(input_node, "semantic");
        }

        //  offsetアトリビュートを取得
        const char* attr_offset = getElementAttribute(input_node, "offset");
        int offset = 0;
        if (attr_offset) {
            offset = atoi(attr_offset);
        }
        input.offset_ = offset;
        

        out.push_back(input);
        //  次へ
        input_node = input_node->NextSiblingElement("input");
    }
}




}   // unname namespace


namespace tc {

//----------------------------------------------------------------------
Perser::Perser()
    : raw_indices_()
    , meshes_()
    , sources_()
    , inputs_()
{
}

//----------------------------------------------------------------------
Perser::~Perser()
{
}

//----------------------------------------------------------------------
Result Perser::perse(
    const char* const dae_path
) {
   
    //  tiny xmlを使って.daeを読み込む
    xml::XMLDocument doc;
    xml::XMLError load_error = doc.LoadFile(dae_path);
    if (load_error != xml::XML_SUCCESS) {
        return Result::READ_ERROR;
    }
    doc.Print();


    //  解析
    Result perse_result = perseCollada(&doc);
    if (perse_result.isFailed()) {
        return perse_result;
    }

    return Result::SUCCESS;
}

//----------------------------------------------------------------------
Result Perser::perseCollada(
    const xml::XMLDocument* const doc
) {

    //  メッシュ情報が入ってるのはgeometryノード
    const xml::XMLElement* root_node = doc->RootElement();
    const xml::XMLElement* library_geometries_node = firstChildElement(
        root_node, 
        "library_geometries"
    );
    const xml::XMLElement* geometry_node = firstChildElement(
        library_geometries_node, 
        "geometry"
    );

    while (geometry_node) {
        //  メッシュデータ解析
        Mesh data;
        const xml::XMLElement* mesh_node = firstChildElement(geometry_node, "mesh");

        perseMeshNode(mesh_node, &data);
        meshes_.push_back(data);
        
        //  次のジオメトリ
        geometry_node = geometry_node->NextSiblingElement("geometry");
    }

    return Result::SUCCESS;
}

//----------------------------------------------------------------------
//  メッシュノードの解析
void Perser::perseMeshNode(
    const xml::XMLElement* mesh_node,
    tc::Mesh* data
) {
    //  インデックス情報保存
    collectIndices(mesh_node, raw_indices_);

    //  ソースノードの情報保存
    collectMeshSources(sources_, mesh_node);

    //  インプットノードの情報保存
    collectMeshInputs(inputs_, mesh_node, "polylist");

    //  ソースとインプットを関連付け
    relateSourcesToInputs();
    
    setupMesh(data);
        
}

//----------------------------------------------------------------------
void Perser::setupMesh(
    tc::Mesh* mesh
) {
    SourceData* pos_source = searchSourceBySemantic("POSITION");
    if (pos_source) {
        mesh->vertex_.data_ = pos_source->data_;
    }

    SourceData* normal_source = searchSourceBySemantic("NORMAL");
    if (normal_source) {
        mesh->normal_.data_ = normal_source->data_;
    }

}

//----------------------------------------------------------------------
//  事前に抜いておいたインデックス一覧からインデックスのセットアップ
void Perser::setupIndices(
    Indices& out,
    int start_offset,
    int stride
) {
    for (int i = start_offset; i < raw_indices_.size(); i += stride) {
        out.push_back(raw_indices_.at(i));
    }
}



//----------------------------------------------------------------------
//  メッシュから抜いたinputsとsourcesを関連付ける
void Perser::relateSourcesToInputs()
{
    std::vector<SourceData>::iterator src_it = sources_.begin();
    std::vector<SourceData>::iterator src_end = sources_.end();

    for (; src_it != src_end; ++src_it) {
        src_it->input_ = searchInputBySource(src_it->id_);
    }
}


//----------------------------------------------------------------------
//  指定idのinputを探す
InputData* Perser::searchInputBySource(
    const char* const id
) {
    for (int i = 0; i < inputs_.size(); ++i) {
        InputData* input = &inputs_[i];
        if (std::strncmp(input->source_, id, 64) == 0){
            return input;
        }
    }
    return nullptr;
}

//----------------------------------------------------------------------
//  指定semanticのsourceを探す
SourceData* Perser::searchSourceBySemantic(
    const char* const semantic
) {
    for (int i = 0; i < sources_.size(); ++i) {
        SourceData* source = &sources_[i];
        InputData* input = source->input_;
        if (!input) {
            continue;
        }
        
        if (std::strncmp(input->semantic_, semantic, 64)) {
            return source;
        }
    }

    return nullptr;
}



}   // namespace tc

