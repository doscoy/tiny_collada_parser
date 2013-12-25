//  mcp_parser.cpp

#include "tiny_collada_parser.hpp"
#include "third_party_libs/tinyxml2/tinyxml2.h"



#if 1
#define TINY_COLLADA_TRACE(...)         ::std::printf(__VA_ARGS__)
#else
#define TINY_COLLADA_TRACE(...)         (void)0
#endif


namespace xml = tinyxml2;




namespace {


struct PrimitiveSelector
{
    const char* name_;
    tc::Mesh::PrimitiveType type_;
};

#define PRIMITIVE_TYPE_NUM 2
const PrimitiveSelector PRIMITIVE_TYPE_SELECT[PRIMITIVE_TYPE_NUM] = {
    {"triangles", tc::Mesh::PRIMITIVE_TRIANGLES},
    {"polylist", tc::Mesh::PRIMITIVE_TRIANGLES}
};


const size_t STRING_COMP_SIZE = 64;


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
    SourceData* out
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
tc::Mesh::PrimitiveType getPrimitiveType(
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

    return tc::Mesh::UNKNOWN_TYPE;
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
        char* text = const_cast<char*>(primitive_node->FirstChildElement("p")->GetText());
       readArray(text, &indices);
    }
    
    TINY_COLLADA_TRACE("Collect index size = %lu\n", indices.size());
}



