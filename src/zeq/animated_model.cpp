
#include "animated_model.hpp"
#include "model_resources.hpp"

extern ModelResources gModelResources;

AnimatedModelPrototype::~AnimatedModelPrototype()
{
    m_animations.destroy();
}

void AnimatedModelPrototype::readSkeleton(byte* binBones, uint32_t len)
{
    DBBone* frames  = (DBBone*)binBones;
    uint32_t count  = len / sizeof(DBBone);
    
    m_boneCount = count;
    m_bones = new Bone[count];
    
    uint32_t cur = 0;
    readSkeletonRecurse(frames, cur, m_bones[0], count);
}

void AnimatedModelPrototype::readSkeletonRecurse(DBBone* frames, uint32_t& cur, Bone& bone, uint32_t count, Bone* parent, uint32_t parentIndex)
{
    if (cur == count)
        return;
    
    DBBone& frame = frames[cur];
    
    bone.hasAnimFrames = false;
    
    bone.pos    = frame.pos;
    bone.scale  = frame.scale;
    bone.rot    = frame.rot;
    
    Mat4 posMatrix;
    posMatrix.setTranslation(frame.pos);
    Mat4 rotMatrix;
    frame.rot.getMatrixTransposed(rotMatrix);
    Mat4 scaleMatrix;
    scaleMatrix.setScale(frame.scale);
    
    Mat4 localMatrix = posMatrix * rotMatrix * scaleMatrix;
    
    if (parent)
        bone.globalMatrix = parent->globalMatrix * localMatrix;
    else
        bone.globalMatrix = localMatrix;
    
    bone.globalInverseMatrix = bone.globalMatrix;
    bone.globalInverseMatrix.invert();
    
    bone.parentIndex        = parentIndex;
    bone.attachPointType    = frame.attachPointType;
    
    uint32_t n = frame.childCount;
    
    if (n)
    {
        uint32_t index = cur;
        
        for (uint32_t i = 0; i < n; i++)
        {
            Bone& child = m_bones[++cur];
            readSkeletonRecurse(frames, cur, child, count, &bone, index);
        }
    }
}

void AnimatedModelPrototype::readAnimationFrames(int animId, int boneIndex, byte* frames, uint32_t len)
{
    m_bones[boneIndex].hasAnimFrames = true;
    m_animations.set(animId, boneIndex, m_boneCount, frames, len);
}

WeightedBoneAssignmentSet& AnimatedModelPrototype::readWeightedBoneAssignments(byte* binWbas, uint32_t len)
{
    WeightedBoneAssignmentSet set;
    
    set.count       = len / sizeof(WeightedBoneAssignment);
    set.assignments = (WeightedBoneAssignment*)binWbas;
    
    m_weightedBoneAssignments.push_back(set);
    return m_weightedBoneAssignments.back();
}

Skeleton* AnimatedModelPrototype::createSkeletonInstance()
{
    Skeleton* sk = new Skeleton;
    
    // The skeleton gets its own copies of all the bones
    uint32_t count  = m_boneCount;
    sk->m_boneCount = count;
    
    Skeleton::Bone* bones   = new Skeleton::Bone[count];
    sk->m_bones             = bones;
    sk->m_animMatrices      = new Mat4[count];
    
    memset(bones, 0, sizeof(Skeleton::Bone) * count);
    
    for (uint32_t i = 0; i < count; i++)
    {
        Skeleton::Bone& dst = bones[i];
        Bone& src           = m_bones[i];
        
        dst.hasAnimFrames   = src.hasAnimFrames;
        
        dst.pos     = src.pos;
        dst.rot     = src.rot;
        dst.scale   = src.scale;
        
        //dst.localAnimMatrix     = src.localMatrix;
        dst.globalAnimMatrix    = src.globalMatrix;
        dst.globalInverseMatrix = src.globalInverseMatrix;
        
        dst.parentGlobalAnimMatrix = (i != 0) ? &bones[src.parentIndex].globalAnimMatrix : nullptr;
        
        dst.animHint        = Animation::DEFAULT_HINT;
        dst.attachPointSlot = src.attachPointType;
    }
    
    // The skeleton needs its own copies of all the VertexBuffers, but needs the original VertexBuffers to transform each frame
    // The bone assignment definitions are centralized, belonging to the prototype
    auto& vbs   = sk->m_ownedVertexBuffers;
    count       = m_weightedBoneAssignments.size();
    
    if (count)
    {
        auto& sets = sk->m_vertexBufferSets;

        for (uint32_t i = 0; i < count; i++)
        {
            WeightedBoneAssignmentSet& src = m_weightedBoneAssignments[i];
            
            vbs.push_back(VertexBuffer());
            VertexBuffer& vb = vbs.back();
            
            vb.copy(*src.vertexBuffer);
            
            Skeleton::VertexBufferSet set;
            
            set.vertexCount     = vb.count();
            set.target          = vb.array();
            set.base            = src.vertexBuffer->array();
            set.assignmentCount = src.count;
            set.assignments     = src.assignments;
            
            sets.push_back(set);
        }
    }
    else
    {
        auto& sets = sk->m_simpleVertexBufferSets;
        
        for (VertexBuffer* src : getReferencedVertexBuffers())
        {
            vbs.push_back(VertexBuffer());
            VertexBuffer& vb = vbs.back();
            
            vb.copy(*src);
            
            Skeleton::SimpleVertexBufferSet set;
            
            set.vertexCount = vb.count();
            set.target      = vb.array();
            set.base        = src->array();
            
            sets.push_back(set);
        }
    }
    
    // Animation definitions are centralized, belonging to the prototype
    sk->m_animations.inherit(m_animations);
    
    sk->m_prototype = this;
    
    return sk;
}

