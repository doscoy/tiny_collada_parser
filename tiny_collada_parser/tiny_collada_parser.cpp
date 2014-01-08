//  mcp_parser.cpp

#include "tiny_collada_parser.hpp"
#include "third_party_libs/tinyxml2/tinyxml2.h"
#include <cassert>

#if 1
    #define TINY_COLLADA_DEBUG  1
#else
    #define TYNE_COLLADA_DEBUG  0
#endif


#if TINY_COLLADA_DEBUG
    #define TINY_COLLADA_TRACE(...)         ::std::printf(__VA_ARGS__)
    #define TINY_COLLADA_ASSERT(exp)        assert(exp)
#else

    #define TINY_COLLADA_TRACE(...)         (void)0
    #define TINY_COLLADA_ASSERT(exp)        (void)0
#endif


namespace xml = tinyxml2;




namespace {

const char* INPUT_NODE_NAME = "input";
const char* SOURCE_NODE_NAME = "source";
const char* SEMANTIC_ATTR_NAME = "semantic";
const char* MESH_NODE_NAME = "mesh";
const char* GEOMETRY_NODE_NAME = "geometry";
const char* INDEX_NODE_NAME = "p";
const char* STRIDE_ATTR_NAME = "stride";
const char* ACCESSOR_NODE_NAME = "accessor";
const char* TECH_COMMON_NODE_NAME = "technique_common";
const char* LIB_GEOMETRY_NODE_NAME = "library_geometries";
const char* ID_ATTR_NAME = "id";
const char* OFFSET_ATTR_NAME = "offset";
const char* VERTICES_NODE_NAME = "vertices";
const char* COUNT_ATTR_NAME = "count";


//======================================================================
struct PrimitiveSelector
{
    const char* name_;
    tc::ColladaMesh::PrimitiveType type_;
};

#define PRIMITIVE_TYPE_NUM 2
const PrimitiveSelector PRIMITIVE_TYPE_SELECT[PRIMITIVE_TYPE_NUM] = {
    {"triangles", tc::ColladaMesh::PRIMITIVE_TRIANGLES},
    {"polylist", tc::ColladaMesh::PRIMITIVE_TRIANGLES}
};


const size_t STRING_COMP_SIZE = 64;



//======================================================================
//  visual_sceneノードデータ
struct VisualSceneData
{
    VisualSceneData()
        : type_(TYPE_UNKNOWN)
        , url_(nullptr)
        , matrix_()
        , bind_material_(nullptr)
    {}


    void dump() {
        TINY_COLLADA_TRACE("VisualSceneNode:type = %d ", type_);
        if (url_) {
            TINY_COLLADA_TRACE("url = %s ", url_);
        }
        if (bind_material_) {
            TINY_COLLADA_TRACE("bind_material = %s", bind_material_);
        }
        TINY_COLLADA_TRACE(" matrix (%d)", matrix_.size());
        for (int i = 0; i < matrix_.size(); ++i) {
            if ((i % 4) == 0) {
                TINY_COLLADA_TRACE("\n");
            }
            TINY_COLLADA_TRACE(" %f", matrix_[i]);
        }
        TINY_COLLADA_TRACE("\n");
    }

    enum Type {
        TYPE_GEOMETRY,
        TYPE_UNKNOWN,
    };
    Type type_;
    const char* url_;
    std::vector<float> matrix_;
    const char* bind_material_;
};
using VisualScenes = std::vector<std::shared_ptr<VisualSceneData>>;


//======================================================================
//  material
struct MaterialData
{
    const char* id_;
    const char* url_;
    
    void dump() {
        TINY_COLLADA_TRACE("MaterialData:id %s  url %s\n", id_, url_);
    }
};
using Materials = std::vector<std::shared_ptr<MaterialData>>;


//======================================================================
//  material effect
struct EffectData
{
    void dump() {
        TINY_COLLADA_TRACE("EffectData: id = %s\n", id_);
        material_->dump();
    }

    EffectData()
        : id_(nullptr)
        , material_(nullptr)
    {}


    const char* id_;
    std::shared_ptr<tc::ColladaMaterial> material_;
};
using Effects = std::vector<std::shared_ptr<EffectData>>;


//======================================================================
//  textures
struct ImageData
{
    void dump() {
        TINY_COLLADA_TRACE("ImageData: id = %s  init_from = %s\n", id_, init_from_);
    }

    ImageData()
        : id_(nullptr)
        , init_from_(nullptr)
    {}

    const char* id_;
    const char* init_from_;
};
using Images = std::vector<std::shared_ptr<ImageData>>;


//======================================================================
//  インプットデータ
struct InputData
{
    InputData()
        : semantic_(nullptr)
        , source_(nullptr)
        , offset_(0)
    {}
    
