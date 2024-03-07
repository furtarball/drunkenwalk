project('drunkenwalk', 'cpp', default_options : [ 'buildtype=debug', 'warning_level=2' ])
subdir('src')
sdl_dep =  [ dependency('sdl2'), dependency('sdl2_ttf'), dependency('sdl2_image') ]
executable('dw', sources : sources, dependencies : sdl_dep)
