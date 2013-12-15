//  mcp_parser.cpp

#include <cstring>
#include "tiny_collada_parser.hpp"
#include "third_party_libs/tinyxml2/tinyxml2.h"
#include <iostream>

namespace {

//  child elementを取得する
//  大文字小文字がバージョンによってバラバラな場合はここで吸収する
const tinyxml2::XMLElement* firstChildElement(
    const tinyxml2::XMLElement* parent,
    const char* const child_name
) {
    const tinyxml2::XMLElement* child = parent->FirstChildElement(child_name);
    return child;
}


//  attributeを取得する
//  大文字小文字がバージョンによってバラバラな場合はここで吸収する
const char* getElementAttribute(
    const tinyxml2::XMLElement* element,
    const char* const attri_name
) {
    return element->Attribute(attri_name);
}

//----------------------------------------------------------------------
//  ストライド幅を取得
uint32_t getStride(
    const tinyxml2::XMLElement* source_node
){
    const tinyxml2::XMLElement* technique_common = firstChildElement(source_node, "technique_common");
    const tinyxml2::XMLElement* accessor = firstChildElement(technique_common, "accessor");
    uint32_t stride;
    int check = accessor->QueryUnsignedAttribute("stride", &stride);
    if (check == tinyxml2::XML_NO_ATTRIBUTE) {
        stride = 1;
    }
        
    return stride;
}


//----------------------------------------------------------------------
//  インデックス値読み込み
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
    const tinyxml2::XMLElement* const source_node,
    tc::Mesh::ArrayData* data
) {
        
    char array_types[2][15] = {
        "float_array",
        "int_array"
    };
        
    for (int i = 0; i < 2; ++i) {
        const tinyxml2::XMLElement* array_node = firstChildElement(source_node, array_types[i]);
        if (!array_node) {
            continue;
        }
            
        uint32_t stride = getStride(source_node);
        data->setStride(stride);
        // Read array values
        char* text = const_cast<char*>(array_node->GetText());
            
        readArray(text, &data->data_);
    }
}

//----------------------------------------------------------------------
//  インデックス読み込み
void readIndices(
    const tinyxml2::XMLElement* mesh_node,
    tc::Mesh* data
) {
    // Determine primitive type
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
            
        const tc::Mesh::PrimitiveType primitive_types[tc::Mesh::PRIMITIVE_TYPE_NUM] = {
            tc::Mesh::LINE,
            tc::Mesh::LINE_STRIP,
            tc::Mesh::POLYGONS,
            tc::Mesh::POLYLIST,
            tc::Mesh::TRIANGLES,
            tc::Mesh::TRIANGLE_FAN,
            tc::Mesh::TRIANGLE_STRIP
        };
            
        const tinyxml2::XMLElement* primitive_node = firstChildElement(
            mesh_node,
            primitive_types_name[prim_idx]
        );
            
        if (!primitive_node) {
            //  知らないデータ構造なのでスキップ
            continue;
        }
            
        //  プリミティブ種別設定
        data->setPrimitiveType(primitive_types[prim_idx]);
            
        //  インデックス値読み込み
        char* text = const_cast<char*>(primitive_node->FirstChildElement("p")->GetText());
        readArray(text, data->getIndices());
            
    }
        
}

