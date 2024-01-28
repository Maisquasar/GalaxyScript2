add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

if is_plat("windows") then
    set_runtimes(is_mode("debug") and "MDd" or "MD")
end

-- Custom repo
add_repositories("galaxy-repo https://github.com/GalaxyEngine/xmake-repo")

-- Packages
add_requires("galaxymath ~1.2")
add_requires("nativefiledialog-extended")

set_languages("c++20")

-- diable warnings
add_cxflags("/wd4251", {tools = "cl"}) -- class needs to have dll-interface to be used by clients of class
add_cxflags("-Wall")            -- Enable all commonly used warning flags

set_rundir("Generate/..")

target("GalaxyScript")
    set_kind("binary")

    if (is_plat("windows", "msvc")) then 
        add_cxflags("/permissive")
    end

    -- Includes --
    add_includedirs("include")
    add_headerfiles("include/**.h");
    add_headerfiles("include/**.inl");
    add_files("src/**.cpp")

    if (is_plat("windows", "msvc")) then 
        add_cxflags("/permissive")
    end

    add_packages("nativefiledialog-extended")
target_end()