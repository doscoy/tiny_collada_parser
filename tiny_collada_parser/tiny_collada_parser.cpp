//  mcp_parser.cpp


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
    const tinyxml2::XMLElement* root = doc->RootElement();
    const tinyxml2::XMLElement* library_geometries = firstChildElement(root, "library_geometries");
    const tinyxml2::XMLElement* geometry = firstChildElement(library_geometries, "geometry");

    while (geometry) {
        //  メッシュ情報を抜く
        Mesh data;
        //  メッシュのIDと名前を取得
        data.setID(getElementAttribute(geometry, "id"));
        data.setName(getElementAttribute(geometry, "name"));

        //  メッシュデータ解析
        const tinyxml2::XMLElement* mesh = firstChildElement(geometry, "mesh");
        perseMesh(mesh, &data);
        meshes_.push_back(data);
        
        //  次のジオメトリ
        geometry = geometry->NextSiblingElement("geometry");
    }
/*
      TiXmlElement* geometry = 
    doc.RootElement()->FirstChildElement("library_geometries")->FirstChildElement("geometry");

  // Iterate through geometry elements 
  while(geometry != NULL) {

    // Create new geometry
    ColGeom data;

    // Set the geometry name
    data.name = geometry->Attribute("id");

    // Iterate through mesh elements 
    mesh = geometry->FirstChildElement("mesh");
    while(mesh != NULL) {         
      vertices = mesh->FirstChildElement("vertices");
      input = vertices->FirstChildElement("input");
      
      // Iterate through input elements 
      while(input != NULL) {
        source_name = std::string(input->Attribute("source"));
        source_name = source_name.erase(0, 1);
        source = mesh->FirstChildElement("source");

        // Iterate through source elements 
        while(source != NULL) {
          if(std::string(source->Attribute("id")) == source_name) {
            data.map[std::string(input->Attribute("semantic"))] = readSource(source);
          }

          source = source->NextSiblingElement("source");
        } 

        input = input->NextSiblingElement("input");
      }

      // Determine primitive type
      for(int i=0; i<7; i++) {
        primitive = mesh->FirstChildElement(primitive_types[i]);
        if(primitive != NULL) {
          
          // Determine number of primitives
          primitive->QueryIntAttribute("count", &prim_count);

          // Determine primitive type and set count
          switch(i) {
            case 0:
              data.primitive = GL_LINES; 
              num_indices = prim_count * 2; 
            break;
            case 1: 
              data.primitive = GL_LINE_STRIP; 
              num_indices = prim_count + 1;
            break;
            case 4: 
              data.primitive = GL_TRIANGLES; 
              num_indices = prim_count * 3; 
            break;
            case 5: 
              data.primitive = GL_TRIANGLE_FAN; 
              num_indices = prim_count + 2; 
            break;
            case 6: 
              data.primitive = GL_TRIANGLE_STRIP; 
              num_indices = prim_count + 2; 
            break;
            default: std::cout << "Primitive " << primitive_types[i] << 
                     " not supported" << std::endl;
          }
          data.index_count = num_indices;

          // Allocate memory for indices
          data.indices = (unsigned short*)malloc(num_indices * sizeof(unsigned short));

          // Read the index values
          char* text = (char*)(primitive->FirstChildElement("p")->GetText());
          data.indices[0] = (unsigned short)atoi(strtok(text, " "));
          for(int index=1; index<num_indices; index++) {
            data.indices[index] = (unsigned short)atoi(strtok(NULL, " "));   
          }
        }
      }

      mesh = mesh->NextSiblingElement("mesh");
    }

    v->push_back(data);    

    geometry = geometry->NextSiblingElement("geometry");
*/

    return Result::SUCCESS;
}

//  メッシュノードの解析
void Perser::perseMesh(
    const tinyxml2::XMLElement* mesh,
    tc::Mesh* data
) {
    while (mesh) {
        const tinyxml2::XMLElement* vertices = firstChildElement(mesh, "vertices");
        const tinyxml2::XMLElement* input = firstChildElement(vertices, "input");

        while (input) {
            std::string source_name;
            source_name = std::string(getElementAttribute(input, "source"));
            source_name = source_name.erase(0, 1);
            const tinyxml2::XMLElement* source = mesh->FirstChildElement("source");

            while (source) {
                if (std::string(getElementAttribute(source, "id")) == source_name) {
                    data->map_[std::string(getElementAttribute(input, "semantic"))] = readSource(source);
                }

                source = source->NextSiblingElement("source");
            }

            input = input->NextSiblingElement("input");
        }


        // Determine primitive type
        for(int i=0; i<7; i++) {
            char primitive_types[7][15] = {
                "lines",
                "linestrips",
                "polygons",
                "polylist",
                "triangles",
                "trifans",
                "tristrips"
            };
            tinyxml2::XMLElement* primitive = firstChildElement(mesh, primitive_types[i]);
            if (primitive) {
          
                // Determine number of primitives
                int prim_count;
                int num_indices;
                primitive->QueryIntAttribute("count", &prim_count);

                // Determine primitive type and set count
                switch (i) {
                    case 0:
//                        data->primitive_ = GL_LINES;
                        num_indices = prim_count * 2;
                        break;
                    case 1:
//                        data->primitive_ = GL_LINE_STRIP;
                        num_indices = prim_count + 1;
                        break;
                    case 4:
//                        data->primitive_ = GL_TRIANGLES;
                        num_indices = prim_count * 3;
                        break;
                    case 5:
 //                       data->primitive_ = GL_TRIANGLE_FAN;
                        num_indices = prim_count + 2;
                        break;
                    case 6:
   //                     data->primitive_ = GL_TRIANGLE_STRIP;
                        num_indices = prim_count + 2;
                        break;
                    default:
                        printf("not supported.\n");
                }
                data->index_count = num_indices;

                // Allocate memory for indices
                data->indices = (unsigned short*)malloc(num_indices * sizeof(unsigned short));

                // Read the index values
                char* text = (char*)(primitive->FirstChildElement("p")->GetText());
                data->indices[0] = (unsigned short)atoi(strtok(text, " "));
                for (int index=1; index < num_indices; index++) {
                    data.indices[index] = (unsigned short)atoi(strtok(NULL, " "));
                }
            }
        }


        mesh = mesh->NextSiblingElement("mesh");
    }
}

SourceData Perser::readSource(
    const tinyxml2::XMLElement* const source
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
        const tinyxml2::XMLElement* array = firstChildElement(source, array_types[i]);
        if (!array) {
            continue;
        }
        
        // Find number of values
        uint32_t num_vals;
        array->QueryUnsignedAttribute("count", &num_vals);
        source_data.count_ = num_vals;

        // Find stride
        const tinyxml2::XMLElement* technique_common = firstChildElement(source, "technique_common");
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
        text = (char*)(array->GetText());

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