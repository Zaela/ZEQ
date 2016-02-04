
#ifndef _ZEQ_MODEL_RESOURCES_HPP_
#define _ZEQ_MODEL_RESOURCES_HPP_

#include "define.hpp"
#include "database.hpp"
#include "window.hpp"
#include "texture.hpp"
#include "opengl.hpp"
#include "model_prototype.hpp"
#include "zone_model.hpp"
#include "log.hpp"
#include "animated_model.hpp"
#include "axis_aligned_bounding_box.hpp"
#include "vec3.hpp"
#include "lua.hpp"
#include "bone_assignment.hpp"
#include <FreeImage.h>
#include <unordered_map>
#include <string>

#define QUERY_BLOB                                          \
    "SELECT realLength, data FROM Blobs "                   \
    "WHERE id = ?"
    
#define QUERY_INSERT_BLOB                                   \
    "INSERT INTO Blobs (realLength, data) VALUES (?, ?)"

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
    
#define QUERY_MOB_MODEL_ID                                  \
    "SELECT modelId FROM MobModels "                        \
    "WHERE race = ? AND gender = ?"
    
#define QUERY_SKELETONS                                     \
    "SELECT blobId FROM Skeletons WHERE modelId = ?"

#define QUERY_BONE_ASSIGNMENTS                              \
    "SELECT vertId, blobId, isWeighted "                    \
    "FROM BoneAssignments "                                 \
    "WHERE vertId IN "                                      \
    " (SELECT vertId FROM Models2Vertices "                 \
    "  WHERE modelId = ?)"
    
#define QUERY_ANIMATION_FRAMES                              \
    "SELECT boneIndex, animType, blobId "                   \
    "FROM AnimationFrames WHERE modelId = ? "               \
    "ORDER BY boneIndex"
    
#define QUERY_MOB_HEADS                                     \
    "SELECT headModelId, headNumber "                       \
    "FROM MobHeadModels WHERE mainModelId = ? "             \
    "ORDER BY headNumber"

class ModelResources
{
private:
    struct MobModelSet
    {
        MobModelPrototype* gender[3];
        
        MobModelSet()
        {
            for (int i = 0; i < 3; i++)
                gender[i] = nullptr;
        }
    };

    // Resource mappings - these use primary keys from the database as indices
    std::unordered_map<int64_t, Texture*>           m_textures;
    std::unordered_map<int64_t, AnimatedTexture*>   m_textureSets;
    std::unordered_map<int64_t, VertexBuffer*>      m_vertices;
    std::unordered_map<int, MobModelSet>            m_mobModelsByRace;

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
    
    void loadEssentials(int64_t modelId, ModelPrototype* model);
    
    void loadTextures(int64_t modelId, ModelPrototype* model);
    void loadTextureSets(int64_t modelId, ModelPrototype* model);
    void loadVertices(int64_t modelId, ModelPrototype* model);
    void loadGeometry(int64_t modelId);
    
    void loadCachedOctree(int64_t modelId, ZoneModel* zoneModel);
    
    void loadAnimationFrames(int64_t modelId, AnimatedModelPrototype* animModel);
    void loadBoneAssignments(int64_t modelId, AnimatedModelPrototype* animModel);
    void loadHeadModels(int64_t modelId, MobModelPrototype* animModel);
    
    ZoneModel*          loadZoneModel_impl(const std::string& shortname);
    MobModelPrototype*  loadMobModel_impl(int race, uint8_t gender);
    MobModelPrototype*  loadMobModel(int race, uint8_t gender);

public:
    ZoneModel*          loadZoneModel(const std::string& shortname);
    MobModelPrototype*  getMobModel(int race, uint8_t gender);

    void cacheOctree(ZoneModel* zoneModel);

    // Unreference methods
    void removeTexture(int64_t id);
    void removeAnimatedTexture(int64_t id);
    void removeVertexBuffer(int64_t id);
    void removeMobModel(int race, uint8_t gender);
};

#endif//_ZEQ_MODEL_RESOURCES_HPP_
