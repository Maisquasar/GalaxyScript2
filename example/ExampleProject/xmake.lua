add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

if is_plat("windows") then
    set_runtimes(is_mode("debug") and "MDd" or "MD")
end

-- Custom repo
add_repositories("galaxy-repo https://github.com/GalaxyEngine/xmake-repo")
-- Packages
add_requires("galaxyscript")

set_languages("c++17")

add_cxflags("/wd4251", {tools = "cl"}) -- diable warnings : class needs to have dll-interface to be used by clients of class
add_cxflags("-Wall")            -- Enable all commonly used warning flags

set_rundir("$(projectdir)")
set_targetdir("Generate")

target("ExampleProject")
    set_kind("shared")

    add_packages("galaxyscript")

    before_build(function (target)
        local script_dir = os.scriptdir()
        print("Running GalaxyHeaderTool inside " .. script_dir)
        -- Here need to be replace by the path of the GalaxyHeaderTool binary
        local program = "../GalaxyHeaderTool"
        local args = {script_dir, script_dir .. "/Generate/Headers"}

        os.runv(program, args)
    end)

    -- Includes --
    add_includedirs("Assets")
    -- The path of the Generated Header files
    add_includedirs("Generate/Headers")
    -- Here need to be replaced by directory of the includes of the Engine
    add_includedirs("../YourEngineDLL") -- TODO explain this

    -- Here need to be replaced by directory of the DLL of the Engine
    add_linkdirs("../Generate")
    -- Here need to be replaced by the name of the DLL of the Engine
    add_links("YourEngineDLL")

    add_headerfiles("Assets/**.h");
    add_headerfiles("Assets/**.inl");
    add_files("**.cpp")

    if (is_plat("windows", "msvc")) then 
        add_cxflags("/permissive")
    end

    -- To avoid "lib" prefix when compiling with MinGW or linux
    set_prefixname("")
target_end()