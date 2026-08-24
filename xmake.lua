-- set required xmake version
set_xmakever("2.8.2")

-- includes (need xmake.lua file in the same directory)
includes("lib/commonlibsse")
includes("extern/styyx-utils")

-- set up for project

local mod_name = "styyx-random-start-date"

set_project(mod_name)
set_version("1.1.0")
set_license("GPL-3.0")

-- language and warnings
set_languages("c++23")
set_warnings("allextra")

set_encodings("utf-8") -- msvc: /utf-8
set_encodings("source:utf-8", "target:utf-8")

-- xmake rules
add_rules("mode.debug", "mode.releasedbg")
set_defaultmode("releasedbg")
--add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"}) --useful for clion or vscode
add_rules("plugin.vsxmake.autoupdate")

-- commonlib options
set_config("skyrim_ae",true)
set_config("commonlib_toml", true)


-- add plugin target
target(mod_name)
    add_deps("commonlibsse", {public = true})
    add_deps("styyx-util", {public = true})

    add_rules("commonlibsse.plugin", {
        name = mod_name,
        author = "styyx",
        description = "start the game at a random date/time"
    })

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")

    after_build(function(target)
        local dist_root = path.join(os.projectdir(), "Distr")
        local runtime   = has_config("skyrim_ae") and "AE" or "SE"
        local plugins   = path.join(dist_root, runtime, "SKSE", "Plugins")
        os.mkdir(plugins)
        os.trycp(target:targetfile(), plugins)
        os.trycp(target:symbolfile(), plugins)
        os.trycp("$(projectdir)/release/**.toml",  plugins)
        os.trycp("$(projectdir)/release/**.json",  plugins)
    end)