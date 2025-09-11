extends Control


@onready var json_edit: CodeEdit = $VSplitContainer/HSplitContainer/JSON/JSONEdit
@onready var pbijson_edit: CodeEdit = $VSplitContainer/HSplitContainer/HSplitContainer/VBoxContainer/PBIJSONEdit
@onready var console: RichTextLabel = $VSplitContainer/RichTextLabel
@onready var mode_button: OptionButton = $VSplitContainer/HSplitContainer/HSplitContainer/VBoxContainer/HBoxContainer/Mode
@onready var path_edit: LineEdit = $VSplitContainer/HSplitContainer/HSplitContainer/VBoxContainer/HBoxContainer/Path
@onready var get_button: Button = $VSplitContainer/HSplitContainer/HSplitContainer/VBoxContainer/HBoxContainer/Button
@onready var cache_checkbox: CheckBox = $VSplitContainer/HSplitContainer/HSplitContainer/VBoxContainer/HBoxContainer/CheckBox

var pbij := PreBuiltIndexJSON.new()

func _ready() -> void:
	get_button.pressed.connect(_on_get_button_pressed)
	cache_checkbox.toggled.connect(_on_cache_toggled)
	_on_cache_toggled(cache_checkbox.button_pressed)

	var file :=FileAccess.open("res://test/test_json_data.json",FileAccess.READ)
	json_edit.text = file.get_as_text()
	#file.close()

	var data = JSON.parse_string(file.get_as_text())

	prints("find in dict:",benchmark_average_time(benchmark_get_value.bind(data)),"usec")

func benchmark_get_value(data:Dictionary) ->void:
	var test_data = data["characters"]["char_0008"]
	print(test_data)

func benchmark_average_time(code_to_run: Callable, repetitions: int = 100) -> float:
	if repetitions <= 0:
		return 0.0

	var total_elapsed_usec: int = 0

	code_to_run.call()

	for i in range(repetitions):
		var start_time_usec := Time.get_ticks_usec()
		code_to_run.call()
		var end_time_usec := Time.get_ticks_usec()
		total_elapsed_usec += end_time_usec - start_time_usec

	var average_time_usec := float(total_elapsed_usec) / repetitions
	return average_time_usec

func _on_build_button_down() -> void:
	var memory1 := Performance.get_monitor(Performance.MEMORY_STATIC)
	pbij.clear()
	console.append_text("Clear memory: %d kb\n" % ((memory1-Performance.get_monitor(Performance.MEMORY_STATIC)) /1024))
	pbijson_edit.text = ""

	console.append_text("[color=cyan]Building PBIJSON from input...[/color]\n")

	var output := pbij.build_from_string(json_edit.text)

	if output.get_error_type() != PreBuiltIndexJSONOutput.ErrorType.OK:
		if output.has_message():
			console.append_text("[color=red]Build Error: %s[/color]\n" % output.get_message())
		else:
			console.append_text("[color=red]An unknown build error occurred.[/color]\n")
		return

	pbijson_edit.text = output.get_data()
	pbij.open_from_string(output.get_data())

	console.append_text("[color=lime]Build successful. PBIJSON data is ready.[/color]\n")

	await get_tree().process_frame

	var base_memory = Performance.get_monitor(Performance.MEMORY_STATIC)
	console.append_text("Baseline Memory: %s kb\n" % (base_memory / 1024.0))

	@warning_ignore("unused_variable")
	var json_data = JSON.parse_string(json_edit.text)
	await get_tree().process_frame
	var json_memory = Performance.get_monitor(Performance.MEMORY_STATIC)
	console.append_text("  - Memory after loading JSON object: %s kb (Delta: %s kb)\n" % [json_memory / 1024.0, (json_memory - base_memory) / 1024.0])

	json_data = null
	await get_tree().process_frame

	var pbij_instance_memory = Performance.get_monitor(Performance.MEMORY_STATIC)
	console.append_text("  - Memory with PBIJSON object loaded: %s kb (Delta: %s kb)\n" % [pbij_instance_memory / 1024.0, (pbij_instance_memory - base_memory) / 1024.0])

func _on_get_button_pressed() -> void:
	if not pbij.is_data_loaded():
		console.append_text("[color=yellow]Warning: No PBIJSON data loaded. Please build first.[/color]\n")
		return

	var mode_id: int = mode_button.selected
	var key_path: String = path_edit.text
	var result: Variant
	var time_start: int
	var time_spent: int

	var mode_name = mode_button.get_item_text(mode_id)
	console.append_text("Executing [b]%s[/b] on path: [i]'%s'[/i]\n" % [mode_name, key_path])

	time_start = Time.get_ticks_usec()

	match mode_id:
		0: # get_value
			result = pbij.get_value(key_path)
		1: # get_keys
			result = pbij.get_keys(key_path)
		2: # get_size
			result = pbij.get_size(key_path)
		3: # get_sub_paths
			result = pbij.get_sub_paths(key_path)
		4: # has_path
			result = pbij.has_path(key_path)

	time_spent = Time.get_ticks_usec() - time_start

	var last_error = pbij.get_last_error()
	if last_error != null and last_error.get_error_type() != PreBuiltIndexJSONOutput.ErrorType.OK:
		if last_error.has_message():
			console.append_text("[color=red]Error: %s[/color]\n" % last_error.get_message())
		else:
			console.append_text("[color=red]An unknown error occurred during get operation.[/color]\n")
		return

	var result_string: String
	if result is String:
		result_string = "\"%s\"" % result
	elif result is Dictionary or result is Array:
		result_string = JSON.stringify(result, "  ")
	else:
		result_string = str(result)

	console.append_text("  [color=lime]Result:[/color] %s\n" % result_string)
	console.append_text("  [color=aqua]Time spent:[/color] %d microseconds\n" % time_spent)
	console.append_text("--------------------------------------------------\n")



func _on_cache_toggled(toggled_on: bool) -> void:
	pbij.set_cache_enabled(PreBuiltIndexJSON.ALL,toggled_on)

	pbij.clear_caches()
