@tool
extends EditorPlugin



var tr_files := [
	preload("res://addons/pbi_json/translations/zh_CN.po")
]

func _enable_plugin() -> void:
	# Add autoloads here.
	pass


func _disable_plugin() -> void:
	# Remove autoloads here.
	pass


func _enter_tree() -> void:
	var doc_domain := TranslationServer.get_or_add_domain("godot.documentation")
	for file in tr_files:
		doc_domain.add_translation(file)
	pass


func _exit_tree() -> void:
	# Clean-up of the plugin goes here.
	var doc_domain := TranslationServer.get_or_add_domain("godot.documentation")
	for file in tr_files:
		doc_domain.remove_translation(file)
