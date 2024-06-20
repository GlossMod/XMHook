package("XMHook")

    set_homepage("https://github.com/GlossMod/XMHook")
    set_description("一个简易的hook.")
    set_license("MIT")

    set_urls("https://github.com/GlossMod/XMHook/archive/$(version).tar.gz",
             "https://github.com/GlossMod/XMHook.git")

    add_versions("v1.3.3", "5bec16358ec9086d4593124bf558635e89135abea2c76e5761ecaf09f4546b19")

    on_install("windows", "mingw", function (package)
        io.writefile("xmake.lua", [[
            add_rules("mode.debug", "mode.release")
            target("XMHook")
                set_kind("$(kind)")
                add_files("src/**.c")
                add_headerfiles("src/**.hpp")
        ]])
        local configs = {}
        if package:config("shared") then
            configs.kind = "shared"
        end
        import("package.tools.xmake").install(package, configs)
    end)