    void dump() {
        TINY_COLLADA_TRACE("InputData:semantic = ");
        if (semantic_) {
            printf(semantic_);
        }
        TINY_COLLADA_TRACE("  source = ");
        if (source_) {
            printf(source_);
        }
        TINY_COLLADA_TRACE("  offset = %d\n", offset_);
    }
    
    const char* semantic_;
    const char* source_;
    int offset_;
};
    
//======================================================================
//  ソースデータ
struct SourceData
{
    SourceData()
        : id_(nullptr)
        , stride_(0)
        , data_()
        , input_(nullptr)
    {}
    
    void dump() {
        TINY_COLLADA_TRACE("SourceData:id = %s  stride = %d\n", id_, stride_);
    }
    
    const char* id_;
    uint32_t stride_;
    std::vector<float> data_;
    InputData* input_;
};



//======================================================================
//  メッシュ情報
struct MeshInformation {

//----------------------------------------------------------------------
//  指定idのinputを探す
InputData* searchInputBySource(
    const char* const id
) {
    for (int i = 0; i < inputs_.size(); ++i) {
        InputData* input = &inputs_[i];
        TINY_COLLADA_TRACE("search %s == %s\n", id, input->source_);
        if (std::strncmp(input->source_, id, STRING_COMP_SIZE) == 0){
            return input;
        }
    }
    return nullptr;
}


//----------------------------------------------------------------------
//  指定semanticのsourceを探す
SourceData* searchSourceBySemantic(
    const char* const semantic
) {
    size_t sources_size = sources_.size();
    TINY_COLLADA_TRACE("\nsearchSourceBySemantic %s %d\n", semantic, sources_size);
    for (int i = 0; i < sources_size; ++i) {
        SourceData* source = &sources_[i];
        InputData* input = source->input_;
        if (!input) {
            TINY_COLLADA_TRACE("no input - %s\n", source->id_);
            continue;
        }
        TINY_COLLADA_TRACE("  %s %s\n", input->semantic_, input->source_);   
        if (std::strncmp(input->semantic_, semantic, STRING_COMP_SIZE) == 0) {
            TINY_COLLADA_TRACE("%s - %s [%s] FOUND.\n", __FUNCTION__, semantic, input->source_);
            return source;
        }
    }
    TINY_COLLADA_TRACE("%s - %s NOT FOUND.\n", __FUNCTION__, semantic);
        
    return nullptr;
}

//----------------------------------------------------------------------
//  inputのインデックスオフセット幅を取得
int getIndexStride() const
{
    int max_offset = 0;
    for (int i = 0; i < inputs_.size(); ++i) {
        int offset = inputs_[i].offset_;
        if (offset > max_offset) {
            max_offset = offset;
        }
    }

    return max_offset + 1;
}

//----------------------------------------------------------------------
//  データ表示
void dump()
{
    
    for (int i = 0; i < sources_.size(); ++i) {
        sources_[i].dump();
    }

    for (int i = 0; i < inputs_.size(); ++i) {
        inputs_[i].dump();
    }

}

tc::Indices raw_indices_;
std::vector<char> face_count_;
std::vector<SourceData> sources_;
std::vector<InputData> inputs_;
    
};



//----------------------------------------------------------------------
//  child elementを取得する
//  大文字小文字がバージョンによってバラバラな場合はここで吸収する
const xml::XMLElement* firstChildElement(
    const xml::XMLElement* parent,
    const char* const child_name
) {
    TINY_COLLADA_ASSERT(parent);
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
    TINY_COLLADA_ASSERT(element);
    return element->Attribute(attri_name);
}

//----------------------------------------------------------------------
//  ストライド幅を取得
uint32_t getStride(
    const xml::XMLElement* source_node
){
    const xml::XMLElement* technique_common = firstChildElement(
        source_node, 
        TECH_COMMON_NODE_NAME
    );
    const xml::XMLElement* accessor = firstChildElement(
        technique_common, 
        ACCESSOR_NODE_NAME
    );
    uint32_t stride;
    int check = accessor->QueryUnsignedAttribute(STRIDE_ATTR_NAME, &stride);
    if (check == xml::XML_NO_ATTRIBUTE) {
        stride = 1;
    }
        
    return stride;
}


//----------------------------------------------------------------------
//  配列データ読み込み
template <typename T>
void readArray(
    const char* text,
    std::vector<T>* container
){
    char* value_str = std::strtok(const_cast<char*>(text), " ");
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
    SourceData* out
) {
    const int ARRAY_TYPE_MAX = 2;
    char array_types[ARRAY_TYPE_MAX][15] = {
        "float_array",
        "int_array"
    };
        
    for (int i = 0; i < ARRAY_TYPE_MAX; ++i) {
        const xml::XMLElement* array_node = firstChildElement(
            source_node, 
            array_types[i]
        );
        if (!array_node) {
            continue;
        }
        //  データのストライドを取得
        out->stride_ = getStride(source_node);

        const char* text = array_node->GetText();
        //  データ数分のメモリをあらかじめリザーブ
        size_t data_count = 0;
        const char* count_str = getElementAttribute(array_node, COUNT_ATTR_NAME);
        if (count_str) {
            data_count = std::atoi(count_str);
        }
        TINY_COLLADA_TRACE("reserve %d\n", data_count);
        out->data_.reserve(data_count);

        //  データ取得
        readArray(text, &out->data_);
    }
}


//----------------------------------------------------------------------
//  プリミティブノード取得
const xml::XMLElement* getPrimitiveNode(
    const xml::XMLElement* mesh_node
) {
    //  どのデータ構造でインデックスをもっているか調査
    for (int prim_idx = 0; prim_idx < PRIMITIVE_TYPE_NUM; ++prim_idx) {

        //  読み込めたら存在している
        const xml::XMLElement* primitive_node = firstChildElement(
            mesh_node,
            PRIMITIVE_TYPE_SELECT[prim_idx].name_
        );

        if (primitive_node) {
            return primitive_node;
        }
    }

    return nullptr;
}

//----------------------------------------------------------------------
//  プリミティブタイプ取得
tc::ColladaMesh::PrimitiveType getPrimitiveType(
    const xml::XMLElement* mesh_node
) {
    //  どのデータ構造でインデックスをもっているか調査
    for (int prim_idx = 0; prim_idx < PRIMITIVE_TYPE_NUM; ++prim_idx) {

        //  読み込めたら存在している
        const xml::XMLElement* primitive_node = firstChildElement(
            mesh_node,
            PRIMITIVE_TYPE_SELECT[prim_idx].name_
        );

        if (primitive_node) {
            return PRIMITIVE_TYPE_SELECT[prim_idx].type_;
        }
    }

    return tc::ColladaMesh::UNKNOWN_TYPE;
}


//----------------------------------------------------------------------
//  visual scene読み込み
void collectVisualSceneNode(
    VisualScenes& out,
    const xml::XMLElement* visual_scene_root
) {
    const xml::XMLElement* visual_scene = firstChildElement(visual_scene_root, "visual_scene");

    while (visual_scene) {
        const xml::XMLElement* visual_scene_node = firstChildElement(visual_scene, "node");
        while (visual_scene_node) {
            std::shared_ptr<VisualSceneData> vs = std::make_shared<VisualSceneData>();
            
            //  行列取得
            const xml::XMLElement* matrix_node = firstChildElement(visual_scene_node, "matrix");
            const char* mtx_text = matrix_node->GetText();
            readArray(mtx_text, &vs->matrix_);

            //  タイプ判定
            const xml::XMLElement* instance_geometry = firstChildElement(visual_scene_node, "instance_geometry");
            if (instance_geometry) {
                // ジオメトリノードだった
                vs->type_ = VisualSceneData::TYPE_GEOMETRY;
                const char* attr_url = getElementAttribute(instance_geometry, "url");
                if (attr_url[0] == '#') {
                    attr_url = &attr_url[1];
                }
                vs->url_ = attr_url;

            
                //  マテリアル取得
                const xml::XMLElement* bind_material = firstChildElement(
                    instance_geometry,
                    "bind_material"
                );
                if (bind_material) {
                    //  マテリアル設定があった
                    const xml::XMLElement* technique_common = firstChildElement(
                        bind_material,
                        "technique_common"
                    );
                    const xml::XMLElement* instance_material = firstChildElement(
                        technique_common,
                        "instance_material"
                    );
                    const char* attr_target = getElementAttribute(
                        instance_material,
                        "target"
                    );
                    if (attr_target[0] == '#') {
                        attr_target = &attr_target[1];
                    }
                    vs->bind_material_ = attr_target;
                }
            
            }
            
            out.push_back(vs);
            visual_scene_node = visual_scene_node->NextSiblingElement("node");
        }
        visual_scene = visual_scene->NextSiblingElement("visual_scene");
    }
}


//----------------------------------------------------------------------
//  マテリアルノード読み込み
void collectMaterialNode(
    Materials& out,
    const xml::XMLElement* library_materials
) {
    const xml::XMLElement* material = firstChildElement(library_materials, "material");

    while (material) {
        const xml::XMLElement* instance_effect = firstChildElement(material, "instance_effect");
        std::shared_ptr<MaterialData> md = std::make_shared<MaterialData>();
        md->id_ = getElementAttribute(material, "id");
        while (instance_effect) {
            const char* attr_url = getElementAttribute(instance_effect, "url");
            if (attr_url[0] == '#') {
                attr_url = &attr_url[1];
            }
            md->url_ = attr_url;
            instance_effect = instance_effect->NextSiblingElement("instance_effect");
        }
        
        out.push_back(md);
        material = material->NextSiblingElement("material");
    }
}


//----------------------------------------------------------------------
//  phongノード読み込み
void parseEffect(
    std::shared_ptr<EffectData>& ef,
    const xml::XMLElement* shading
) {
    //  エミッション
    const xml::XMLElement* emission = firstChildElement(shading, "emission");
    if (emission) {
        const xml::XMLElement* color = firstChildElement(emission, "color");
        if (color) {
            const char* text = color->GetText();
            readArray(text, &ef->material_->emission_);
        }
    }
    //  アンビエント
    const xml::XMLElement* ambient = firstChildElement(shading, "ambient");
    if (ambient) {
        const xml::XMLElement* color = firstChildElement(ambient, "color");
        if (color) {
            const char* text = color->GetText();
            readArray(text, &ef->material_->ambient_);
        }
    }

    //  ディフューズ
    const xml::XMLElement* diffuse = firstChildElement(shading, "diffuse");
    if (diffuse) {
        const xml::XMLElement* color = firstChildElement(diffuse, "color");
        if (color) {
            const char* text = color->GetText();
            readArray(text, &ef->material_->diffuse_);
        }
    }

    //  スペキュラ
    const xml::XMLElement* specular = firstChildElement(shading, "specular");
    if (specular) {
        const xml::XMLElement* color = firstChildElement(specular, "color");
        if (color) {
            const char* text = color->GetText();
            readArray(text, &ef->material_->specular_);
        }
    }

    //  リフレクティブ
    const xml::XMLElement* reflective = firstChildElement(shading, "reflective");
    if (reflective) {
        const xml::XMLElement* color = firstChildElement(reflective, "color");
        if (color) {
            const char* text = color->GetText();
            readArray(text, &ef->material_->reflective_);
        }
    }
    
    
    //  リフレクティビティ
    const xml::XMLElement* reflectivity = firstChildElement(shading, "reflectivity");
    if (reflectivity) {
        const xml::XMLElement* data = firstChildElement(reflectivity, "float");
        const char* val = data->GetText();

        ef->material_->reflectivity_ = atof(val);
    }

    //  シャイネス
    const xml::XMLElement* shininess = firstChildElement(shading, "shininess");
    if (shininess) {
        const xml::XMLElement* data = firstChildElement(shininess, "float");
        const char* val = data->GetText();

        ef->material_->shininess_ = atof(val);
    }
    
    //  透明度
    const xml::XMLElement* transparency = firstChildElement(shading, "transparency");
    if (transparency) {
        const xml::XMLElement* data = firstChildElement(transparency, "float");
        const char* val = data->GetText();

        ef->material_->transparency_ = atof(val);
    }

}


//----------------------------------------------------------------------
//  エフェクトノード読み込み
void collectEffectNode(
    Effects& out,
    const xml::XMLElement* library_effects
) {
    const char* SHADING_NAME[3] = {
        "blinn",
        "phong",
        "----"
    };
    const xml::XMLElement* effect = firstChildElement(library_effects, "effect");

    while (effect) {
        const xml::XMLElement* profile_common = firstChildElement(effect, "profile_COMMON");
        const xml::XMLElement* technique = firstChildElement(profile_common, "technique");

        std::shared_ptr<EffectData> ed = std::make_shared<EffectData>();
        ed->material_ = std::make_shared<tc::ColladaMaterial>();
        ed->id_ = getElementAttribute(effect, "id");

        for (int shade_idx = 0; shade_idx < 3; ++shade_idx) {
            const xml::XMLElement* shading = firstChildElement(technique, SHADING_NAME[shade_idx]);
            if (shading) {
                //  シェーディング
                ed->material_->shading_name_ = SHADING_NAME[shade_idx];
                parseEffect(ed, shading);
                break;
            }
        }
        
        out.push_back(ed);
        effect = effect->NextSiblingElement("effect");
    }

}


//----------------------------------------------------------------------
//  画像パス読み込み
void collectImageNode(
    Images& out,
    const xml::XMLElement* library_images
) {
    const xml::XMLElement* image = firstChildElement(library_images, "image");

    while (image) {
        //  ID取得
        std::shared_ptr<ImageData> image_data = std::make_shared<ImageData>();
        image_data->id_ = getElementAttribute(image, "id");


        //  テクスチャパス取得
        const xml::XMLElement* init_from = firstChildElement(image, "init_from");
        image_data->init_from_ = init_from->GetText();
        

        //  次へ
        out.push_back(image_data);
        image = image->NextSiblingElement("image");
    }

}


//----------------------------------------------------------------------
//  インデックス読み込み
void collectIndices(
    const xml::XMLElement* mesh_node,
    tc::Indices& indices
) {        
    const xml::XMLElement* primitive_node = getPrimitiveNode(mesh_node);

    //  インデックス値読み込み
    if (primitive_node) {
        const xml::XMLElement* index_node = firstChildElement(primitive_node, INDEX_NODE_NAME);
        const char* text = index_node->GetText();
        readArray(text, &indices);
    }
    
    TINY_COLLADA_TRACE("Collect index size = %lu\n", indices.size());
}

//----------------------------------------------------------------------
//  面の頂点数読み込み
void collectFaceCount(
    const xml::XMLElement* mesh_node,
    std::vector<char>& face_count
) {        
    const xml::XMLElement* primitive_node = getPrimitiveNode(mesh_node);

    //  インデックス値読み込み
    if (primitive_node) {
        const xml::XMLElement* vcount_node = primitive_node->FirstChildElement("vcount");
        if (vcount_node) {
            char* text = const_cast<char*>(vcount_node->GetText());
            readArray(text, &face_count);
        }
    }
    
    TINY_COLLADA_TRACE("Collect face count size = %lu\n", face_count.size());
}



//----------------------------------------------------------------------
//  メッシュノードのソース情報を取得
void collectMeshSources(
    std::vector<SourceData>& out,
    const xml::XMLElement* mesh
){
    //  ソースノードを総なめして情報を保存
    const xml::XMLElement* target = firstChildElement(mesh, SOURCE_NODE_NAME);
    while (target) {
        SourceData data;
        //  ID保存
        data.id_ = getElementAttribute(target, ID_ATTR_NAME);
        
        //  配列データ保存
        readSourceNode(target, &data);

        //  コンテナに追加
        out.push_back(data);
        
        //  次へ
        target = target->NextSiblingElement(SOURCE_NODE_NAME);
    }
}

//----------------------------------------------------------------------
//  インプット情報を取得
void collectInputNodeData(
    std::vector<InputData>& out,
    const xml::XMLElement* input_node
) {
    while (input_node) {
        InputData input;
        //  sourceアトリビュートを取得
        const char* attr_source = getElementAttribute(input_node, SOURCE_NODE_NAME);
        //  source_nameの先頭の#を取る
        if (attr_source[0] == '#') {
            attr_source = &attr_source[1];
        }
        input.source_ = attr_source;
        input.semantic_ = getElementAttribute(input_node, SEMANTIC_ATTR_NAME);
        
        //  offsetアトリビュートを取得
        const char* attr_offset = getElementAttribute(input_node, OFFSET_ATTR_NAME);
        int offset = 0;
        if (attr_offset) {
            offset = atoi(attr_offset);
        }
        input.offset_ = offset;


        //  インプットノード保存
        out.push_back(input);

        //  次へ
        input_node = input_node->NextSiblingElement(INPUT_NODE_NAME);
    }
 
}

//----------------------------------------------------------------------
//  インプット情報を取得
void collectMeshInputs(
    std::vector<InputData>& out,
    const xml::XMLElement* mesh
){
    //  primitiveノード
    const xml::XMLElement* primitive_node = getPrimitiveNode(mesh);
	if (!primitive_node) {
		return;
	}
    const xml::XMLElement* prim_input_node = firstChildElement(primitive_node, INPUT_NODE_NAME);
    collectInputNodeData(out, prim_input_node);
    
    //  vertices_node
    const xml::XMLElement* vertices =  firstChildElement(mesh, VERTICES_NODE_NAME);
    const xml::XMLElement* vert_input_node = firstChildElement(vertices, INPUT_NODE_NAME);
    collectInputNodeData(out, vert_input_node);
}

std::shared_ptr<tc::ColladaMaterial> searchMaterial(
    const char* bind_material,
    const Materials& materials,
    const Effects& effects
) {
    //  マテリアルを探す
    const char* effect_url = nullptr;
    for (int i = 0; i < materials.size(); ++i) {
        if (std::strncmp(materials[i]->id_, bind_material, 64) == 0) {
            //  あった
            effect_url = materials[i]->url_;
            break;
        }
    }

    if (!effect_url) {
        //  指定マテリアルは存在しなかった
        return nullptr;
    }

    //  エフェクトを探す
    std::shared_ptr<tc::ColladaMaterial> mat = nullptr;
    for (int i = 0; i < effects.size(); ++i) {
        if (std::strncmp(effect_url, effects[i]->id_, 64) == 0) {
            //  あった
            mat = effects[i]->material_;
        }
    }


    
    return mat;
}



void transposeMatrix(std::vector<float>& mtx)
{
    for (int x = 0; x < 4; ++x) {
        for (int y = x; y < 4;++y) {
            if (x == y) {
                continue;
            }
            int xy = x * 4 + y;
            int yx = y * 4 + x;
            std::swap(mtx[xy], mtx[yx]);
        }
    }

}

}   // unname namespace


