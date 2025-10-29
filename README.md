# Nucleo_WL55JC Zephyr Projects

This repository contains a collection of Zephyr-based projects for the **Nucleo-WL55JC** development board.

---
## 🚀 Usage

1. Open **VS Code** in the `Nucleo_wl55jc` directory.  
2. Press **Ctrl + Shift + B**.  
3. Choose the project number you want to build.  
4. Wait for the build process to complete.

---
## 📂 Project Structure

```
Nucleo_wl55jc/
├── [Projects]              # Example Zephyr application
├── .vscode/                # Visual Studio Code configuration
├── build_project.sh        # Bash script for automated project build
└── README.md
```

---

## ⚙️ `.vscode/` — VS Code Integration with Zephyr

The `.vscode` folder contains configuration files to make Zephyr development in VS Code easier and more powerful:

- **`settings.json`**  
  Connects to the `compile_commands.json` file so VS Code can understand all Zephyr include paths.  
  Enables navigation with `Ctrl+Click` (Go to Definition) for Zephyr API functions, structs, and macros.

- **`c_cpp_properties.json`**  
  Configures IntelliSense: autocompletion, header file recognition, and error highlighting.

- **`tasks.json`**  
  Adds a build task that can be executed with `Ctrl + Shift + B`.  
  This task runs the `build_project.sh` script, which allows you to select a project to build.

---

## 🛠️ `build_project.sh` — Automatic Project Selection and Build Script

The `build_project.sh` script automates project selection and building.  
It performs the following steps:

1. Scans the current directory (`Nucleo_wl55jc/*`) for all Zephyr projects.  
2. Displays them in a numbered list, for example:
   ```
   1) brightnessControl
   2) sensorTest
   3) buttonDemo
   ```
3. Prompts you to enter the project number.
4. Activates the Zephyr virtual environment (`.venv`).
5. Changes into the selected project directory and runs:
   ```bash
   west build --pristine=always -b nucleo_wl55jc
   ```