
--------------------------------------------------------------------------------
-- ZEQ Model Database Schema
-- Targeting SQLite 3.10.*
--------------------------------------------------------------------------------

PRAGMA page_size = 8192; -- Optimal page size for small blob IO

CREATE TABLE Blobs (
    id          INTEGER PRIMARY KEY,
    realLength  INT, -- If realLength is non-zero, the blob is zlib compressed
    data        BLOB
);

CREATE INDEX Index_Blobs ON Blobs (realLength);

CREATE TABLE TextureSets (
    id                  INTEGER PRIMARY KEY,
    animMilliseconds    INT -- Number of milliseconds to display each frame, if this is an animated texture
);

CREATE TABLE Textures (
    id          INTEGER PRIMARY KEY,
    name        TEXT,
    diffuseId   INT,
    normalId    INT,
    width       INT,
    height      INT
);

CREATE TABLE Sets2Textures (
    setId   INT,
    texId   INT
);

CREATE INDEX Index_Sets2Textures ON Sets2Textures (setId);

CREATE TABLE Materials (
    id          INTEGER PRIMARY KEY,
    name        TEXT,
    -- Blend types:
    -- 0 No Blending (basic solids)
    -- 1 Alpha >= 0.5 (masked, e.g. for trees)
    -- 2 Depth Mask disabled, Blend Func one : one-minus-src-color, Alpha >= 0.5 (particle-style, e.g. for fires)
    -- 3 Depth Mask disabled, Blend Func one : src-color (cheap additive blending, e.g. for water surfaces)
    -- 4 Invisible (e.g. zone walls)
    blendType   INT
);

INSERT INTO Materials (name, blendType) VALUES ('NULL', 4);

CREATE TABLE Vertices (
    id              INTEGER PRIMARY KEY,
    noCollision     BOOLEAN,
    verticesOnly    BOOLEAN DEFAULT 0,
    blobId          INT
);

-- Geometry links Textures to Materials to Vertices; a set of geometry covered by a single texture
CREATE TABLE Geometry (
    id          INTEGER PRIMARY KEY,
    vertId      INT,
    texSetId    INT,
    matId       INT
);

CREATE TABLE Models (
    id INTEGER PRIMARY KEY
);

CREATE TABLE Models2Textures (
    modelId INT,
    texId   INT,
    PRIMARY KEY(modelId, texId)
);

CREATE TABLE Models2TextureSets (
    modelId INT,
    setId   INT,
    PRIMARY KEY (modelId, setId)
);

CREATE TABLE Models2Materials (
    modelId INT,
    matId   INT,
    PRIMARY KEY (modelId, matId)
);

CREATE TABLE Models2Vertices (
    modelId INT,
    vertId  INT,
    PRIMARY KEY (modelId, vertId)
);

CREATE TABLE Models2Geometry (
    modelId INT,
    geoId   INT,
    PRIMARY KEY (modelId, geoId)
);

CREATE TABLE ZoneModels (
    shortname       TEXT PRIMARY KEY,
    modelId         INT,
    octreeCached    BOOLEAN DEFAULT 0
);

CREATE TABLE CachedOctrees (
    zoneModelId INT,
    vertId      INT,
    minX        REAL,
    minY        REAL,
    minZ        REAL,
    maxX        REAL,
    maxY        REAL,
    maxZ        REAL
);

CREATE INDEX Index_CachedOctrees ON CachedOctrees (zoneModelId, vertId);

CREATE TABLE ObjectModels (
    modelId     INT PRIMARY KEY,
    isTree      BOOLEAN,
    isAnimated  BOOLEAN DEFAULT 0
);

-- Mainly for Minor Illusion and Treeform
CREATE TABLE ObjectPlacements (
    zoneModelId INT,
    objModelId  INT,
    x           REAL,
    y           REAL,
    z           REAL
);
