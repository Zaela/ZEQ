
--[[ Filesystem Options ]]--

-- Use forward slashes
PathToEverquest = "C:/Everquest/"


--[[ Screen Options ]]--

-- Note that ScreenWidth and ScreenHeight are used to calculate your aspect ratio even when using Fullscreen,
-- unless you also use UseNativeAspectRatio at the same time.

ScreenWidth             = 800
ScreenHeight            = 600
Fullscreen              = false
UseNativeAspectRatio    = false
HideTitleBar            = false
Vsync                   = false
AntiAliasLevel          = 0


--[[ Performance Options ]]--

SleepMillisecondsBetweenFrames  = 25
SleepMillisecondsInBackground   = 100


--[[ Zone Loading Options ]]--

-- Octrees can be cached to improve loading times for some zones at the cost of taking up more disk space.
-- Generating octrees dynamically at load time is usually fast enough, but some zones present degenerate
-- cases that take unusually long times (for example, most zones take about 0.5 seconds on my machine, but
-- growthplane can take as much as 20 seconds).

-- If generating an octree takes more than the following number of seconds, it will automatically be cached
-- to improve subsequent load times.
CacheOctreesIfLongerThan = 1.5

-- If you aren't concerned about disk space or want to take maximum advantage of an SSD or ramdisk, use
-- this option to force octrees to be cached whenever a zone is loaded for the first time. This makes
-- the process of loading a zone almost entirely disk-bound.
AlwaysCacheOctrees = true
