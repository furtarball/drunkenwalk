project('drunkenwalk', 'cpp', default_options : [ 'cpp_std=c++23', 'buildtype=debug', 'warning_level=2' ])
subdir('src')
sdl_dep =  [ dependency('SDL2'), dependency('SDL2_ttf'), dependency('SDL2_image') ]
json_dep = dependency('nlohmann_json')
executable('dw', sources : sources, dependencies : [ sdl_dep, json_dep ])
