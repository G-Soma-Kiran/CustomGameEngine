local scene  = arg[1] or "all"
local config = arg[2] or "Debug"

local root = (arg[0]:gsub("/", "\\")):match("^(.*)\\[^\\]*$") or "."

local l = config:lower()
if l ~= "debug" and l ~= "release" then
    print("Unknown configuration '" .. config .. "'. Use Debug or Release.")
    os.exit(1)
end
config = l:sub(1, 1):upper() .. l:sub(2)

local function ok(r) return r == true or r == 0 end

local function sh(cmd)
    print("> " .. cmd)
    return ok(os.execute('cd /d "' .. root .. '" && ' .. cmd))
end

local function findGame(name)
    local p = io.popen('dir /b /ad "' .. root .. '\\Games" 2>nul')
    if not p then return nil, {} end
    local found, available = nil, {}
    for dir in p:lines() do
        local f = io.open(root .. "\\Games\\" .. dir .. "\\premake5.lua", "r")
        if f then
            f:close()
            table.insert(available, dir)
            if dir:lower() == name:lower() then found = dir end
        end
    end
    p:close()
    return found, available
end

local target, exe

if scene:lower() == "all" then
    target = "Engine"
    exe    = ".\\bin\\" .. config .. "\\Engine.exe"
else
    local game, available = findGame(scene)
    if not game then
        print("Scene '" .. scene .. "' is not present.")
        print("Available: " .. table.concat(available, ", "))
        os.exit(1)
    end
    target = game .. "App"
    exe    = ".\\bin\\" .. config .. "\\" .. game .. ".exe"
end

local success = sh("premake5 gmake")
    and sh("mingw32-make config=" .. config .. " " .. target)
    and sh(exe)

print(success and "Success" or "Failure")
os.exit(success and 0 or 1)