project('drunkenwalk', 'cpp', default_options : [ 'buildtype=debug', 'warning_level=2' ])
subdir('src')
sdl_dep =  [ dependency('SDL2'), dependency('SDL2_ttf'), dependency('SDL2_image') ]
executable('dw', sources : sources, dependencies : sdl_dep)
