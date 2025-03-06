project('drunkenwalk', 'cpp', default_options : [ 'cpp_std=c++20', 'buildtype=debug', 'warning_level=2' ])
subdir('src')

sdl_dep =  [ dependency('SDL2'), dependency('SDL2_ttf'), dependency('SDL2_image') ]
json_dep = dependency('nlohmann_json')
executable('drunkenwalk', sources : sources, dependencies : [ sdl_dep, json_dep ], install : true)

if host_machine.system() in ['linux', 'bsd']
	desktopfiledir = join_paths(get_option('prefix'), 'share/applications')
	install_data('io.github.furtarball.drunkenwalk.desktop', install_dir : desktopfiledir)
	iconsdir = join_paths(get_option('prefix'), 'share/pixmaps')
	install_data('icon.png', rename : 'drunkenwalk.png', install_dir : iconsdir)
endif
assetsdir = join_paths(get_option('datadir'), 'drunkenwalk')

install_subdir('assets', install_dir : assetsdir, strip_directory : true)
configure_file(input : 'compiletime.h.in', output : 'compiletime.h',
	configuration : {
		'assetsdir': assetsdir # relative to prefix
	})

appimage_target = custom_target('appimage', command : [ './appimage.sh' ], output : 'dw.AppImage')
