# Wandelt — VS Code Extension

Syntax highlighting for the Wandelt programming language.

## Installation

### 1. Install the VSCE packager

```bash
npm install -g @vscode/vsce
```

### 2. Package the extension

```bash
cd editors/vscode
vsce package
```

This will produce a `wandelt-x.x.x.vsix` file.

### 3. Install in VS Code

Open VS Code, press `Ctrl+Shift+P`, type **"Install from VSIX"**, and select the generated `.vsix` file.