//----------------------------------------------------------------------
//  メッシュノードのソース情報を取得
void collectMeshSources(
    std::vector<SourceData>& out,
    const xml::XMLElement* mesh
){
    //  ソースノードを総なめして情報を保存
    const xml::XMLElement* target = firstChildElement(mesh, "source");
    while (target) {
        SourceData data;
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
    std::vector<InputData>& out,
    const xml::XMLElement* mesh,
    const char* const target_name
){
    //  verticesノードのインプットとの２重連携の為に
    const xml::XMLElement* vertices =  firstChildElement(mesh, "vertices");

    //  inputノード
    const xml::XMLElement* target_node = getPrimitiveNode(mesh);
	if (!target_node) {
		return;
	}
    const xml::XMLElement* input_node = firstChildElement(target_node, "input");

    while (input_node) {
        InputData input;
        //  sourceアトリビュートを取得
        const char* attr_source = getElementAttribute(input_node, "source");
        //  source_nameの先頭の#を取る
        if (attr_source[0] == '#') {
            attr_source = &attr_source[1];
        }

        //  verticesノードのinputで置き換えるべきものか判定
        if (vertices) {
            const char* vertices_id = getElementAttribute(vertices, "id");
            if (std::strncmp(attr_source, vertices_id, STRING_COMP_SIZE) == 0) {
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



class Parser::Impl
{
public:
Impl()
    : raw_indices_()
    , meshes_()
    , sources_()
    , inputs_()
{
}

~Impl()
{
}

    
//----------------------------------------------------------------------
Result parseCollada(
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
        std::shared_ptr<Mesh> data = std::make_shared<Mesh>();
        const xml::XMLElement* mesh_node = firstChildElement(geometry_node, "mesh");
        
        parseMeshNode(mesh_node, data);
        meshes_.push_back(data);
        
        //  次のジオメトリ
        geometry_node = geometry_node->NextSiblingElement("geometry");
    }
    
    return Result::Code::SUCCESS;
}

//----------------------------------------------------------------------
//  メッシュノードの解析
void parseMeshNode(
    const xml::XMLElement* mesh_node,
    ::std::shared_ptr<tc::Mesh> data
) {
    //  インデックス情報保存
    collectIndices(mesh_node, raw_indices_);
    
    //  ソースノードの情報保存
    collectMeshSources(sources_, mesh_node);
    
    //  インプットノードの情報保存
    collectMeshInputs(inputs_, mesh_node, "polylist");
    
    //  ソースとインプットを関連付け
    relateSourcesToInputs();
    printf("\n\n");
    for (int i = 0; i < sources_.size(); ++i) {
        SourceData* src = &sources_[i];
		if (src->input_) {
			printf("SRC:%s - INPUT:%s\n", src->id_, src->input_->source_);
			printf("  DATA size %d\n", src->data_.size());
		}
    }
    printf("\n\n");


    setupMesh(mesh_node, data);
    
}

//----------------------------------------------------------------------
void setupMesh(
    const xml::XMLElement* mesh_node,
    ::std::shared_ptr<tc::Mesh> mesh
) {
    //  プリミティブの描画タイプを設定
    tc::Mesh::PrimitiveType prim_type = getPrimitiveType(mesh_node);
    mesh->setPrimitiveType(prim_type);

    SourceData* pos_source = searchSourceBySemantic("POSITION");
    if (pos_source) {
        printf("pos_source size %d\n", pos_source->data_.size());
        mesh->vertex_.data_ = pos_source->data_;
        mesh->vertex_.stride_ = pos_source->stride_;
        setupIndices(mesh->vertex_.indices_, pos_source->input_->offset_, 2);
    }
    
    SourceData* normal_source = searchSourceBySemantic("NORMAL");
    if (normal_source) {
        printf("normal_source size %d\n", normal_source->data_.size());
        mesh->normal_.data_ = normal_source->data_;
        mesh->normal_.stride_ = normal_source->stride_;
        setupIndices(mesh->normal_.indices_, normal_source->input_->offset_, 2);
    }
}



//----------------------------------------------------------------------
//  事前に抜いておいたインデックス一覧からインデックスのセットアップ
void setupIndices(
    Indices& out,
    int start_offset,
    int stride
) {
    TINY_COLLADA_TRACE("%s start_offset = %d stride = %d\n", __FUNCTION__, start_offset, stride);
    for (int i = start_offset; i < raw_indices_.size(); i += stride) {
        out.push_back(raw_indices_.at(i));
    }
    TINY_COLLADA_TRACE("get size = %lu\n", out.size());
}


    
//----------------------------------------------------------------------
//  メッシュから抜いたinputsとsourcesを関連付ける
void relateSourcesToInputs()
{
    TINY_COLLADA_TRACE("%s\n", __FUNCTION__);
    std::vector<SourceData>::iterator src_it = sources_.begin();
    std::vector<SourceData>::iterator src_end = sources_.end();
    

	while (src_it != src_end) {
		InputData* input = searchInputBySource(src_it->id_);
		src_it->input_ = input;
		
		TINY_COLLADA_TRACE(" %s", src_it->id_);
		if (input) {
			TINY_COLLADA_TRACE(" OK\n");
			TINY_COLLADA_TRACE("  %s %s", input->semantic_, input->source_);
		}
		else {
			TINY_COLLADA_TRACE(" NG\n");
		}
		++src_it;
	}
}
    
//----------------------------------------------------------------------
//  指定idのinputを探す
InputData* searchInputBySource(
    const char* const id
) {
    for (int i = 0; i < inputs_.size(); ++i) {
        InputData* input = &inputs_[i];
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
    printf("\nsearchSourceBySemantic %s\n", semantic);
    for (int i = 0; i < sources_.size(); ++i) {
        SourceData* source = &sources_[i];
        InputData* input = source->input_;
        if (!input) {
            continue;
        }
        printf("  %s %s\n", input->semantic_, input->source_);   
        if (std::strncmp(input->semantic_, semantic, STRING_COMP_SIZE) == 0) {
            TINY_COLLADA_TRACE("%s - %s [%s] FOUND.\n", __FUNCTION__, semantic, input->source_);
            return source;
        }
    }
    TINY_COLLADA_TRACE("%s - %s NOT FOUND.\n", __FUNCTION__, semantic);
        
    return nullptr;
}

//----------------------------------------------------------------------
//  メッシュリスト取得
const Meshes* getMeshList() const {
    
    return &meshes_;
}


private:
    Indices raw_indices_;
    Meshes meshes_;
    std::vector<SourceData> sources_;
    std::vector<InputData> inputs_;

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

    
const Meshes* Parser::meshes() const
{
    return impl_->getMeshList();
}

//----------------------------------------------------------------------
//  データをコンソールに出力
void Mesh::dump()
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
void Mesh::ArrayData::dump()
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

