add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

if is_plat("windows") then
    set_runtimes(is_mode("debug") and "MDd" or "MD")
end

set_languages("c++20")

-- Custom repo
add_repositories("galaxy-repo https://github.com/GalaxyEngine/xmake-repo")
-- Packages
add_requires("galaxyscript v1.0-galaxyengine")


set_rundir("$(projectdir)")
set_targetdir("Generate")

target("YourEngineDLL")
    set_kind("shared")

    add_includedirs("YourEngineDLL")
    add_files("YourEngineDLL/**.cpp")
    add_headerfiles("YourEngineDLL/**.inl");
    add_headerfiles("YourEngineDLL/**.h");

    add_defines("EXAMPLE_EXPORT")

    add_packages("galaxyscript")
target_end()

target("YourEngineBin")
    set_default(true)
    set_kind("binary")
    add_deps("YourEngineDLL")

    add_includedirs("YourEngineDLL")
    add_files("YourEngineBin/main.cpp")
target_end()