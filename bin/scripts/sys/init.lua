
local paths = {
    "scripts/sys/?.lua",
    "scripts/lib/?.lua",
    ------------------------
    "scripts/database/?.lua",
    "scripts/convert/?.lua",
}

package.path = table.concat(paths, ";")

require "output"
