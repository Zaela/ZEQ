
#ifndef _ZEQ_MODEL_RESOURCES_HPP_
#define _ZEQ_MODEL_RESOURCES_HPP_

#include "define.hpp"
#include "database.hpp"
#include "window.hpp"
#include "texture.hpp"
#include "opengl.hpp"
#include "model_prototype.hpp"
#include "zone_model.hpp"
#include "axis_aligned_bounding_box.hpp"
#include "vec3.hpp"
#include "lua.hpp"
#include <FreeImage.h>
#include <unordered_map>
#include <string>

#define QUERY_BLOB                                          \
    "SELECT realLength, data FROM Blobs "                   \
    "WHERE id = ?"

#define QUERY_MODEL_TEXTURES                                \
    "SELECT texId, "                                        \
    "(SELECT diffuseId FROM Textures WHERE id = texId), "   \
    "(SELECT normalId FROM Textures WHERE id = texId), "    \
    "(SELECT width FROM Textures WHERE id = texId), "       \
    "(SELECT height FROM Textures WHERE id = texId) "       \
    "FROM Models2Textures WHERE modelId = ?"
    
#define QUERY_MODEL_TEXTURE_SETS                            \
    "SELECT ts.id, animMilliseconds, texId "                \
    "FROM TextureSets ts "                                  \
    "JOIN Sets2Textures s2t ON ts.id = s2t.setId "          \
    "JOIN Models2TextureSets mts ON mts.setId = ts.id "     \
    "WHERE mts.modelId = ? "                                \
    "ORDER BY ts.id"
    
#define QUERY_MODEL_VERTICES                                \
    "SELECT id, blobId, noCollision, verticesOnly "         \
    "FROM Models2Vertices m2v "                             \
    "JOIN Vertices v ON v.id = m2v.vertId "                 \
    "WHERE m2v.modelId = ?"
    
#define QUERY_MODEL_GEOMETRY                                \
    "SELECT vertId, texSetId, matId, "                      \
    "(SELECT blendType FROM Materials "                     \
    " WHERE id = g.matId) "                                 \
    "FROM Geometry g "                                      \
    "JOIN Models2Geometry m ON m.geoId = g.id "             \
    "WHERE m.modelId = ? "
    
#define QUERY_ZONE_MODEL_ID                                 \
    "SELECT modelId, octreeCached FROM ZoneModels "         \
    "WHERE shortname = ?"
    
#define QUERY_ZONE_CACHED_OCTREES                           \
    "SELECT minX, minY, minZ, maxX, maxY, maxZ "            \
    "FROM CachedOctrees "                                   \
    "WHERE zoneModelId = ? AND vertId = ?"

class ModelResources
{
private:
    // Queries
    /*Query m_queryBlob;
    Query m_queryModelTextures;
    Query m_queryModelTextureSets;
    Query m_queryModelMaterials;
    Query m_queryModelVertices;
    Query m_queryModelGeometry;
    Query m_queryZoneModelId;
    Query m_queryZoneCachedOctrees;*/

    // The model that is currently being loaded
    ModelPrototype* m_buildModel;

    // Resource mappings - these use primary keys from the database as indices
    std::unordered_map<int64_t, Texture*>           m_textures;
    std::unordered_map<int64_t, AnimatedTexture*>   m_textureSets;
    std::unordered_map<int64_t, VertexBuffer*>      m_vertices;

private:
    struct Blob
    {
        byte*       data;
        uint32_t    length;
        bool        needsDelete;
        Query       query;
        
        Blob() : data(nullptr), length(0), needsDelete(false) { }
        
        ~Blob();
        
        byte* takeOwnership()
        {
            if (data)
            {
                byte* ret;
                
                if (needsDelete)
                {
                    ret = data;
                    needsDelete = false;
                }
                else
                {
                    ret = new byte[length];
                    memcpy(ret, data, length);
                }
                
                data = nullptr;
                return ret;
            }
            
            return nullptr;
        }
    };
    
    int64_t     insertBlob(void* data, uint32_t len, bool compress = true);
    void        getBlob(int64_t id, Blob& blob);
    uint32_t    getTextureBlob(int64_t id, int width, int height);
    
    void loadEssentials(int64_t modelId);
    
    void loadTextures(int64_t modelId);
    void loadTextureSets(int64_t modelId);
    void loadVertices(int64_t modelId);
    void loadGeometry(int64_t modelId);
    
    void loadCachedOctree(int64_t modelId, ZoneModel* zoneModel);
    
    ZoneModel* loadZoneModel_impl(const std::string& shortname);

public:
    void init();

    ZoneModel* loadZoneModel(const std::string& shortname);

    void cacheOctree(ZoneModel* zoneModel);

    // Unreference methods
    void removeTexture(int64_t id);
    void removeAnimatedTexture(int64_t id);
    void removeVertexBuffer(int64_t id);
};

#endif//_ZEQ_MODEL_RESOURCES_HPP_
