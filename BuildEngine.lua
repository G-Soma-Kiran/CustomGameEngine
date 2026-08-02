local config = arg[1] or "Debug"

local function run(success)
    success = success and os.execute(".\\bin\\".. config .."\\Engine.exe")
    return success
end

local function buildAndCompile(success)

    success = success and os.execute("mingw32-make config="..config)
    return run(success)

end

local function makeFiles()
    local success = os.execute("premake5 gmake")
    return buildAndCompile(success)
end

print(makeFiles() and "Success" or "Failure")