namespace tc {



class Parser::Impl
{
public:
Impl()
    : scenes_()
{
}

~Impl()
{
}

    
//----------------------------------------------------------------------
Result parseCollada(
    const xml::XMLDocument* const doc
) {
    
    //  ルートノード取得
    const xml::XMLElement* root_node = doc->RootElement();


    //  visual_scene解析
    VisualScenes visual_scenes;
    const xml::XMLElement* library_visual_scene = firstChildElement(
        root_node,
        "library_visual_scenes"
    );
    collectVisualSceneNode(visual_scenes, library_visual_scene);

    //  マテリアルノード解析
    Materials materials;
    const xml::XMLElement* library_materials = firstChildElement(root_node, "library_materials");
    if (library_materials) {
        collectMaterialNode(materials, library_materials);
    }
    
    //  エフェクトノード解析
    Effects effects;
    const xml::XMLElement* library_effects = firstChildElement(root_node, "library_effects");
    if (library_effects) {
        collectEffectNode(effects, library_effects);
    }

    //  テクスチャパス解析
    Images images;
    const xml::XMLElement* library_images = firstChildElement(root_node, "library_images");
    if (library_images) {
        collectImageNode(images, library_images);
    }


    //  ダンプ
    for (int i = 0; i < visual_scenes.size(); ++i) {
        visual_scenes[i]->dump();
    }
    for (int i = 0; i < materials.size(); ++i) {
        materials[i]->dump();
    }
    for (int i = 0; i < effects.size(); ++i) {
        effects[i]->dump();
    }
    for (int i = 0; i < images.size(); ++i) {
        images[i]->dump();
    }

    //  ジオメトリノード解析
    const xml::XMLElement* library_geometries = firstChildElement(
        root_node,
        LIB_GEOMETRY_NODE_NAME
    );
    
    
    for (int vs_idx = 0; vs_idx < visual_scenes.size(); ++vs_idx) {
        std::shared_ptr<VisualSceneData>& vs = visual_scenes[vs_idx];
        if (vs->type_ != VisualSceneData::TYPE_GEOMETRY) {
            continue;
        }

        const xml::XMLElement* geometry = firstChildElement(
            library_geometries,
            GEOMETRY_NODE_NAME
        );

        //  シーン作成
        std::shared_ptr<ColladaScene> scene = std::make_shared<ColladaScene>();

        //  マトリックス登録
        transposeMatrix(vs->matrix_);
        scene->matrix_ = vs->matrix_;
        scenes_.push_back(scene);
        //  マテリアル設定
        scene->material_ = searchMaterial(vs->bind_material_, materials, effects);
    
        //  メッシュ情報生成
        while (geometry) {
            const char* geometry_id = getElementAttribute(geometry, "id");
            
            printf("a%s ", geometry_id);
            printf("b%s" , vs->url_);
            if (std::strncmp(vs->url_, geometry_id, STRING_COMP_SIZE) != 0) {
                //  次のジオメトリ
                geometry = geometry->NextSiblingElement(GEOMETRY_NODE_NAME);
            }
        
            //  メッシュデータ解析
            const xml::XMLElement* mesh_node = firstChildElement(
                geometry,
                MESH_NODE_NAME
            );
            while (mesh_node) {
            
                std::shared_ptr<ColladaMesh> data = std::make_shared<ColladaMesh>();
                parseMeshNode(mesh_node, data);
            
                //  頂点と法線の並びが同じになっているかチェック
                if (data->hasVertex()) {
                    const ColladaMesh::ArrayData* varray = data->getVertex();
                    if (data->hasNormal()) {
                        const ColladaMesh::ArrayData* narray = data->getNormals();
                        size_t visize = varray->data_.size();
                        size_t nisize = narray->data_.size();
                        TINY_COLLADA_TRACE("%d[v] == %d[n]\n", visize, nisize);
                        TINY_COLLADA_ASSERT(visize == nisize);
                    }
                }

                //  データ登録
//                meshes_.push_back(data);
                scene->meshes_.push_back(data);
                
                //  次へ
                mesh_node = mesh_node->NextSiblingElement(MESH_NODE_NAME);
            }
            break;
        }
        
    }
    
    
    
    return Result::Code::SUCCESS;
}

//----------------------------------------------------------------------
//  メッシュノードの解析
void parseMeshNode(
    const xml::XMLElement* mesh_node,
    ::std::shared_ptr<tc::ColladaMesh> data
) {
    std::shared_ptr<MeshInformation> info = std::make_shared<MeshInformation>();

    //  インデックス情報保存
    collectIndices(mesh_node, info->raw_indices_);
    collectFaceCount(mesh_node, info->face_count_);

    //  ソースノードの情報保存
    collectMeshSources(info->sources_, mesh_node);
    
    //  インプットノードの情報保存
    collectMeshInputs(info->inputs_, mesh_node);
    
    info->dump();
    
    //  ソースとインプットを関連付け
    relateSourcesToInputs(info);
    printf("\n\n");
    for (int i = 0; i < info->sources_.size(); ++i) {
        SourceData* src = &info->sources_[i];
		if (src->input_) {
			printf("SRC:%s - INPUT:%s\n", src->id_, src->input_->source_);
			printf("  DATA size %d\n", src->data_.size());
		}
    }
    printf("\n\n");


    setupMesh(mesh_node, info, data);
    
}

//----------------------------------------------------------------------
void setupMesh(
    const xml::XMLElement* mesh_node,
    std::shared_ptr<MeshInformation> info,
    std::shared_ptr<tc::ColladaMesh> mesh
) {
    //  プリミティブの描画タイプを設定
    tc::ColladaMesh::PrimitiveType prim_type = getPrimitiveType(mesh_node);
    mesh->setPrimitiveType(prim_type);

    int offset_size = info->getIndexStride();

    const SourceData* pos_source = info->searchSourceBySemantic("POSITION");
    if (pos_source) {
        printf("pos_source size %d\n", pos_source->data_.size());
        mesh->vertex_.data_ = pos_source->data_;
        mesh->vertex_.stride_ = pos_source->stride_;
        if (info->face_count_.empty()) {
            setupIndices(
                mesh->vertex_.indices_,
                info->raw_indices_,
                pos_source->input_->offset_,
                offset_size
            );
        }
        else {
            setupIndicesMultiFace(
                mesh->vertex_.indices_,
                info,
                pos_source->input_->offset_,
                offset_size
            );
        }
    }
    
    const SourceData* normal_source = info->searchSourceBySemantic("NORMAL");
    if (normal_source) {
        printf("normal_source size %d\n", normal_source->data_.size());
        
        mesh->normal_.stride_ = normal_source->stride_;
        if (info->face_count_.empty()) {
            setupIndices(
                mesh->normal_.indices_,
                info->raw_indices_,
                normal_source->input_->offset_,
                offset_size
            );
        }
        else {
            setupIndicesMultiFace(
                mesh->normal_.indices_,
                info,
                normal_source->input_->offset_,
                offset_size
            );
        }
        
        //  頂点インデックスにあわせてデータ変更
#if 1
        Indices& vindices = mesh->vertex_.indices_;
        Indices& nindices = mesh->normal_.indices_;
        mesh->normal_.data_.resize(pos_source->data_.size(), 8.8);
        for (int vert_idx = 0; vert_idx < vindices.size(); ++vert_idx) {
            uint32_t vidx = vindices.at(vert_idx);
            uint32_t nidx = nindices.at(vert_idx);
            uint32_t nstride = normal_source->stride_;
            uint32_t from_idx = nidx * nstride;
            uint32_t to_idx = vidx * nstride;
            printf("%d-%d\n", vidx, nidx);
            for (int di = 0; di < nstride; ++di) {
                int to = to_idx + di;
                int from = from_idx + di;
                printf("　　%d -> %d\n", from, to );
                mesh->normal_.data_[to_idx + di] = normal_source->data_.at(from_idx + di);
            }
            
            
            for (int x = 0; x < mesh->normal_.data_.size(); ++x) {
                if ((x % 3) == 0) {
//                    printf("  ");
                }
//                printf ("%1.1f ",mesh->normal_.data_[x]);
                
            }
 //           printf("\n");
        }
#else 
        mesh->normal_.data_ = normal_source->data_;

#endif
        
    }
}




//----------------------------------------------------------------------
//  事前に抜いておいたインデックス一覧からインデックスのセットアップ
void setupIndices(
    Indices& out,
    Indices& src,
    int start_offset,
    int stride
) {
    TINY_COLLADA_TRACE("%s start_offset = %d stride = %d\n", __FUNCTION__, start_offset, stride);
    for (int i = start_offset; i < src.size(); i += stride) {
        out.push_back(src.at(i));
    }
    TINY_COLLADA_TRACE("index size = %lu\n", out.size());
}

//----------------------------------------------------------------------
//  事前に抜いておいたインデックス一覧からインデックスのセットアップ2
void setupIndicesMultiFace(
    Indices& out,
    std::shared_ptr<MeshInformation>& info,
    int start_offset,
    int stride
) {
    int idx = start_offset;
    for (int i = 0; i < info->face_count_.size(); ++ i) {
        int vcnt = info->face_count_.at(i);

        if (vcnt == 3) {
            uint32_t idx1 = info->raw_indices_.at(idx);
            idx += stride;
            uint32_t idx2 = info->raw_indices_.at(idx);
            idx += stride;
            uint32_t idx3 = info->raw_indices_.at(idx);
            idx += stride;
            out.push_back(idx1);
            out.push_back(idx2);
            out.push_back(idx3);

        }
        else if (vcnt == 4) {
            uint32_t idx1 = info->raw_indices_.at(idx);
            idx += stride;
            uint32_t idx2 = info->raw_indices_.at(idx);
            idx += stride;
            uint32_t idx3 = info->raw_indices_.at(idx);
            idx += stride;
            uint32_t idx4 = info->raw_indices_.at(idx);
            idx += stride;

            out.push_back(idx1);
            out.push_back(idx2);
            out.push_back(idx3);

            out.push_back(idx1);
            out.push_back(idx3);
            out.push_back(idx4);
        }
    }
}

    
//----------------------------------------------------------------------
//  メッシュから抜いたinputsとsourcesを関連付ける
void relateSourcesToInputs(
    std::shared_ptr<MeshInformation>& info
)
{
    TINY_COLLADA_TRACE("%s\n", __FUNCTION__);
    auto src_it = info->sources_.begin();
    auto src_end = info->sources_.end();
    

	while (src_it != src_end) {
		InputData* input = info->searchInputBySource(src_it->id_);
		src_it->input_ = input;
		
		TINY_COLLADA_TRACE(" %s", src_it->id_);
		if (input) {
			TINY_COLLADA_TRACE(" OK\n");
			TINY_COLLADA_TRACE("  %s %s\n", input->semantic_, input->source_);
		}
		else {
			TINY_COLLADA_TRACE(" NG\n");
		}
		++src_it;
	}
}
    


//----------------------------------------------------------------------
//  メッシュリスト取得
//const Meshes* getMeshList() const {
    
//    return &meshes_;
//}

const ColladaScenes* getScenes() const {
    return &scenes_;
}


private:
    ColladaScenes scenes_;
//    Meshes meshes_;
};  // class Parser::Impl



//----------------------------------------------------------------------
Parser::Parser()
    : impl_(nullptr)
{
    impl_.reset(new Impl());
}

//----------------------------------------------------------------------
Parser::~Parser()
{
}

//----------------------------------------------------------------------
Result Parser::parse(
    const char* const dae_path
) {
   
    //  tiny xmlを使って.daeを読み込む
    xml::XMLDocument doc;
    xml::XMLError load_error = doc.LoadFile(dae_path);
    if (load_error != xml::XML_SUCCESS) {
        return Result::Code::READ_ERROR;
    }
//    doc.Print();


    //  解析
    Result parse_result = impl_->parseCollada(&doc);
    if (parse_result.isFailed()) {
        return parse_result;
    }

    return Result::Code::SUCCESS;
}

    
//const Meshes* Parser::meshes() const
//{
//    return impl_->getMeshList();
//}

const ColladaScenes* Parser::scenes() const
{
    return impl_->getScenes();
}

//----------------------------------------------------------------------
//  データをコンソールに出力
void ColladaMesh::dump()
{
    printf("--- Vertex data dump ---\n");
    vertex_.dump();
    printf("\n");
    printf("--- Normal data dump ---\n");
    normal_.dump();
    printf("\n");
    
}

//----------------------------------------------------------------------
//  データをコンソールに出力
void ColladaMesh::ArrayData::dump()
{
    if (!isValidate()) {
        printf("This ArrayData is invalidate data.\n");
        return;
    }
    printf("vertices   size = %lu  stride = %lu\n", data_.size(), stride_);
    size_t data_size = data_.size();
    //  データ
    if (data_size > 0) {
        for (size_t i = 0; i < data_size; ++i) {
            printf(" %f", data_[i]);
        }
        printf("\n");
    }
    else {
        printf("nothing.\n");
    }
    
    //  インデックス
    printf("indices   size = %lu\n", indices_.size());
    size_t idx_size = indices_.size();
    //  データ
    if (idx_size > 0) {
        for (size_t i = 0; i < idx_size; ++i) {
            printf(" %d", indices_[i]);
        }
        printf("\n");
    }
    else {
        printf("nothing.\n");
    }
    
    
}

}   // namespace tc

