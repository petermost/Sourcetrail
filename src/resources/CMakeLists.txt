add_library(Sourcetrail_res_gui
	QtResources.h
	QtResources.cpp
)

qt_add_resources(Sourcetrail_res_gui Sourcetrail_resources
	PREFIX "/"
	FILES
		# Before changing any paths, check whether they are referenced via <setting:gui_path> in the stylesheets!
		about/about.css
		about/logo_sourcetrail.png
		
		bookmark_view/images/arrow_down.png
		bookmark_view/images/arrow_line_down.png
		bookmark_view/images/arrow_line_up.png
		bookmark_view/images/arrow_right.png
		bookmark_view/images/bookmark_active.png
		bookmark_view/images/bookmark_delete_icon.png
		bookmark_view/images/bookmark_edit_icon.png
		bookmark_view/images/bookmark_list_icon.png
		bookmark_view/images/edit_bookmark_icon.png
		bookmark_view/bookmark_view.css
		
		code_view/code_view.css
		code_view/images/arrow_down.png
		code_view/images/arrow_left.png
		code_view/images/arrow_right.png
		code_view/images/arrow_up.png
		code_view/images/button_background.png
		code_view/images/edit.png
		code_view/images/file.png
		code_view/images/list.png
		code_view/images/maximize_active.png
		code_view/images/maximize_inactive.png
		code_view/images/pattern_bright.png
		code_view/images/pattern_dark.png
		code_view/images/pattern_grey.png
		code_view/images/snippet_active.png
		code_view/images/snippet_arrow_down.png
		code_view/images/snippet_arrow_right.png
		code_view/images/snippet_inactive.png
		
		custom_trail_view/custom_trail_view.css
		custom_trail_view/images/circle.png
		
		graph_view/images/annotation.png
		graph_view/images/arrow.png
		graph_view/images/bundle.png
		graph_view/images/default.png
		graph_view/images/enum.png
		graph_view/images/file_incomplete.png
		graph_view/images/file.png
		graph_view/images/graph_arrow.png
		graph_view/images/graph_custom.png
		graph_view/images/graph_down.png
		graph_view/images/graph_left.png
		graph_view/images/graph.png
		graph_view/images/graph_right.png
		graph_view/images/graph_up.png
		graph_view/images/group_namespace.png
		graph_view/images/legend.png
		graph_view/images/macro.png
		graph_view/images/namespace.png
		graph_view/images/pattern.png
		graph_view/images/private.png
		graph_view/images/protected.png
		graph_view/images/public.png
		graph_view/images/template.png
		graph_view/images/typedef.png
		graph_view/images/zoom_in.png
		graph_view/images/zoom_out.png
		graph_view/graph_view.css
		
		history_list/history_list.css
		history_list/images/arrow.png
		
		icon/cbp_icon.png
		icon/cdb_icon.png
		icon/c_icon.png
		icon/cpp_icon.png
		icon/empty_icon.png
		icon/gradle_icon.png
		icon/java_icon.png
		icon/logo_1024_1024.png
		icon/mvn_icon.png
		icon/project_256_256.png
		icon/project.ico
		icon/vs_icon.png
		
		indexing_dialog/indexing_dialog.css
		indexing_dialog/error.png
		indexing_dialog/flag.png
		indexing_dialog/progress_bar_element.png
		
		keyboard_shortcuts/keyboard_shortcuts.css
		
		main/images/close_black.png
		main/images/close_grey.png
		main/images/close_white.png
		main/images/float_black.png
		main/images/float_grey.png
		main/images/float_white.png
		main/main.css
		main/scrollbar.css
		
		refresh_view/refresh_view.css
		refresh_view/images/refresh.png
		
		screen_search_view/images/close.png
		screen_search_view/screen_search_view.css
		
		search_view/images/arrow.png
		search_view/images/home.png
		search_view/images/search.png
		search_view/search_view.css
		
		startscreen/delete_icon.png
		startscreen/github_icon.png
		startscreen/startscreen.css
		
		statusbar_view/dot.png
		statusbar_view/loader.gif
		
		tabbed_view/arrow_down.png
		tabbed_view/arrow_up.png
		tabbed_view/tabbed_view.css
		
		tabs_view/images/add.png
		tabs_view/images/close.png
		tabs_view/tabs_view.css
		
		tooltip_view/tooltip_view.css
		
		undoredo_view/images/arrow_left.png
		undoredo_view/images/arrow_right.png
		undoredo_view/images/history.png
		undoredo_view/undoredo_view.css
		
		window/dots.png
		window/dots_hover.png
		window/help_hover.png
		window/help.png
		window/listbox.css
		window/logo.png
		window/minus_hover.png
		window/minus.png
		window/plus_hover.png
		window/plus.png
		window/refresh_hover.png
		window/refresh.png
		window/size_grip_black.png
		window/size_grip_white.png
		window/source_group_add_hover.png
		window/source_group_add.png
		window/source_group_copy_hover.png
		window/source_group_copy.png
		window/source_group_delete_hover.png
		window/source_group_delete.png
		window/window.css
)

setDefaultTargetOptions(Sourcetrail_res_gui)

set_target_properties(Sourcetrail_res_gui
	PROPERTIES
		AUTOMOC ON
)

target_include_directories(Sourcetrail_res_gui
	PUBLIC
		.
)

target_link_libraries(Sourcetrail_res_gui
	PUBLIC
		Sourcetrail_lib
		External_lib_qt
)
