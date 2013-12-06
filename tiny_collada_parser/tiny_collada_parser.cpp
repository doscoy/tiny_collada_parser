//  mcp_parser.cpp


#include "tiny_collada_parser.hpp"
#include "third_party_libs/tinyxml2/tinyxml2.h"


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
    
    //  Collada情報の取得
    result perse_result = perseDae(&doc);
    if (perse_result.isFailed()) {
        return perse_result;
    }

    
    
    
    
    

    return Result(Result::SUCCESS);
}



Result Perser::loadDae(
    tinyxml2::XMLDocument* const doc,
    const char* const dae_path
) {
    //  tiny xmlを使って.daeを読み込む
    tinyxml2::XMLError load_error = doc->LoadFile(dae_path);
    if (load_error != tinyxml2::XML_SUCCESS) {
        return tc::Result(tc::Result::READ_ERROR);
    }

    //  成功
    return tc::Result(tc::Result::SUCCESS);
}


Result Perser::perseDae(
    const tinyxml2::XMLDocument *const doc
) {

    //  メッシュ情報が入ってるのはgeometryノード
    tinyxml2::XMLElement* root = doc->RootElement();
    tinyxml2::XMLElement* library_geometries = root->FirstChildElement("library_geometries");
    tinyxml2::XMLElement* geometry = library_geometries->FirstChildElement("geometry");

    while (geometry != nullptr) {
        ;
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
}


}   // namespace tc