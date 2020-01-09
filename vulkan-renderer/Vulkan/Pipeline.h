
enum PipelineType
{
  MODEL
};

struct PipelineId
{
  PipelineType type;
  struct Model
  {
    struct PrimitivePart
    {
      union {
        struct
        {
          unsigned int positionVertexData : 1;
          unsigned int normalVertexData : 1;
          unsigned int tangentVertexData : 1;
          unsigned int countTexCoord : 2;
          unsigned int countColor : 2;
          unsigned int primitiveMode : 3;
        };
        uint32_t value;
      };
    };
    struct MaterialPart
    {
      union {
        struct
        {
          unsigned int baseColorInfo:          2;
          unsigned int metallicRoughnessInfo:  2;
          unsigned int normalInfo:             2;
          unsigned int occlusionInfo:          2;
          unsigned int emissiveInfo:           2;
        };

        uint32_t value;
      };
    };
  };
};

class Pipeline
{
public:
    Pipeline(PipelineId id);
    ~Pipeline();

private:
    PipelineId
};