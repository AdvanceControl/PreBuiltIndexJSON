# PreBuiltIndexJSON

Instead of parsing a complex JSON tree every time you need a value, PBIJSON pre-processes your .json file into a highly optimized, flat-indexed .pbijson format. This format uses depth markers, jump markers to enable lightning-fast data retrieval using linear search.

## Features

* **Blazing Fast Lookups:** Radically speeds up  operations, especially for deeply nested data or large collections.
  
* **Low Memory Overhead:** The memory occupied by PBIJSON data is much less than that of a dictionary.
  
* **Smart Caching:** Features an optional, granular caching system to provide near-instant results for repeated queries.
  
* **Simple API:** Interact with your data using an intuitive, path-based API ("path/to/my/value") that feels just like using a nested Dictionary.
  
* **Offline Build Process:**  The heavy lifting of indexing is done once, offline, ensuring no performance spikes during gameplay.
  
* **Full Key Support:** Handles all valid JSON key names by supporting standard path escaping (e.g., characters/\\/char_0000 to access the key "\/char_0000").
