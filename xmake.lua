-- set minimum xmake version
set_xmakever("2.8.2")

includes("lib/commonlibsse")

set_project("CraftTracker")
set_version("1.0.0")
set_license("GPL-3.0")

set_languages("c++23")
set_warnings("allextra")

set_policy("package.requires_lock", true)

add_rules("mode.release")
add_rules("plugin.vsxmake.autoupdate")

target("CraftTracker")
    set_kind("shared")
    add_deps("commonlibsse-ng")

    add_rules("commonlibsse-ng.plugin", {
       name = "CraftTracker",
       author = "Alik",
       description = "Craft Tracker SKSE plugin with Prisma UI"
    })

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")