tc::Mesh::PrimitiveType getMeshPrimitiveType(
    const tinyxml2::XMLElement* mesh_node
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
        
        const tinyxml2::XMLElement* primitive_node = firstChildElement(
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
void persePolylistMeshNode(
    const tinyxml2::XMLElement* mesh_node,
    tc::Mesh* data
) {
    printf("persePolylistMeshNode\n");

    const tinyxml2::XMLElement* polylist_node = firstChildElement(mesh_node, "polylist");
    const tinyxml2::XMLElement* input_node = firstChildElement(polylist_node, "input");
    const tinyxml2::XMLElement* source_node = mesh_node->FirstChildElement("source");

    int input_num = 0;
    while (input_node) {
        input_num += 1;
        
        //  sourceアトリビュートを取得
        const char* attr_source = getElementAttribute(input_node, "source");
        
        
        //  semanticアトリビュートを取得
        const char* attr_semantic = getElementAttribute(input_node, "semantic");
        
        
        //  offsetアトリビュートを取得
        const char* attr_offset = getElementAttribute(input_node, "offset");
        int offset = 0;
        if (attr_offset) {
            offset = atoi(attr_offset);
        }
        printf("offset %d\n", offset);
        
        
        
        printf("a\n");
        std::string input_source_name = std::string();
        
        //  source_nameの先頭の#を取る
        input_source_name = input_source_name.erase(0, 1);
        const tinyxml2::XMLElement* target = source_node;
        while (target) {
            std::string id(getElementAttribute(target, "id"));
            printf("%s == %s\n", id.c_str(), input_source_name.c_str());
            // 元のsourceと同じIDのものを探して解析
            if (id == input_source_name) {
                std::string semantic(getElementAttribute(input_node, "semantic"));
                
                if (semantic == std::string("VERTEX")) {
                    readSourceNode(target, data->getVertex());
                }
                else if (semantic == std::string("NORMAL")) {
                    readSourceNode(target, data->getNormals());
                }
                else {
                    printf("unknown semantic %s", semantic.c_str());
                }
            }
            
            target = target->NextSiblingElement("source");
        }
        
        input_node = input_node->NextSiblingElement("input");
    }
}

void perseStandardMeshNode(
    const tinyxml2::XMLElement* mesh_node,
    tc::Mesh* data
) {
    
    const tinyxml2::XMLElement* vertices_node = firstChildElement(mesh_node, "vertices");
    const tinyxml2::XMLElement* input_node = firstChildElement(vertices_node, "input");
    const tinyxml2::XMLElement* source_node = mesh_node->FirstChildElement("source");

    while (input_node) {
        std::string source_name;
        source_name = std::string(getElementAttribute(input_node, "source"));
            
        //  source_nameの先頭の # を取る
        source_name = source_name.erase(0, 1);
            
        while (source_node) {
            std::string id(getElementAttribute(source_node, "id"));
            printf("%s == %s\n", id.c_str(), source_name.c_str());
            // 元のsourceと同じIDのものを探して解析
            if (id == source_name) {
                std::string semantic(getElementAttribute(input_node, "semantic"));
                    
                if (semantic == std::string("POSITION")) {
                    readSourceNode(source_node, data->getVertex());
                }
                else if (semantic == std::string("NORMAL")) {
                    readSourceNode(source_node, data->getNormals());
                }
                else {
                    printf("unknown semantic %s", semantic.c_str());
                }
            }
                
            source_node = source_node->NextSiblingElement("source");
        }
            
        input_node = input_node->NextSiblingElement("input");
    }
        
}


}   // unname namespace


namespace tc {

//----------------------------------------------------------------------
Perser::Perser()
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
    //  XML読み込み
    tinyxml2::XMLDocument doc;
    Result load_result = loadDae(&doc, dae_path);
    if (load_result.isFailed()) {
        return load_result;
    }
    
    doc.Print();
    
    //  Collada情報の取得
    Result perse_result = perseDae(&doc);
    if (perse_result.isFailed()) {
        return perse_result;
    }

    return Result::SUCCESS;
}


//----------------------------------------------------------------------
Result Perser::loadDae(
    tinyxml2::XMLDocument* const doc,
    const char* const dae_path
) {
    //  tiny xmlを使って.daeを読み込む
    tinyxml2::XMLError load_error = doc->LoadFile(dae_path);
    if (load_error != tinyxml2::XML_SUCCESS) {
        return Result::READ_ERROR;
    }

    //  成功
    return Result::SUCCESS;
}


//----------------------------------------------------------------------
Result Perser::perseDae(
    const tinyxml2::XMLDocument *const doc
) {

    //  メッシュ情報が入ってるのはgeometryノード
    const tinyxml2::XMLElement* root_node = doc->RootElement();
    const tinyxml2::XMLElement* library_geometries_node = firstChildElement(root_node, "library_geometries");
    const tinyxml2::XMLElement* geometry_node = firstChildElement(library_geometries_node, "geometry");

    while (geometry_node) {
        //  メッシュ情報を抜く
        Mesh data;
        //  メッシュのIDと名前を取得
        const char* geometry_node_id = getElementAttribute(geometry_node, "id");
        if (geometry_node_id) {
            data.setID(geometry_node_id);
        }
        const char* geometry_node_name = getElementAttribute(geometry_node, "name");
        if (geometry_node_name) {
            data.setName(geometry_node_name);
        }

        //  メッシュデータ解析
        const tinyxml2::XMLElement* mesh_node = firstChildElement(geometry_node, "mesh");
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
    const tinyxml2::XMLElement* mesh_node,
    tc::Mesh* data
) {
    while (mesh_node) {
    
        //  プリミティブのタイプを取得
        tc::Mesh::PrimitiveType primitive_type = getMeshPrimitiveType(mesh_node);
    
        if (primitive_type == Mesh::UNKNOWN_TYPE) {
            //  知らないタイプなのでスキップ
            mesh_node = mesh_node->NextSiblingElement("mesh");
            continue;
        }
        
        if (primitive_type == Mesh::POLYLIST) {
            persePolylistMeshNode(
                mesh_node,
                data
            );
        }
        else {
            perseStandardMeshNode(
                mesh_node,
                data
            );
        }
    

        readIndices(mesh_node, data);

        mesh_node = mesh_node->NextSiblingElement("mesh");
    }
}





}   // namespace tc

