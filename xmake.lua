add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

if is_plat("windows") then
    set_runtimes(is_mode("debug") and "MDd" or "MD")
end

-- Custom repo
add_repositories("galaxy-repo https://github.com/GalaxyEngine/xmake-repo")

-- Packages
add_requires("cpp_serializer")

set_languages("c++20")

-- diable warnings
add_cxflags("/wd4251", {tools = "cl"}) -- class needs to have dll-interface to be used by clients of class
add_cxflags("-Wall")            -- Enable all commonly used warning flags

set_rundir("$(projectdir)")

target("GalaxyScript")
    set_kind("static")

    if (is_plat("windows", "msvc")) then 
        add_cxflags("/permissive")
    end

    -- Includes --
    add_includedirs("include")
    add_headerfiles("include/**.h");
    add_headerfiles("include/**.inl");
    add_files("src/**.cpp")

    if (is_plat("linux")) then
        add_cxxflags("-fPIC")  -- Usefull for linux
    end

    if (is_plat("windows", "msvc")) then 
        add_cxflags("/permissive")
    end

    add_packages("cpp_serializer")
target_end()