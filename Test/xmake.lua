add_rules("mode.debug", "mode.release")

add_repositories("local-repo build")
add_requires("xmhook")

target('Test')
    set_kind('binary')
    add_files('src/**.cpp')
    add_packages('xmhook')