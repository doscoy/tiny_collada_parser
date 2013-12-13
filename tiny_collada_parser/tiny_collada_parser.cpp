//  mcp_parser.cpp

#include <cstring>
#include "tiny_collada_parser.hpp"
#include "third_party_libs/tinyxml2/tinyxml2.h"


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





}   // unname namespace


namespace tc {


Mesh::Mesh()
    : stride_vertex_(TINY_COLLADA_MESH_INVARIDATE_STRIDE)
    , stride_normal_(TINY_COLLADA_MESH_INVARIDATE_STRIDE)
{
}


Mesh::~Mesh()
{
}



Perser::Perser()
{
}

Perser::~Perser()
{
}

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
        data.setID(getElementAttribute(geometry_node, "id"));
        data.setName(getElementAttribute(geometry_node, "name"));

        //  メッシュデータ解析
        const tinyxml2::XMLElement* mesh_node = firstChildElement(geometry_node, "mesh");
        perseMeshNode(mesh_node, &data);
        meshes_.push_back(data);
        
        //  次のジオメトリ
        geometry_node = geometry_node->NextSiblingElement("geometry");
    }

    return Result::SUCCESS;
}

//  メッシュノードの解析
void Perser::perseMeshNode(
    const tinyxml2::XMLElement* mesh_node,
    tc::Mesh* data
) {
    while (mesh_node) {
        const tinyxml2::XMLElement* vertices_node = firstChildElement(mesh_node, "vertices");
        const tinyxml2::XMLElement* input_node = firstChildElement(vertices_node, "input");

        while (input_node) {
            std::string source_name;
            source_name = std::string(getElementAttribute(input_node, "source"));
            source_name = source_name.erase(0, 1);
            const tinyxml2::XMLElement* source_node = mesh_node->FirstChildElement("source");

            while (source_node) {
                if (std::string(getElementAttribute(source_node, "id")) == source_name) {
                    data->map_[std::string(getElementAttribute(input_node, "semantic"))] = readSourceNode(source_node);
                }

                source_node = source_node->NextSiblingElement("source");
            }

            input_node = input_node->NextSiblingElement("input");
        }


        // Determine primitive type
        for(int prim_idx =0; prim_idx < 7; ++prim_idx) {
            const char primitive_types_name[7][15] = {
                "lines",
                "linestrips",
                "polygons",
                "polylist",
                "triangles",
                "trifans",
                "tristrips"
            };
            
            const Mesh::PrimitiveType primitive_types[7] = {
                Mesh::LINE,
                Mesh::LINE_STRIP,
                Mesh::POLYGONS,
                Mesh::POLYLIST,
                Mesh::TRIANGLES,
                Mesh::TRIANGLE_FAN,
                Mesh::TRIANGLE_STRIP
            };
            
            const tinyxml2::XMLElement* primitive_node = firstChildElement(
                mesh_node,
                primitive_types_name[prim_idx]
            );
            
            if (!primitive_node) {
                //  違うデータ構造なのでスキップ
                continue;
            }
            
            //  プリミティブ種別設定
            data->primitive_type_ = primitive_types[prim_idx];
                
            //  インデックス値読み込み
            char* text = const_cast<char*>(primitive_node->FirstChildElement("p")->GetText());
            readIndices(text, &data->indices_);
            
        }


        mesh_node = mesh_node->NextSiblingElement("mesh");
    }
}

//----------------------------------------------------------------------
//  インデックス値読み込み
void Perser::readIndices(
    char* index_text,
    Indices* container
){
    char* idx_char = std::strtok(index_text, " ");
    while (idx_char) {
        uint16_t index = static_cast<uint16_t>(atoi(idx_char));
        container->push_back(index);
        idx_char = std::strtok(nullptr, " ");
    }
}

//----------------------------------------------------------------------
//  ソース解析
SourceData Perser::readSourceNode(
    const tinyxml2::XMLElement* const source_node
) {
  
    char array_types[7][15] = {
        "float_array",
        "int_array",
        "bool_array",
        "Name_array",
        "IDREF_array",
        "SIDREF_array",
        "token_array"
    };


  
    SourceData source_data;
    char* text;

    for (int i=0; i < 7; i++) {
        const tinyxml2::XMLElement* array_node = firstChildElement(source_node, array_types[i]);
        if (!array_node) {
            continue;
        }
        
        // Find number of values
        uint32_t num_vals;
        array_node->QueryUnsignedAttribute("count", &num_vals);
        source_data.count_ = num_vals;

        // Find stride
        const tinyxml2::XMLElement* technique_common = firstChildElement(source_node, "technique_common");
        const tinyxml2::XMLElement* accessor = firstChildElement(technique_common, "accessor");
        uint32_t stride;
        int check = accessor->QueryUnsignedAttribute("stride", &stride);
        if (check != tinyxml2::XML_NO_ATTRIBUTE) {
            source_data.stride_ = stride;
        }
        else {
            source_data.stride_ = 1;
        }
            
        // Read array values
        text = (char*)(array_node->GetText());

        // Initialize mesh data according to data type
        switch (i) {

            // Array of floats
            case 0:
//                source_data.type = GL_FLOAT;
                source_data.size_ = source_data.count_ * sizeof(float);
                source_data.data_ = malloc(num_vals * sizeof(float));

                // Read the float values
                ((float*)source_data.data_)[0] = atof(strtok(text, " "));
                for(unsigned int index=1; index<num_vals; index++) {
                    ((float*)source_data.data_)[index] = atof(strtok(NULL, " "));
                }
                break;

            // Array of integers
            case 1:
//                source_data.type = GL_INT;
                source_data.size_ = source_data.count_ * sizeof(int);
                source_data.data_ = malloc(num_vals * sizeof(int));

                // Read the int values
                ((int*)source_data.data_)[0] = atof(strtok(text, " "));
                for(unsigned int index=1; index<num_vals; index++) {
                    ((int*)source_data.data_)[index] = atof(strtok(NULL, " "));
                }
                break;

            // Other
            default:
                printf("Collada Reader doesn't support mesh data in this format");
                break;
        }
    }
    return source_data;
}




}   // namespace tc