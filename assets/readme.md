# Stylist

Stylist is a plugin that allows you to customize any player character’s appearance quickly from a command line.  It can also be used to prevent character ‘blinking’ in a similar manner to GearLock from Ashita3.  Some notes:

* Model specific filters take priority over slot filters.
* Character-specific and Model-specific filters take priority over just model-specific filters.
* You may specify a model by using an item name or item ID.  You do not need to ever input model IDs yourself, but they are stored in ashita/resources/stylist/modelinfo.xml if you need to modify any.
* Note that while you specify item name or item ID for simplicity, a model filter will still apply to other items with the same model.
* Anywhere where equipment slot is specified, you may also change race or face.  Note that the full list of slot names and race names are configurable in ashita/resources/stylist/modelinfo.xml.
* Faces are specified using a numeric value from 0 to 15.
* All configuration can be done using the typed commands and exports, but you may also directly modify a setting file, see xmllayout.htm for details.

#### Default values for slots are:
* Race
* Face
* Head
* Body
* Hands
* Legs
* Feet
* Main
* Sub
* Range

#### Default values for races are:
* HumeM
* HumeF
* ElvaanM
* ElvaanF
* TaruM
* TaruF
* Mithra
* Galka


### Commands
All commands can be prefixed with /sl or /stylist.  You may also configure stylist via XML directly.

* /sl blink<br>
Triggers all characters who are outdated to blink to their current equipment, with any specified filters.

* /sl self [Optional: On/Off]<br>
When enabled, prevents you from blinking except when adding or removing filters or using /sl blink.

* /sl target [Optional: On/Off]<br>
When enabled, prevents your current target from blinking except when adding or removing filters or using /sl blink.

* /sl others [Optional: On/Off]<br>
When enabled, prevents all players from blinking except when adding or removing filters or using /sl blink.

* /sl add [Required: Character Name] [Required: Equipment Slot] [Required: Model]<br>
Adds a filter, forcing the specified slot to show the specified model for the specified character instead of what they actually have equipped.

* /sl remove [Required: Character Name] [Required: Equipment Slot]<br>
Removes a filter, allowing the specified character to display their real equipment in the specified slot.

* /sl addmodel [Required: Equipment Slot] [Required: Initial Model] [Required: Target Model]<br>
Adds a global model filter, changing the initial model to the target model for all characters.

* /sl removemodel [Required: Equipment Slot] [Required: Initial Model]<br>
Removes a global model filter.

* /sl addmodelc [Required: Character Name] [Required: Equipment Slot] [Required: Initial Model] [Required: Target Model]<br>
Adds a character-specific model filter, changing the initial model to the target model for only the specified character.

* /sl removemodelc [Required: Character Name] [Required: Equipment Slot] [Required: Initial Model]<br>
Removes a character-specific model filter.

* /sl clear [Required: Character Name]<br>
Removes all filters and model filters for the specified character, allowing them to show their real equipment.  Global model filters will still apply.

* /sl load [Optional: Filename]<br>
Loads a configuration XML.  If Filename is not specified, reloads current configuration XML.

* /sl reload<br>
Reloads the current configuration XML from disk.

* /sl newconfig [Required: Filename]<br>
Creates a new configuration XML on disk and loads it.  This will return you to default settings.  If you are trying to save your settings, use /sl export.

* /sl export [Optional: Filename]<br>
Saves the current configuration to a configuration XML. 