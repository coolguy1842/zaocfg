project('zaocfg', 'cpp')



zaolib_proj = subproject('zaolib')
zaolib_dep = zaolib_proj.get_variable('zaolib_dep')

executable(
    'zaocfg',
    'src/main.cpp',
    dependencies: [
        zaolib_dep,
        dependency('hidapi')
    ],
    include_directories: [
        include_directories('include')
    ]
)
