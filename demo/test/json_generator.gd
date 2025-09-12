class_name JSONGenerator
extends Node

const NUM_CHARACTERS = 1000
var OUTPUT_FILE_PATH = "res://large_test_data.json"


func generate_and_save_json() -> bool:
	var root_dict = {
		"config": {
			"version": "1.0",
			"generated_at": Time.get_datetime_string_from_system()
		},
		"characters": {}
	}

	for i in range(NUM_CHARACTERS):
		var char_key = "char_%04d" % i
		var character_data = {
			"name": "Character " + str(i),
			"class": ["warrior", "mage", "rogue"][randi() % 3],
			"level": randi_range(1, 99),
			"hp": randi_range(100, 2000),
			"is_active": randf() > 0.2,
			"last_login": null,
			"inventory": [],
			"stats": {
				"strength": randi_range(5, 50),
				"dexterity": randi_range(5, 50),
				"intelligence": randi_range(5, 50),
				"resistances": {
					"fire": randf(),
					"ice": randf(),
					"lightning": randf()
				}
			}
		}

		var inventory_size = randi_range(0, 5)
		for j in range(inventory_size):
			character_data.inventory.append("item_%04d" % randi_range(1, 100))

		root_dict.characters[char_key] = character_data

	var json_string = JSON.stringify(root_dict, "  ")

	if json_string.is_empty():
		return false

	var file = FileAccess.open(OUTPUT_FILE_PATH, FileAccess.WRITE)
	if FileAccess.get_open_error() != OK:
		return false

	file.store_string(json_string)
	file.close()

	return true