MobModelPrototype::MobModelPrototype(int race, uint8_t gender)
    : m_race(race),
      m_gender(gender)
{
    
}

MobModelPrototype::~MobModelPrototype()
{
    gModelResources.removeMobModel(m_race, m_gender);
}

void MobModelPrototype::addHeadModel(AnimatedModelPrototype& headModel, int headIndex)
{
    (void)headIndex; // At the moment we are assuming indices are sequential starting from 0
    
    for (Texture* tex : headModel.m_referencedTextures)
    {
        m_referencedTextures.push_back(tex);
    }
    
    headModel.m_referencedTextures.clear();
    
    for (AnimatedTexture* animTex : headModel.m_referencedAnimatedTextures)
    {
        m_referencedAnimatedTextures.push_back(animTex);
    }
    
    headModel.m_referencedAnimatedTextures.clear();
    
    for (VertexBuffer* vb : headModel.m_referencedVertexBuffers)
    {
        m_referencedVertexBuffers.push_back(vb);
    }
    
    auto& wbas      = headModel.m_weightedBoneAssignments;
    uint32_t count  = wbas.size();
    
    if (count != 0)
    {
        // EQG head model
        WeightedHeadModel head;
        
        head.count = count;
        
        if (count == 1)
        {
            head.setSingle = wbas[0];
        }
        else
        {
            head.setArray = new WeightedBoneAssignmentSet[count];
            
            count = 0;
            for (WeightedBoneAssignmentSet& set : wbas)
            {
                head.setArray[count++] = set;
            }
        }
        
        m_weightedHeads.push_back(head);
        
        wbas.clear();
    }
    else
    {
        // WLD head model
        SimpleHeadModel head;
        
        count = headModel.m_referencedVertexBuffers.size();
        
        head.count = count;
        
        if (count == 1)
        {
            head.vbSingle = headModel.m_referencedVertexBuffers[0];
        }
        else
        {
            head.vbArray = new VertexBuffer*[count];
            
            count = 0;
            for (VertexBuffer* vb : headModel.m_referencedVertexBuffers)
            {
                head.vbArray[count++] = vb;
            }
        }
        
        m_simpleHeads.push_back(head);
    }
        
    headModel.m_referencedVertexBuffers.clear();
}

Skeleton* MobModelPrototype::createSkeletonInstance()
{
    Skeleton* sk    = AnimatedModelPrototype::createSkeletonInstance();
    auto& vbs       = sk->m_ownedVertexBuffers;
    
    if (m_weightedHeads.size())
    {
        WeightedHeadModel& head = m_weightedHeads[0];
        
        TempVector<WeightedBoneAssignmentSet> wbas;
        
        if (head.count == 1)
        {
            wbas.push_back(head.setSingle);
        }
        else
        {
            for (uint32_t i = 0; i < head.count; i++)
            {
                wbas.push_back(head.setArray[i]);
            }
        }
        
        auto& sets = sk->m_vertexBufferSets;
        
        for (WeightedBoneAssignmentSet& src : wbas)
        {
            vbs.push_back(VertexBuffer());
            VertexBuffer& vb = vbs.back();
            
            vb.copy(*src.vertexBuffer);
            
            Skeleton::VertexBufferSet set;
            
            set.vertexCount     = vb.count();
            set.target          = vb.array();
            set.base            = src.vertexBuffer->array();
            set.assignmentCount = src.count;
            set.assignments     = src.assignments;
            
            sets.push_back(set);
        }
    }
    else if (m_simpleHeads.size())
    {
        SimpleHeadModel& head = m_simpleHeads[0];
        
        TempVector<VertexBuffer*> tvbs;
        
        if (head.count == 1)
        {
            tvbs.push_back(head.vbSingle);
        }
        else
        {
            for (uint32_t i = 0; i < head.count; i++)
            {
                tvbs.push_back(head.vbArray[i]);
            }
        }
        
        auto& sets = sk->m_simpleVertexBufferSets;
        
        for (VertexBuffer* src : tvbs)
        {
            vbs.push_back(VertexBuffer());
            VertexBuffer& vb = vbs.back();
            
            vb.copy(*src);
            
            Skeleton::SimpleVertexBufferSet set;
            
            set.vertexCount = vb.count();
            set.target      = vb.array();
            set.base        = src->array();
            
            sets.push_back(set);
        }
    }
    
    return sk;
}
